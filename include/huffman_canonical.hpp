#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bitstream.hpp"
#include "common.hpp"

namespace huffman {

// Códigos canónicos de Huffman. Símbolos ordenados por (largo asc, símbolo asc).
struct HuffmanCanonical {
    std::vector<Elem>          symbols;
    std::vector<std::uint64_t> codes;
    std::vector<std::uint32_t> code_len;
    std::vector<std::uint32_t> F;        // F[l] = índice del 1er símbolo de largo l
    std::vector<std::uint64_t> C;        // C[l] = primer código de largo l
    std::uint32_t              h_min = 0;
    std::uint32_t              h_max = 0;
    std::unordered_map<Elem, std::uint32_t> sym_to_idx;

    void build(const std::vector<Elem>& sequence) {
        // TODO
        (void)sequence;
    }

    // Devuelve (símbolo, bits consumidos).
    std::pair<Elem, std::uint32_t> decode_one(const BitReader& br, std::size_t pos) const {
        // TODO
        (void)br; (void)pos;
        return {Elem{0}, std::uint32_t{0}};
    }
};

}  // namespace huffman
