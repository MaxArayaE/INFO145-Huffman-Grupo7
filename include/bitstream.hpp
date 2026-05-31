#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace huffman {

// Convención MSB-first: el primer bit escrito ocupa el bit más significativo
// (bit 63) del primer uint64_t. Esto coincide con cómo se interpretan los
// códigos canónicos de Huffman (el primer bit leído es el MSB del código).
class BitWriter {
public:
    explicit BitWriter(std::vector<std::uint64_t>& buf) : buf_(buf), bit_pos_(0) {}

    void write(std::uint64_t value, std::uint32_t nbits) {
        std::size_t word = bit_pos_ >> 6;
        std::uint32_t off = static_cast<std::uint32_t>(bit_pos_ & 63u);

        std::size_t needed = (off + nbits > 64u) ? word + 2 : word + 1;
        if (buf_.size() < needed) buf_.resize(needed, 0ULL);

        std::uint64_t mask = (nbits == 64u) ? ~0ULL : ((1ULL << nbits) - 1ULL);
        std::uint64_t v = value & mask;

        if (off + nbits <= 64u) {
            buf_[word] |= v << (64u - off - nbits);
        } else {
            std::uint32_t hi_bits = 64u - off;
            std::uint32_t lo_bits = nbits - hi_bits;
            buf_[word]     |= v >> lo_bits;
            buf_[word + 1] |= v << (64u - lo_bits);
        }
        bit_pos_ += nbits;
    }

    std::size_t bits_written() const { return bit_pos_; }

private:
    std::vector<std::uint64_t>& buf_;
    std::size_t bit_pos_;
};

class BitReader {
public:
    explicit BitReader(const std::uint64_t* buf) : buf_(buf) {}

    std::uint64_t read(std::size_t pos, std::uint32_t nbits) const {
        std::size_t word = pos >> 6;
        std::uint32_t off = static_cast<std::uint32_t>(pos & 63u);

        if (off + nbits <= 64u) {
            std::uint64_t mask = (nbits == 64u) ? ~0ULL : ((1ULL << nbits) - 1ULL);
            return (buf_[word] >> (64u - off - nbits)) & mask;
        }
        std::uint32_t hi_bits = 64u - off;
        std::uint32_t lo_bits = nbits - hi_bits;
        std::uint64_t hi = (buf_[word] & ((1ULL << hi_bits) - 1ULL)) << lo_bits;
        std::uint64_t lo = buf_[word + 1] >> (64u - lo_bits);
        return hi | lo;
    }

private:
    const std::uint64_t* buf_;
};

}  // namespace huffman
