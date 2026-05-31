#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <vector>

#include "common.hpp"

namespace huffman {

// Caso 1: arreglo ordenado explícito + búsqueda binaria.
template <typename T = Elem>
class ExplicitArray {
public:
    void build(const std::vector<T>& sorted_input) {
        A_ = sorted_input;
    }

    long search(T value) const {
        auto it = std::lower_bound(A_.begin(), A_.end(), value);
        if (it != A_.end() && *it == value) {
            return std::distance(A_.begin(), it);
        }
        return -1;
    }

    std::size_t size_in_bytes() const {
        return A_.size() * sizeof(T);
    }

private:
    std::vector<T> A_;
};

}  // namespace huffman
