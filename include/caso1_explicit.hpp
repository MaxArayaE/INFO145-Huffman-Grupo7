#pragma once

#include <cstddef>
#include <vector>

#include "common.hpp"

namespace huffman {

// Caso 1: arreglo ordenado explícito + búsqueda binaria.
template <typename T = Elem>
class ExplicitArray {
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
        return A_.size() * sizeof(T);
    }

private:
    std::vector<T> A_;
};

}  // namespace huffman
