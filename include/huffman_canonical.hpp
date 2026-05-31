#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <queue>
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
    std::vector<std::uint64_t> C;        // C[l] = primer código de largo l (sentinela en h_max+1)
    std::uint32_t              h_min = 0;
    std::uint32_t              h_max = 0;
    std::unordered_map<Elem, std::uint32_t> sym_to_idx;

    void build(const std::vector<Elem>& sequence) {
        if (sequence.empty()) { reset(); return; }
        std::unordered_map<Elem, std::uint64_t> freq;
        freq.reserve(sequence.size() / 4 + 1);
        for (Elem s : sequence) ++freq[s];
        build_from_freq(freq);
    }

    void build_from_freq(const std::unordered_map<Elem, std::uint64_t>& freq) {
        reset();
        if (freq.empty()) return;

        // Construir árbol con min-heap. Tie-breaking determinista por índice.
        struct Node { std::uint64_t f; std::int32_t left; std::int32_t right; Elem sym; };
        std::vector<Node> nodes;
        nodes.reserve(2 * freq.size());

        using PQItem = std::pair<std::uint64_t, std::int32_t>; // (freq, node_idx)
        std::priority_queue<PQItem, std::vector<PQItem>, std::greater<PQItem>> pq;

        for (const auto& kv : freq) {
            std::int32_t idx = static_cast<std::int32_t>(nodes.size());
            nodes.push_back({kv.second, -1, -1, kv.first});
            pq.push({kv.second, idx});
        }

        // Caso especial: un único símbolo → forzar largo 1.
        std::unordered_map<Elem, std::uint32_t> sym_to_len;
        sym_to_len.reserve(freq.size());

        if (freq.size() == 1) {
            sym_to_len[nodes[0].sym] = 1;
        } else {
            while (pq.size() >= 2) {
                auto a = pq.top(); pq.pop();
                auto b = pq.top(); pq.pop();
                std::int32_t idx = static_cast<std::int32_t>(nodes.size());
                nodes.push_back({a.first + b.first, a.second, b.second, Elem{0}});
                pq.push({a.first + b.first, idx});
            }
            std::int32_t root = pq.top().second;

            // Asignar largos por DFS iterativo
            std::vector<std::pair<std::int32_t, std::uint32_t>> stack;
            stack.reserve(nodes.size());
            stack.push_back({root, 0u});
            while (!stack.empty()) {
                auto [n, d] = stack.back();
                stack.pop_back();
                if (nodes[n].left == -1) {
                    sym_to_len[nodes[n].sym] = d;
                } else {
                    stack.push_back({nodes[n].left,  d + 1});
                    stack.push_back({nodes[n].right, d + 1});
                }
            }
        }

        // Pares (sym, len) ordenados por (len asc, sym asc)
        std::vector<std::pair<Elem, std::uint32_t>> pairs;
        pairs.reserve(sym_to_len.size());
        for (const auto& kv : sym_to_len) pairs.push_back({kv.first, kv.second});
        std::sort(pairs.begin(), pairs.end(),
                  [](const auto& a, const auto& b) {
                      return a.second != b.second ? a.second < b.second
                                                  : a.first  < b.first;
                  });

        symbols.reserve(pairs.size());
        code_len.reserve(pairs.size());
        for (const auto& p : pairs) {
            symbols.push_back(p.first);
            code_len.push_back(p.second);
        }
        h_min = code_len.front();
        h_max = code_len.back();
        assert(h_max < 64u);

        // count[l] = símbolos de largo l
        std::vector<std::uint32_t> count(h_max + 2u, 0u);
        for (std::uint32_t l : code_len) ++count[l];

        // Tablas F y C
        F.assign(h_max + 2u, 0u);
        C.assign(h_max + 2u, 0ULL);
        std::uint32_t cum = 0;
        for (std::uint32_t l = h_min; l <= h_max; ++l) {
            F[l] = cum;
            cum += count[l];
        }
        for (std::uint32_t l = h_min; l <= h_max; ++l) {
            if (l == h_min) C[l] = 0;
            else            C[l] = (C[l - 1] + count[l - 1]) << 1;
        }

        // Códigos canónicos: code[F[l] + j] = C[l] + j
        codes.resize(symbols.size());
        for (std::uint32_t l = h_min; l <= h_max; ++l) {
            for (std::uint32_t j = 0; j < count[l]; ++j) {
                codes[F[l] + j] = C[l] + j;
            }
        }

        // Mapa de encode
        sym_to_idx.reserve(symbols.size());
        for (std::uint32_t i = 0; i < symbols.size(); ++i) {
            sym_to_idx.emplace(symbols[i], i);
        }
    }

private:
    void reset() {
        symbols.clear(); codes.clear(); code_len.clear();
        F.clear(); C.clear(); sym_to_idx.clear();
        h_min = h_max = 0;
    }

public:
    // Devuelve (símbolo, bits consumidos).
    std::pair<Elem, std::uint32_t> decode_one(const BitReader& br, std::size_t pos) const {
        std::uint64_t N = br.read(pos, h_max);

        // Binary search sobre C[h_min..h_max]: mayor l con C[l]·2^(h_max-l) <= N
        std::uint32_t lo = h_min, hi = h_max;
        while (lo < hi) {
            std::uint32_t mid = (lo + hi + 1u) / 2u;
            std::uint64_t boundary = C[mid] << (h_max - mid);
            if (boundary <= N) lo = mid;
            else               hi = mid - 1u;
        }
        std::uint32_t l = lo;
        std::uint64_t Nprime = N >> (h_max - l);
        Elem sym = symbols[F[l] + static_cast<std::uint32_t>(Nprime - C[l])];
        return {sym, l};
    }
};

}  // namespace huffman
