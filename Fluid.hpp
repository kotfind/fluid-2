#pragma once

#include "VectorField.hpp"
#include "Rnd.hpp"
#include "ParticleParams.hpp"

#include <cstring>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>

template<typename T>
class ParticleParams;

template<size_t N, size_t M, typename T>
class Fluid {
    private:
        static constexpr size_t max_ticks = 1'000'000;

    public:
        Fluid(const std::string& filename) {
            std::ifstream fin(filename);
            std::string line;
            std::stringstream ss;

            // N and M
            if (!std::getline(fin, line)) {
                throw std::runtime_error("failed to read line with N and M");
            }
            ss = std::stringstream{line};
            size_t n, m;
            if (!(ss >> n) || !(ss >> m)) {
                throw std::runtime_error("failed to read N or M");
            }
            assert(n == N && m == M);

            // Field
            // std::vector<std::vector<char>> field(n, std::vector<char>(m));
            // field.assign(n, std::vector<char>(m));
            for (size_t i = 0; i < n; ++i) {
                if (!std::getline(fin, line)) {
                    throw std::runtime_error("failed to read field");
                }
                if (line.size() != m) {
                    throw std::runtime_error("wrong length of fileld row");
                }
                for (size_t j = 0; j < m; ++j) {
                    field[i][j] = line[j];
                }
            }

            // G
            if (!std::getline(fin, line)) {
                throw std::runtime_error("failed to read line with G");
            }
            ss = std::stringstream{line};
            if (!(ss >> g)) {
                std::cout << "__" << line << "__" << std::endl;
                throw std::runtime_error("failed to read G");
            }

            // Rho
            // T rho[256];
            while (std::getline(fin, line)) {
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

        void run() {
            for (size_t x = 0; x < N; ++x) {
                for (size_t y = 0; y < M; ++y) {
                    if (field[x][y] == '#')
                        continue;
                    for (auto [dx, dy] : deltas) {
                        dirs[x][y] += (field[x + dx][y + dy] != '#');
                    }
                }
            }

            for (size_t i = 0; i < max_ticks; ++i) {
                T total_delta_p = 0;
                // Apply external forces
                for (size_t x = 0; x < N; ++x) {
                    for (size_t y = 0; y < M; ++y) {
                        if (field[x][y] == '#')
                            continue;
                        if (field[x + 1][y] != '#')
                            velocity.add(x, y, 1, 0, g);
                    }
                }

                // Apply forces from p
                std::memcpy(old_p, p, sizeof(p));
                for (size_t x = 0; x < N; ++x) {
                    for (size_t y = 0; y < M; ++y) {
                        if (field[x][y] == '#')
                            continue;
                        for (auto [dx, dy] : deltas) {
                            int nx = x + dx, ny = y + dy;
                            if (field[nx][ny] != '#' && old_p[nx][ny] < old_p[x][y]) {
                                auto delta_p = old_p[x][y] - old_p[nx][ny];
                                auto force = delta_p;
                                auto &contr = velocity.get(nx, ny, -dx, -dy);
                                if (contr * rho[(int) field[nx][ny]] >= force) {
                                    contr -= force / rho[(int) field[nx][ny]];
                                    continue;
                                }
                                force -= contr * rho[(int) field[nx][ny]];
                                contr = 0;
                                velocity.add(x, y, dx, dy, force / rho[(int) field[x][y]]);
                                p[x][y] -= force / dirs[x][y];
                                total_delta_p -= force / dirs[x][y];
                            }
                        }
                    }
                }

                // Make flow from velocities
                velocity_flow = {};
                bool prop = false;
                do {
                    UT += 2;
                    prop = 0;
                    for (size_t x = 0; x < N; ++x) {
                        for (size_t y = 0; y < M; ++y) {
                            if (field[x][y] != '#' && last_use[x][y] != UT) {
                                auto [t, local_prop, _] = propagate_flow(x, y, 1);
                                if (t > 0) {
                                    prop = 1;
                                }
                            }
                        }
                    }
                } while (prop);

                // Recalculate p with kinetic energy
                for (size_t x = 0; x < N; ++x) {
                    for (size_t y = 0; y < M; ++y) {
                        if (field[x][y] == '#')
                            continue;
                        for (auto [dx, dy] : deltas) {
                            auto old_v = velocity.get(x, y, dx, dy);
                            auto new_v = velocity_flow.get(x, y, dx, dy);
                            if (old_v > 0) {
                                assert(new_v <= old_v);
                                velocity.get(x, y, dx, dy) = new_v;
                                auto force = (old_v - new_v) * rho[(int) field[x][y]];
                                if (field[x][y] == '.')
                                    force *= 0.8;
                                if (field[x + dx][y + dy] == '#') {
                                    p[x][y] += force / dirs[x][y];
                                    total_delta_p += force / dirs[x][y];
                                } else {
                                    p[x + dx][y + dy] += force / dirs[x + dx][y + dy];
                                    total_delta_p += force / dirs[x + dx][y + dy];
                                }
                            }
                        }
                    }
                }

                UT += 2;
                prop = false;
                for (size_t x = 0; x < N; ++x) {
                    for (size_t y = 0; y < M; ++y) {
                        if (field[x][y] != '#' && last_use[x][y] != UT) {
                            if (Rnd::random01<T>() < move_prob(x, y)) {
                                prop = true;
                                propagate_move(x, y, true);
                            } else {
                                propagate_stop(x, y, true);
                            }
                        }
                    }
                }

                if (prop) {
                    std::cout << "Tick " << i << ":\n";
                    for (size_t x = 0; x < N; ++x) {
                        std::cout << field[x] << "\n";
                    }
                }
            }
        }

    private:
        std::tuple<T, bool, std::pair<int, int>> propagate_flow(int x, int y, T lim) {
            last_use[x][y] = UT - 1;
            T ret = 0;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT) {
                    auto cap = velocity.get(x, y, dx, dy);
                    auto flow = velocity_flow.get(x, y, dx, dy);
                    if (flow == cap) {
                        continue;
                    }
                    // assert(v >= velocity_flow.get(x, y, dx, dy));
                    auto vp = std::min(lim, cap - flow);
                    if (last_use[nx][ny] == UT - 1) {
                        velocity_flow.add(x, y, dx, dy, vp);
                        last_use[x][y] = UT;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << vp << " / " << lim << "\n";
                        return {vp, 1, {nx, ny}};
                    }
                    auto [t, prop, end] = propagate_flow(nx, ny, vp);
                    ret += t;
                    if (prop) {
                        velocity_flow.add(x, y, dx, dy, t);
                        last_use[x][y] = UT;
                        // cerr << x << " " << y << " -> " << nx << " " << ny << " " << t << " / " << lim << "\n";
                        return {t, prop && end != std::make_pair(x, y), end};
                    }
                }
            }
            last_use[x][y] = UT;
            return {ret, 0, {0, 0}};
        }

        void propagate_stop(int x, int y, bool force = false) {
            if (!force) {
                bool stop = true;
                for (auto [dx, dy] : deltas) {
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) > 0) {
                        stop = false;
                        break;
                    }
                }
                if (!stop) {
                    return;
                }
            }
            last_use[x][y] = UT;
            for (auto [dx, dy] : deltas) {
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT || velocity.get(x, y, dx, dy) > 0) {
                    continue;
                }
                propagate_stop(nx, ny);
            }
        }

        T move_prob(int x, int y) {
            T sum = 0;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
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

        bool propagate_move(int x, int y, bool is_first) {

            last_use[x][y] = UT - is_first;
            bool ret = false;
            int nx = -1, ny = -1;
            do {
                std::array<T, deltas.size()> tres;
                T sum = 0;
                for (size_t i = 0; i < deltas.size(); ++i) {
                    auto [dx, dy] = deltas[i];
                    int nx = x + dx, ny = y + dy;
                    if (field[nx][ny] == '#' || last_use[nx][ny] == UT) {
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

                T p = Rnd::random01<T>() * sum;
                size_t d = std::ranges::upper_bound(tres, p) - tres.begin();

                auto [dx, dy] = deltas[d];
                nx = x + dx;
                ny = y + dy;
                assert(velocity.get(x, y, dx, dy) > 0 && field[nx][ny] != '#' && last_use[nx][ny] < UT);

                ret = (last_use[nx][ny] == UT - 1 || propagate_move(nx, ny, false));
            } while (!ret);
            last_use[x][y] = UT;
            for (size_t i = 0; i < deltas.size(); ++i) {
                auto [dx, dy] = deltas[i];
                int nx = x + dx, ny = y + dy;
                if (field[nx][ny] != '#' && last_use[nx][ny] < UT - 1 && velocity.get(x, y, dx, dy) < 0) {
                    propagate_stop(nx, ny);
                }
            }
            if (ret) {
                if (!is_first) {
                    ParticleParams<T> pp{};
                    pp.swap_with(*this, x, y);
                    pp.swap_with(*this, nx, ny);
                    pp.swap_with(*this, x, y);
                }
            }
            return ret;
        }

        char field[N][M + 1];

        T rho[256];

        T p[N][M];
        T old_p[N][M];

        VectorField<N, M, T> velocity;
        VectorField<N, M, T> velocity_flow;
        int last_use[N][M];
        int UT;

        T g;

        int dirs[N][M];

    friend ParticleParams<T>;
};
