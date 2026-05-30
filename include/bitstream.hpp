#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace huffman {

class BitWriter {
public:
    explicit BitWriter(std::vector<std::uint64_t>& buf) : buf_(buf), bit_pos_(0) {}

    void write(std::uint64_t value, std::uint32_t nbits) {
        // TODO
        (void)value; (void)nbits;
    }

    std::size_t bits_written() const { return bit_pos_; }

private:
    std::vector<std::uint64_t>& buf_;
    std::size_t bit_pos_;
};

// Lector stateless: la posición se pasa explícita en cada llamada.
class BitReader {
public:
    explicit BitReader(const std::uint64_t* buf) : buf_(buf) {}

    std::uint64_t read(std::size_t pos, std::uint32_t nbits) const {
        // TODO
        (void)pos; (void)nbits;
        return 0;
    }

private:
    const std::uint64_t* buf_;
};

}  // namespace huffman
