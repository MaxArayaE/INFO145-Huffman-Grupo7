#include <cstdio>
#include <cstdlib>
#include <string>

#include "include/common.hpp"
#include "include/generators.hpp"
#include "include/caso1_explicit.hpp"
#include "include/caso2_gap_sample.hpp"
#include "include/caso3_huffman.hpp"

namespace {

void print_usage(const char* prog) {
    std::fprintf(stderr,
        "INFO145 Huffman -- Grupo 7\n"
        "Uso:\n"
        "  %s --benchmark           Ejecuta la bateria automatica de pruebas y\n"
        "                           escribe bench/results.csv\n"
        "  %s -i archivo.csv        Modo interactivo: lee el CSV, construye los 3\n"
        "                           Casos y abre un REPL para buscar valores\n",
        prog, prog);
}

int run_benchmark() {
    std::fprintf(stderr, "[--benchmark] no implementado todavia\n");
    return 1;
}

int run_interactive(const std::string& csv_path) {
    std::fprintf(stderr, "[-i %s] no implementado todavia\n", csv_path.c_str());
    return 1;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    const std::string mode = argv[1];
    if (mode == "--benchmark") {
        return run_benchmark();
    }
    if (mode == "-i" && argc >= 3) {
        return run_interactive(argv[2]);
    }
    print_usage(argv[0]);
    return 1;
}
