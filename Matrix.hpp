#pragma once

#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>

template<typename T>
class AbstractMatrix {
    public:
        AbstractMatrix& operator=(const AbstractMatrix& other) {
            assert(get_n() == other.get_n() && get_m() == other.get_m());
            for (size_t i = 0; i < get_n(); ++i) {
                for (size_t j = 0; j < get_m(); ++j) {
                    (*this)[i][j] = other[i][j];
                }
            }
            return *this;
        }

        virtual ~AbstractMatrix() = default;

        virtual size_t get_n() const = 0;

        virtual size_t get_m() const = 0;

        void reset() {
            for (size_t i = 0; i < get_n(); ++i) {
                for (size_t j = 0; j < get_m(); ++j) {
                    (*this)[i][j] = T{};
                }
            }
        }

        virtual T* operator[](size_t i) = 0;
        virtual T* operator[](size_t i) const = 0;
};

template<typename T, size_t N, size_t M>
class StaticMatix : public AbstractMatrix<T> {
    public:
        StaticMatix(size_t n, size_t m)
          : data(new T[N * M]{})
        {
            assert(n == N && m == M);
        }

        size_t get_n() const override {
            return N;
        }

        size_t get_m() const override {
            return M;
        }

        T* operator[](size_t i) override {
            return (T*)data.get() + i * M;
        }

        T* operator[](size_t i) const override {
            return (T*)data.get() + i * M;
        }

    private:
        std::unique_ptr<T[]> data;
};

template<typename T>
class DynamicMatrix : public AbstractMatrix<T> {
    public:
        DynamicMatrix(size_t n, size_t m)
          : data(new T[n * m]{}),
            n(n),
            m(m)
        {}

        T* operator[](size_t i) override {
            return (T*)data.get() + i * m;
        }

        T* operator[](size_t i) const override {
            return (T*)data.get() + i * m;
        }

        size_t get_n() const override {
            return n;
        }

        size_t get_m() const override {
            return m;
        }

    private:
        size_t n, m;;
        std::unique_ptr<T[]> data;
};

struct size_marker {
    size_t n;
    size_t m;
};

template<typename T, size_marker... SZS>
struct create_matrix_;

template<typename T, size_marker SZ, size_marker... SZS>
struct create_matrix_<T, SZ, SZS...> {
    AbstractMatrix<T>* operator()(size_t n, size_t m) {
        constexpr auto N = SZ.n;
        constexpr auto M = SZ.m;

        if (N == n && M == m) {
            std::cout << "Using StaticMatrix: N = " << N << ", M = " << M << std::endl;
            return new StaticMatix<T, N, M>(N, M);
        }

        return create_matrix_<T, SZS...>{}(n, m);
    }
};

template<typename T>
struct create_matrix_<T> {
    AbstractMatrix<T>* operator()(size_t n, size_t m) {
        std::cout << "Using DynamicMatrix: N = " << n << ", M = " << m << std::endl;
        return new DynamicMatrix<T>(n, m);
    }
};

#define S(n, m) size_marker(n, m)
#define SIZES S(10, 10),S(17, 30)

template<typename T>
struct create_matrix : public create_matrix_<T, SIZES> {};
