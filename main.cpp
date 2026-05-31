#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "include/common.hpp"
#include "include/generators.hpp"
#include "include/caso1_explicit.hpp"
#include "include/caso2_gap_sample.hpp"
#include "include/caso3_huffman.hpp"

namespace {

using namespace huffman;

void print_usage(const char* prog) {
    std::fprintf(stderr,
        "INFO145 Huffman -- Grupo 7\n"
        "Uso:\n"
        "  %s --benchmark                       Bateria automatica; escribe bench/results.csv\n"
        "  %s -i archivo.csv                    Modo interactivo (REPL) sobre un CSV ordenado\n"
        "  %s -q <num> -c <caso> archivo.csv    One-shot: busca <num> en <caso> {1|2|3|all}\n",
        prog, prog, prog);
}

// 50% hits (valor existente), 50% misses (uniforme en [data.front(), data.back()]).
std::vector<Elem> generate_queries(const std::vector<Elem>& data, std::uint64_t seed) {
    std::vector<Elem> q;
    q.reserve(NUM_QUERIES);
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::size_t> idx(0, data.size() - 1);
    std::uniform_int_distribution<Elem> val(data.front(), data.back());
    for (std::size_t i = 0; i < NUM_QUERIES; ++i) {
        q.push_back((i & 1u) ? val(rng) : data[idx(rng)]);
    }
    return q;
}

struct BenchmarkResult {
    std::string   distribution;
    std::size_t   n;
    double        sigma_factor;
    int           case_num;
    std::uint64_t build_time_ns;
    std::uint64_t total_search_time_ns;
    double        avg_search_time_ns;
    std::size_t   bytes;
    double        bits_per_element;
};

template <typename Structure>
BenchmarkResult bench_one(int case_num,
                          const std::string& dist,
                          std::size_t n,
                          double sigma_factor,
                          const std::vector<Elem>& data,
                          const std::vector<Elem>& queries) {
    Structure s;
    Timer t_build;
    s.build(data);
    std::uint64_t build_ns = t_build.elapsed_ns();

    long checksum = 0;
    for (std::size_t w = 0; w < WARMUP; ++w) {
        checksum += s.search(queries[w % queries.size()]);
    }

    Timer t_search;
    for (Elem q : queries) {
        checksum += s.search(q);
    }
    std::uint64_t search_ns = t_search.elapsed_ns();
    asm volatile("" : : "r"(checksum));  // evita que -O3 elida el loop de búsquedas

    BenchmarkResult r;
    r.distribution         = dist;
    r.n                    = n;
    r.sigma_factor         = sigma_factor;
    r.case_num             = case_num;
    r.build_time_ns        = build_ns;
    r.total_search_time_ns = search_ns;
    r.avg_search_time_ns   = static_cast<double>(search_ns) / queries.size();
    r.bytes                = s.size_in_bytes();
    r.bits_per_element     = (n > 0) ? static_cast<double>(r.bytes) * 8.0 / n : 0.0;
    return r;
}

void write_csv(const std::vector<BenchmarkResult>& results, const std::string& path) {
    std::FILE* csv = std::fopen(path.c_str(), "w");
    if (!csv) {
        std::fprintf(stderr, "[ERROR] no se pudo crear %s\n", path.c_str());
        return;
    }
    std::fprintf(csv, "distribution,n,sigma_factor,case,build_time_ns,total_search_time_ns,avg_search_time_ns,bytes,bits_per_element\n");
    for (const auto& r : results) {
        std::fprintf(csv, "%s,%zu,%.3f,%d,%llu,%llu,%.2f,%zu,%.4f\n",
                    r.distribution.c_str(), r.n, r.sigma_factor, r.case_num,
                    static_cast<unsigned long long>(r.build_time_ns),
                    static_cast<unsigned long long>(r.total_search_time_ns),
                    r.avg_search_time_ns, r.bytes, r.bits_per_element);
    }
    std::fclose(csv);
}

int run_benchmark() {
    const std::vector<std::size_t> sizes = {1'000'000, 10'000'000, 100'000'000};
    const double mu = static_cast<double>(std::numeric_limits<Elem>::max()) / 2.0;
    const std::vector<double> sigma_factors = {0.05, 0.15, 0.5};

    std::vector<BenchmarkResult> results;

    for (std::size_t n : sizes) {
        std::fprintf(stderr, "[benchmark] n = %zu\n", n);

        // Distribución lineal: epsilon adaptativo para que n*eps no desborde uint32.
        {
            std::uint32_t epsilon = std::max<std::uint32_t>(
                2u, std::numeric_limits<Elem>::max() / static_cast<std::uint32_t>(n));
            auto data    = gen_linear(n, epsilon, SEED_DATA);
            auto queries = generate_queries(data, SEED_QUERIES);

            results.push_back(bench_one<ExplicitArray<Elem>>(1, "linear", n, 0.0, data, queries));
            results.push_back(bench_one<GapCodedArray<Elem>>(2, "linear", n, 0.0, data, queries));
            results.push_back(bench_one<HuffmanCompressedArray>(3, "linear", n, 0.0, data, queries));

            std::fprintf(stderr,
                         "  [linear eps=%u]  C1 %.0fns  C2 %.0fns  C3 %.0fns\n",
                         epsilon,
                         results[results.size()-3].avg_search_time_ns,
                         results[results.size()-2].avg_search_time_ns,
                         results[results.size()-1].avg_search_time_ns);
        }

        // Distribución normal con varios sigma.
        for (double sf : sigma_factors) {
            double sigma = sf * mu;
            auto data    = gen_normal(n, mu, sigma, SEED_DATA);
            auto queries = generate_queries(data, SEED_QUERIES);

            results.push_back(bench_one<ExplicitArray<Elem>>(1, "normal", n, sf, data, queries));
            results.push_back(bench_one<GapCodedArray<Elem>>(2, "normal", n, sf, data, queries));
            results.push_back(bench_one<HuffmanCompressedArray>(3, "normal", n, sf, data, queries));

            std::fprintf(stderr,
                         "  [normal sf=%.2f] C1 %.0fns  C2 %.0fns  C3 %.0fns\n",
                         sf,
                         results[results.size()-3].avg_search_time_ns,
                         results[results.size()-2].avg_search_time_ns,
                         results[results.size()-1].avg_search_time_ns);
        }
    }

    (void)std::system("mkdir -p bench");
    write_csv(results, "bench/results.csv");
    std::fprintf(stderr,
                 "[benchmark] %zu filas escritas. Caso 2/3 = 0 bytes hasta que se implementen.\n",
                 results.size());
    return 0;
}

std::vector<Elem> read_csv(const std::string& path) {
    std::vector<Elem> data;
    std::ifstream in(path);
    if (!in.is_open()) {
        std::fprintf(stderr, "[ERROR] no se pudo abrir %s\n", path.c_str());
        return data;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::size_t pos = 0;
        while (pos < line.size()) {
            char* end = nullptr;
            unsigned long v = std::strtoul(line.c_str() + pos, &end, 10);
            if (end == line.c_str() + pos) break;
            if (v > std::numeric_limits<Elem>::max()) {
                std::fprintf(stderr, "[WARN] %lu fuera de rango uint32, ignorado\n", v);
            } else {
                data.push_back(static_cast<Elem>(v));
            }
            pos = static_cast<std::size_t>(end - line.c_str());
            while (pos < line.size() &&
                   (line[pos] == ',' || line[pos] == ' ' || line[pos] == '\t')) {
                ++pos;
            }
        }
    }
    return data;
}

bool validate_sorted(const std::vector<Elem>& data) {
    for (std::size_t i = 1; i < data.size(); ++i) {
        if (data[i] < data[i - 1]) {
            std::fprintf(stderr, "[ERROR] CSV no ordenado ascendentemente (pos %zu)\n", i);
            return false;
        }
    }
    return true;
}

template <typename Structure>
void search_and_print(int case_num, Elem v, const std::vector<Elem>& data) {
    Structure s;
    s.build(data);
    Timer t;
    long pos = s.search(v);
    std::uint64_t ns = t.elapsed_ns();
    if (pos >= 0) {
        std::printf("[Caso %d] %u en pos %ld (%llu ns)\n",
                    case_num, v, pos, static_cast<unsigned long long>(ns));
    } else {
        std::printf("[Caso %d] %u NO encontrado (%llu ns)\n",
                    case_num, v, static_cast<unsigned long long>(ns));
    }
}

void print_stats(std::size_t n, std::size_t bytes_c1, std::size_t bytes_c2, std::size_t bytes_c3) {
    auto bpe = [n](std::size_t b) { return n > 0 ? b * 8.0 / static_cast<double>(n) : 0.0; };
    std::printf("Caso 1: %zu bytes (%.2f bits/elem)\n", bytes_c1, bpe(bytes_c1));
    std::printf("Caso 2: %zu bytes (%.2f bits/elem)\n", bytes_c2, bpe(bytes_c2));
    std::printf("Caso 3: %zu bytes (%.2f bits/elem)\n", bytes_c3, bpe(bytes_c3));
}

int run_interactive(const std::string& csv_path) {
    auto data = read_csv(csv_path);
    if (data.empty()) {
        std::fprintf(stderr, "[ERROR] CSV vacio o invalido\n");
        return 1;
    }
    if (!validate_sorted(data)) return 1;
    std::fprintf(stderr, "[ok] %zu elementos cargados\n", data.size());

    ExplicitArray<Elem>     c1;
    GapCodedArray<Elem>     c2;
    HuffmanCompressedArray  c3;
    c1.build(data);
    c2.build(data);
    c3.build(data);
    std::fprintf(stderr, "[ok] Casos 1, 2 y 3 construidos\n");

    std::printf("Comandos: <num> {1|2|3|all}  |  stats  |  quit\n");
    std::string line;
    while (true) {
        std::cout << "huffman> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        if (line == "quit") break;
        if (line == "stats") {
            print_stats(data.size(), c1.size_in_bytes(), c2.size_in_bytes(), c3.size_in_bytes());
            continue;
        }

        std::istringstream iss(line);
        unsigned long val;
        std::string which;
        if (!(iss >> val >> which)) {
            std::printf("comando invalido (use: <num> {1|2|3|all})\n");
            continue;
        }
        if (val > std::numeric_limits<Elem>::max()) {
            std::printf("valor fuera de rango uint32\n");
            continue;
        }
        Elem v = static_cast<Elem>(val);

        auto run_search = [v](auto& s, int c) {
            Timer t;
            long pos = s.search(v);
            std::uint64_t ns = t.elapsed_ns();
            if (pos >= 0) {
                std::printf("[Caso %d] %u en pos %ld (%llu ns)\n",
                            c, v, pos, static_cast<unsigned long long>(ns));
            } else {
                std::printf("[Caso %d] %u NO encontrado (%llu ns)\n",
                            c, v, static_cast<unsigned long long>(ns));
            }
        };

        if      (which == "1")   run_search(c1, 1);
        else if (which == "2")   run_search(c2, 2);
        else if (which == "3")   run_search(c3, 3);
        else if (which == "all") { run_search(c1, 1); run_search(c2, 2); run_search(c3, 3); }
        else std::printf("caso desconocido: %s\n", which.c_str());
    }
    return 0;
}

int run_one_shot(const std::string& csv_path,
                 const std::string& value_str,
                 const std::string& case_str) {
    auto data = read_csv(csv_path);
    if (data.empty()) {
        std::fprintf(stderr, "[ERROR] CSV vacio o invalido\n");
        return 1;
    }
    if (!validate_sorted(data)) return 1;

    char* end = nullptr;
    unsigned long val = std::strtoul(value_str.c_str(), &end, 10);
    if (end == value_str.c_str() || *end != '\0' || val > std::numeric_limits<Elem>::max()) {
        std::fprintf(stderr, "[ERROR] valor invalido o fuera de uint32: %s\n", value_str.c_str());
        return 1;
    }
    Elem v = static_cast<Elem>(val);

    if (case_str != "1" && case_str != "2" && case_str != "3" && case_str != "all") {
        std::fprintf(stderr, "[ERROR] caso desconocido: %s (use 1, 2, 3, o all)\n", case_str.c_str());
        return 1;
    }

    if (case_str == "1" || case_str == "all") search_and_print<ExplicitArray<Elem>>(1, v, data);
    if (case_str == "2" || case_str == "all") search_and_print<GapCodedArray<Elem>>(2, v, data);
    if (case_str == "3" || case_str == "all") search_and_print<HuffmanCompressedArray>(3, v, data);
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    const std::string first = argv[1];
    if (first == "--benchmark")         return run_benchmark();
    if (first == "-i" && argc >= 3)     return run_interactive(argv[2]);

    // Modo one-shot: parsear -q <num> -c <caso> y un argumento posicional (CSV).
    std::string csv_path, value_str, case_str;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "-q" && i + 1 < argc) value_str = argv[++i];
        else if (a == "-c" && i + 1 < argc) case_str  = argv[++i];
        else if (!a.empty() && a[0] != '-') csv_path  = a;
        else { print_usage(argv[0]); return 1; }
    }
    if (!value_str.empty() && !case_str.empty() && !csv_path.empty()) {
        return run_one_shot(csv_path, value_str, case_str);
    }

    print_usage(argv[0]);
    return 1;
}
