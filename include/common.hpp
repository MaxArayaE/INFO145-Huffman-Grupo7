#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace huffman {

using Elem = std::uint32_t;

constexpr std::uint64_t SEED_DATA    = 42;
constexpr std::uint64_t SEED_QUERIES = 43;
constexpr std::size_t   NUM_QUERIES  = 100'000;
constexpr std::size_t   WARMUP       = 1'000;

class Timer {
public:
    Timer() : start_(std::chrono::steady_clock::now()) {}

    void reset() { start_ = std::chrono::steady_clock::now(); }

    std::uint64_t elapsed_ns() const {
        auto end = std::chrono::steady_clock::now();
        return static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count());
    }

private:
    std::chrono::steady_clock::time_point start_;
};

}  // namespace huffman
