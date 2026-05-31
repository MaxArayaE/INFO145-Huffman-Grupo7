#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

#include "common.hpp"

namespace huffman {

// Distribución lineal: A[0] = rand() % epsilon; A[i] = A[i-1] + rand() % epsilon.
// Clampa al máximo de Elem si la suma se desbordaría.
inline std::vector<Elem> gen_linear(std::size_t n, std::uint32_t epsilon, std::uint64_t seed) {
    if (n == 0 || epsilon == 0) return {};

    std::vector<Elem> A;
    A.reserve(n);
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint32_t> step(0, epsilon - 1);

    Elem current = step(rng);
    A.push_back(current);
    const Elem ELEM_MAX = std::numeric_limits<Elem>::max();
    for (std::size_t i = 1; i < n; ++i) {
        std::uint32_t s = step(rng);
        if (current > ELEM_MAX - s) {
            current = ELEM_MAX;
        } else {
            current += s;
        }
        A.push_back(current);
    }
    return A;
}

// Distribución normal de los valores: muestrea n valores de N(mu, sigma),
// los clampa al rango de Elem y los ordena ascendentemente.
inline std::vector<Elem> gen_normal(std::size_t n, double mu, double sigma, std::uint64_t seed) {
    if (n == 0) return {};

    std::vector<Elem> A;
    A.reserve(n);
    std::mt19937_64 rng(seed);
    std::normal_distribution<double> dist(mu, sigma);

    const double ELEM_MAX_D = static_cast<double>(std::numeric_limits<Elem>::max());
    for (std::size_t i = 0; i < n; ++i) {
        double v = dist(rng);
        if (v < 0.0) v = 0.0;
        if (v > ELEM_MAX_D) v = ELEM_MAX_D;
        A.push_back(static_cast<Elem>(v));
    }
    std::sort(A.begin(), A.end());
    return A;
}

}  // namespace huffman
