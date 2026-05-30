#pragma once

#include <cstddef>
#include <vector>

#include "common.hpp"

namespace huffman {

// Caso 2: Gap-Coding + Sample. m = ceil(sqrt(n)), b = ceil(n / m).
template <typename T = Elem>
class GapCodedArray {
public:
    void build(const std::vector<T>& sorted_input) {
        // TODO
        (void)sorted_input;
    }

    long search(T value) const {
        // TODO
        (void)value;
        return -1;
    }

    std::size_t size_in_bytes() const {
        return (GC_.size() + Sample_.size()) * sizeof(T);
    }

    std::size_t n() const { return n_; }
    std::size_t m() const { return m_; }
    std::size_t b() const { return b_; }

private:
    std::vector<T> GC_;
    std::vector<T> Sample_;
    std::size_t   n_ = 0;
    std::size_t   m_ = 0;
    std::size_t   b_ = 0;
};

}  // namespace huffman
