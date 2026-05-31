#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <vector>

#include "common.hpp"

namespace huffman {

// Caso 2: Gap-Coding + Sample. m = ceil(sqrt(n)), b = ceil(n / m).
template <typename T = Elem>
class GapCodedArray {
public:
    void build(const std::vector<T>& sorted_input) {
        n_ = sorted_input.size();
        GC_.clear();
        Sample_.clear();
        if (n_ == 0) {
            m_ = 0;
            b_ = 0;
            return;
        }

        m_ = static_cast<std::size_t>(
                std::ceil(std::sqrt(static_cast<double>(n_))));
        if (m_ > n_) m_ = n_;
        b_ = (n_ + m_ - 1) / m_;

        GC_.resize(n_);
        GC_[0] = sorted_input[0];
        for (std::size_t i = 1; i < n_; ++i) {
            GC_[i] = sorted_input[i] - sorted_input[i - 1];
        }

        Sample_.reserve(m_);
        for (std::size_t k = 0; k < m_; ++k) {
            Sample_.push_back(sorted_input[k * b_]);
        }
    }

    long search(T value) const {
        if (n_ == 0) return -1;

        auto lo = std::lower_bound(Sample_.begin(), Sample_.end(), value);
        std::size_t k = static_cast<std::size_t>(
                std::distance(Sample_.begin(), lo));

        if (k == 0) {
            return (Sample_[0] == value) ? 0L : -1L;
        }

        std::size_t L = (k - 1) * b_;
        std::size_t R = std::min(L + b_, n_);

        T acc = Sample_[k - 1];
        if (acc == value) return static_cast<long>(L);

        for (std::size_t i = L + 1; i < R; ++i) {
            acc += GC_[i];
            if (acc == value) return static_cast<long>(i);
            if (acc > value)  return -1;
        }

        // Borde: valor podría estar exactamente en la posición k*b (sample siguiente).
        if (k < m_ && Sample_[k] == value) return static_cast<long>(k * b_);
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
