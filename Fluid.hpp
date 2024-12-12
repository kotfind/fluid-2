#pragma once

#include "FixedInner.hpp"
#include "ParticleParams.hpp"
#include "Matrix.hpp"
#include "ThreadPool.hpp"
#include "VectorField.hpp"
#include "Rnd.hpp"

#include <concepts>
#include <cstring>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <memory>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <string>

template<typename P_TYPE, typename V_TYPE>
class ParticleParams;

template<typename P_TYPE, typename V_TYPE, typename V_FLOW_TYPE>
class Fluid {
    private:
        using V_COMMON_TYPE = typename CommonTypeFixed<V_TYPE, V_FLOW_TYPE>::type;

    public:
        Fluid(const std::string& filename) {
            read_from_file(filename);
        }

        void run(size_t ticks_count = 1'000'000, bool quiet = false) {
            init_dirs();

            for (size_t tick_num = 0; tick_num < ticks_count; ++tick_num) {
                tick(tick_num, quiet);
            }
        }

    private:
        /// Reads data from file (is used in the constructor)
        void read_from_file(const std::string& filename) {
            std::ifstream fin(filename);
            if (!fin) {
                throw std::runtime_error("failed to open file");
            }

            std::string line;
            std::stringstream ss;

            auto read_line = [&fin, &line]() -> bool {
                for (;;) {
                    if (!std::getline(fin, line)) {
                        return false;
                    }

                    if (line.starts_with("//")) {
                        continue;
                    }

                    return true;
                }
            };

            // N and M
            if (!read_line()) {
                throw std::runtime_error("failed to read line with N and M");
            }
            ss = std::stringstream{line};
            if (!(ss >> n) || !(ss >> m)) {
                throw std::runtime_error("failed to read N or M");
            }

            // Field
            field.reset(create_matrix<char>{}(n, m + 1));
            p.reset(create_matrix<P_TYPE>{}(n, m));
            last_use.reset(create_matrix<int>{}(n, m));
            dirs.reset(create_matrix<int>{}(n, m));
            velocity = VectorField<V_TYPE>{n, m};
            velocity_flow = VectorField<V_FLOW_TYPE>{n, m};
            for (size_t i = 0; i < n; ++i) {
                if (!read_line()) {
                    throw std::runtime_error("failed to read field");
                }
                if (line.size() != m) {
                    throw std::runtime_error("wrong length of fileld row");
                }
                for (size_t j = 0; j < m; ++j) {
                    (*field)[i][j] = line[j];
                }
            }

            // G
            if (!read_line()) {
                throw std::runtime_error("failed to read line with G");
            }
            ss = std::stringstream{line};
            if (!(ss >> g)) {
                throw std::runtime_error("failed to read G");
            }

            // Rho
            while (read_line()) {
                if (line.find_first_not_of(' ') == std::string::npos || line.empty()) {
                    break;
                }

                ss = std::stringstream{line};
                char c = ss.get();
                if (!(ss >> rho[c])) {
                    throw std::runtime_error("failed to read rho value");
                }
            }
        }

        /// Inits dirs matrix
        void init_dirs() {
            forall([this](size_t x, size_t y){
                if ((*field)[x][y] == '#')
                    return;
                for (auto [dx, dy] : deltas) {
                    (*dirs)[x][y] += ((*field)[x + dx][y + dy] != '#');
                }
            });
        }

        /// Performs single tick
        void tick(size_t tick_num, bool quiet = false) {
            P_TYPE total_delta_p = 0;

            apply_gravity();
            apply_p_forces(total_delta_p);
            recalc_flow();
            recalc_p(total_delta_p);

            if (maybe_propagate() && !quiet) {
                std::cout
                    << "Tick " << tick_num << ":\n"
                    << *field << std::endl;
            }
        }

        /// Apply external forces
        /// Reads:
        ///     field
        /// Writes:
        ///     velocity
        void apply_gravity() {
            forall([this](size_t x, size_t y) {
                if ((*field)[x][y] == '#')
                    return;
                if ((*field)[x + 1][y] != '#')
                    velocity.add(x, y, 1, 0, g);
            });
        }

        /// Apply forces from p
        /// Reads:
        ///     p, velocity
        /// Writes:
        ///     velocity
        void apply_p_forces(P_TYPE& total_delta_p) {
            static auto old_p = std::unique_ptr<AbstractMatrix<P_TYPE>>(create_matrix<P_TYPE>{}(n, m));

            forall([this](size_t x, size_t y) -> void {
                (*old_p)[x][y] = (*p)[x][y];
            });

            for (size_t x = 0; x < n; ++x) {
                for (size_t y = 0; y < m; ++y) {
                    if ((*field)[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        int nx = x + dx, ny = y + dy;
                        if ((*field)[nx][ny] != '#' && (*old_p)[nx][ny] < (*old_p)[x][y]) {
                            auto delta_p = (*old_p)[x][y] - (*old_p)[nx][ny];
                            auto force = delta_p;
                            auto &contr = velocity.get(nx, ny, -dx, -dy);
                            if (contr * rho[(int) ((*field)[nx][ny])] >= force) {
                                contr -= force / rho[(int) ((*field)[nx][ny])];
                                continue;
                            }
                            force -= contr * rho[(int) ((*field)[nx][ny])];
                            contr = 0;
                            velocity.add(x, y, dx, dy, force / rho[(int) ((*field)[x][y])]);
                            (*p)[x][y] -= force / (*dirs)[x][y];
                            total_delta_p -= force / (*dirs)[x][y];
                        }
                    }
                }
            }
        }

        /// Make flow from velocities
        /// Reads:
        ///     last_use, UT
        /// Writes:
        ///     UT
        /// TODO: inderect: propagate_flow
        void recalc_flow() {
            velocity_flow.reset();
            bool prop = false;
            do {
                UT += 2;
                prop = 0;
                for (size_t x = 0; x < n; ++x) {
                    for (size_t y = 0; y < m; ++y) {
                        if ((*field)[x][y] != '#' && (*last_use)[x][y] != UT) {
                            auto [t, local_prop, _] = propagate_flow(x, y, 1);
                            if (t > 0) {
                                prop = 1;
                            }
                        }
                    }
                }
            } while (prop);
        }

        /// Recalculate p with kinetic energy
        /// Reads:
        ///     velocity, velocity_flow
        /// Writes:
        ///     p
        void recalc_p(P_TYPE& total_delta_p) {
            for (size_t x = 0; x < n; ++x) {
                for (size_t y = 0; y < m; ++y) {
                    if ((*field)[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        auto old_v = velocity.get(x, y, dx, dy);
                        auto new_v = velocity_flow.get(x, y, dx, dy);
                        if (old_v > 0) {
                            assert(new_v <= old_v);
                            velocity.get(x, y, dx, dy) = new_v;
                            auto force = (old_v - new_v) * rho[(int) ((*field)[x][y])];
                            if ((*field)[x][y] == '.')
                                force *= 0.8;
                            if ((*field)[x + dx][y + dy] == '#') {
                                (*p)[x][y] += force / (*dirs)[x][y];
                                total_delta_p += force / (*dirs)[x][y];
                            } else {
                                (*p)[x + dx][y + dy] += force / (*dirs)[x + dx][y + dy];
                                total_delta_p += force / (*dirs)[x + dx][y + dy];
                            }
                        }
                    }
                }
            }
        }

        /// Reads:
        ///     last_use, UT
        /// Writes:
        ///     UT
        /// TODO: inderect: propagate_move, propagate_stop, move_prob
        bool maybe_propagate() {
            UT += 2;
            bool prop = false;
            for (size_t x = 0; x < n; ++x) {
                for (size_t y = 0; y < m; ++y) {
                    if ((*field)[x][y] != '#' && (*last_use)[x][y] != UT) {
                        if (Rnd::random01<V_TYPE>() < move_prob(x, y)) {
                            prop = true;
                            propagate_move(x, y, true);
                        } else {
                            propagate_stop(x, y, true);
                        }
                    }
                }
            }
            return prop;
        }

        /// Reads:
        ///     UT, last_use, velocity_flow, velocity
        /// Writes:
        ///     last_use, velocity_flow, 
        /// TODO: inderect: propagate_flow
        std::tuple<V_COMMON_TYPE, bool, std::pair<int, int>> propagate_flow(int x, int y, V_COMMON_TYPE lim) {
            (*last_use)[x][y] = UT - 1;
            V_COMMON_TYPE ret = 0;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if ((*field)[nx][ny] != '#' && (*last_use)[nx][ny] < UT) {
                    auto cap = velocity.get(x, y, dx, dy);
                    auto flow = velocity_flow.get(x, y, dx, dy);
                    if (flow == cap) {
                        continue;
                    }
                    // assert(v >= velocity_flow.get(x, y, dx, dy));
                    auto vp = std::min(lim, cap - flow);
                    if ((*last_use)[nx][ny] == UT - 1) {
                        velocity_flow.add(x, y, dx, dy, vp);
                        (*last_use)[x][y] = UT;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                        return {vp, 1, {nx, ny}};
                    }
                    auto [t, prop, end] = propagate_flow(nx, ny, vp);
                    ret += t;
                    if (prop) {
                        velocity_flow.add(x, y, dx, dy, t);
                        (*last_use)[x][y] = UT;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                        return {t, prop && end != std::make_pair(x, y), end};
                    }
                }
            }
            (*last_use)[x][y] = UT;
            return {ret, 0, {0, 0}};
        }

        /// Reads:
        ///     last_use, velocity
        /// Writes:
        ///     last_use
        /// TODO: inderect: propagate_flow
        void propagate_stop(int x, int y, bool force = false) {
            if (!force) {
                bool stop = true;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if ((*field)[nx][ny] != '#' && (*last_use)[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > 0) {
                        stop = false;
                        break;
                    }
                }
                if (!stop) {
                    return;
                }
            }
            (*last_use)[x][y] = UT;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if ((*field)[nx][ny] == '#' || (*last_use)[nx][ny] == UT || velocity.get(x, y, dx, dy) > 0) {
                    continue;
                }
                propagate_stop(nx, ny);
            }
        }

        /// Reads:
        ///     last_use, UT, velocity
        V_TYPE move_prob(int x, int y) {
            V_TYPE sum = 0;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if ((*field)[nx][ny] == '#' || (*last_use)[nx][ny] == UT) {
                    continue;
                }
                auto v = velocity.get(x, y, dx, dy);
                if (v < 0) {
                    continue;
                }
                sum += v;
            }
            return sum;
        }

        /// Reads:
        ///     UT, last_use, velocity
        /// Writes:
        ///     last_use
        /// TODO: inderect: propagate_move, propagate_stop, swap_with
        bool propagate_move(int x, int y, bool is_first) {
            (*last_use)[x][y] = UT - is_first;
            bool ret = false;
            int nx = -1, ny = -1;
            do {
                std::array<V_TYPE, deltas.size()> tres;
                V_TYPE sum = 0;
                for (size_t i = 0; i < deltas.size(); ++i) {
                    auto [dx, dy] = deltas[i];
                    int nx = x + dx, ny = y + dy;
                    if ((*field)[nx][ny] == '#' || (*last_use)[nx][ny] == UT) {
                        tres[i] = sum;
                        continue;
                    }
                    auto v = velocity.get(x, y, dx, dy);
                    if (v < 0) {
                        tres[i] = sum;
                        continue;
                    }
                    sum += v;
                    tres[i] = sum;
                }

                if (sum == 0) {
                    break;
                }

                V_TYPE p = Rnd::random01<V_TYPE>() * sum;
                size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

                auto [dx, dy] = deltas[d];
                nx = x + dx;
                ny = y + dy;
                assert(velocity.get(x, y, dx, dy) > 0 && (*field)[nx][ny] != '#' && (*last_use)[nx][ny] < UT);

                ret = ((*last_use)[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
            } while (!ret);
            (*last_use)[x][y] = UT;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if ((*field)[nx][ny] != '#' && (*last_use)[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < 0) {
                    propagate_stop(nx, ny);
                }
            }
            if (ret) {
                if (!is_first) {
                    ParticleParams<P_TYPE, V_TYPE> pp{};
                    pp.swap_with(*this, x, y);
                    pp.swap_with(*this, nx, ny);
                    pp.swap_with(*this, x, y);
                }
            }
            return ret;
        }

        template<typename F>
        requires requires(const F& f, size_t x, size_t y) {
            { f(x, y) } -> std::same_as<void>;
        }
        void forall(const F& f) {
            size_t chunk_size = 1000;

            for (size_t from = 0; from < n * m; from += chunk_size) {
                pool.add_task([this, &f, from, chunk_size]{
                    size_t i = from;
                    size_t x = from / m;
                    size_t y = from % m;
                    while (i - from < chunk_size && x < n) {
                        f(x, y);

                        // increment 
                        ++i;
                        ++y;
                        if (y == m) {
                            y = 0;
                            ++x;
                        }
                    }
                });
            }
            pool.wait_all();
        }

        size_t n, m;
        std::unique_ptr<AbstractMatrix<char>> field = nullptr; // N x M + 1

        P_TYPE rho[256];

        std::unique_ptr<AbstractMatrix<P_TYPE>> p = nullptr; // N x M

        VectorField<V_TYPE> velocity;
        VectorField<V_FLOW_TYPE> velocity_flow;
        std::unique_ptr<AbstractMatrix<int>> last_use = nullptr; // N x M
        int UT;

        V_TYPE g;

        std::unique_ptr<AbstractMatrix<int>> dirs = nullptr; // N x M

    friend ParticleParams<P_TYPE, V_TYPE>;
};
