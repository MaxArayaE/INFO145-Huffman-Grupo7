#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
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
        bits_.clear();
        block_offsets_.clear();
        Sample_.clear();
        n_ = sorted_input.size();
        if (n_ == 0) { m_ = 0; b_ = 0; return; }

        m_ = static_cast<std::size_t>(
                std::ceil(std::sqrt(static_cast<double>(n_))));
        if (m_ > n_) m_ = n_;
        b_ = (n_ + m_ - 1) / m_;

        Sample_.reserve(m_);
        for (std::size_t k = 0; k < m_; ++k) {
            Sample_.push_back(sorted_input[k * b_]);
        }

        // Recolectar gaps internos (los inter-bloque NO se encodean: el sample los tiene).
        std::vector<Elem> gaps;
        gaps.reserve(n_ > m_ ? n_ - m_ : 0);
        for (std::size_t k = 0; k < m_; ++k) {
            std::size_t L = k * b_;
            std::size_t R = std::min(L + b_, n_);
            for (std::size_t i = L + 1; i < R; ++i) {
                gaps.push_back(sorted_input[i] - sorted_input[i - 1]);
            }
        }

        huffman_.build(gaps);

        // Encodear bloque por bloque, registrando offsets
        block_offsets_.resize(m_);
        BitWriter writer(bits_);
        std::size_t gap_idx = 0;
        for (std::size_t k = 0; k < m_; ++k) {
            block_offsets_[k] = writer.bits_written();
            std::size_t L = k * b_;
            std::size_t R = std::min(L + b_, n_);
            for (std::size_t i = L + 1; i < R; ++i) {
                Elem g = gaps[gap_idx++];
                auto it = huffman_.sym_to_idx.find(g);
                std::uint32_t idx = it->second;
                writer.write(huffman_.codes[idx], huffman_.code_len[idx]);
            }
        }
        // Padding: decode_one lee h_max bits adelante; aseguramos un word extra de ceros.
        bits_.push_back(0ULL);
    }

    long search(Elem value) const {
        if (n_ == 0) return -1;

        auto lo = std::lower_bound(Sample_.begin(), Sample_.end(), value);
        std::size_t k = static_cast<std::size_t>(
                std::distance(Sample_.begin(), lo));

        if (k == 0) {
            return (Sample_[0] == value) ? 0L : -1L;
        }

        std::size_t L = (k - 1) * b_;
        std::size_t R = std::min(L + b_, n_);

        Elem acc = Sample_[k - 1];
        if (acc == value) return static_cast<long>(L);

        BitReader br(bits_.data());
        std::size_t pos = block_offsets_[k - 1];

        for (std::size_t i = L + 1; i < R; ++i) {
            auto [gap, len] = huffman_.decode_one(br, pos);
            pos += len;
            acc += gap;
            if (acc == value) return static_cast<long>(i);
            if (acc > value)  return -1;
        }

        if (k < m_ && Sample_[k] == value) return static_cast<long>(k * b_);
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
