#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bitstream.hpp"
#include "common.hpp"
#include "huffman_canonical.hpp"

namespace huffman {

// Caso 3: GC comprimido con Huffman canónico sobre bitstream estricto.
// block_offsets_[k] marca el inicio del bloque k en el bitstream.
class HuffmanCompressedArray {
public:
    void build(const std::vector<Elem>& sorted_input) {
        // TODO
        (void)sorted_input;
    }

    long search(Elem value) const {
        // TODO
        (void)value;
        return -1;
    }

    std::size_t size_in_bytes() const {
        const std::size_t bits_bytes    = bits_.size()          * sizeof(std::uint64_t);
        const std::size_t sample_bytes  = Sample_.size()        * sizeof(Elem);
        const std::size_t offsets_bytes = block_offsets_.size() * sizeof(std::size_t);
        const std::size_t model_bytes =
              huffman_.symbols.size()  * sizeof(Elem)
            + huffman_.codes.size()    * sizeof(std::uint64_t)
            + huffman_.code_len.size() * sizeof(std::uint32_t)
            + huffman_.F.size()        * sizeof(std::uint32_t)
            + huffman_.C.size()        * sizeof(std::uint64_t);
        return bits_bytes + sample_bytes + offsets_bytes + model_bytes;
    }

private:
    std::vector<std::uint64_t> bits_;
    std::vector<std::size_t>   block_offsets_;
    std::vector<Elem>          Sample_;
    HuffmanCanonical           huffman_;
    std::size_t                n_ = 0;
    std::size_t                m_ = 0;
    std::size_t                b_ = 0;
};

}  // namespace huffman
