#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "common.hpp"

namespace huffman {

// Distribución lineal: A[0] = rand(); A[i] = A[i-1] + rand() % epsilon.
inline std::vector<Elem> gen_linear(std::size_t n, std::uint32_t epsilon, std::uint64_t seed) {
    // TODO
    (void)n; (void)epsilon; (void)seed;
    return {};
}

// Distribución normal N(mu, sigma), ordenada ascendentemente.
inline std::vector<Elem> gen_normal(std::size_t n, double mu, double sigma, std::uint64_t seed) {
    // TODO
    (void)n; (void)mu; (void)sigma; (void)seed;
    return {};
}

}  // namespace huffman
