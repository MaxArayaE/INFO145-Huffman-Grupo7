# INFO145 Huffman -- Grupo 7

Proyecto semestral del ramo INFO145 *Diseño y Análisis de Algoritmos* (Prof. Héctor Ferrada, UACh, primer semestre 2026). Implementa y compara tres representaciones de arreglos ordenados de gran magnitud:

1. **Caso 1**: representación explícita (línea base, búsqueda binaria).
2. **Caso 2**: Gap-Coding con índice de muestreo (Sample).
3. **Caso 3**: compresión del Gap-Coding con **códigos canónicos de Huffman** sobre bitstream estricto (algoritmo asignado al grupo).

## Compilación

```bash
make
```

Requiere `g++` con soporte C++17. Flags por defecto: `-O3 -std=c++17 -Wall -Wextra -Wpedantic -march=native`.

Para limpiar: `make clean`. Para verificar fugas de memoria con un dataset pequeño: `make valgrind` (requiere `valgrind` y `data/small.csv`).

## Ejecución

### Modo benchmark
```bash
./main --benchmark
```

Genera `bench/results.csv` con tiempos de construcción, tiempos de búsqueda y espacio en bytes para cada caso, distribución y tamaño. Las columnas son:

```
distribution, n, sigma, case, build_time_ns,
total_search_time_ns, avg_search_time_ns, bytes, bits_per_element
```

### Modo archivo (interactivo)

```bash
./main -i ruta/archivo.csv
```

Lee un CSV de enteros ordenados ascendentemente (uno por línea o coma-separados), construye las 3 estructuras y abre un REPL.

**Comandos del REPL:**

| Comando | Descripción |
|---|---|
| `<num> 1` / `<num> 2` / `<num> 3` | Busca `<num>` en el Caso indicado |
| `<num> all` | Busca en los 3 Casos y compara resultados/tiempos |
| `stats` | Muestra bytes y bits/elemento de cada Caso |
| `quit` | Salir |

**Tipo numérico de entrada**: `uint32_t` (rango `0` a `4 294 967 295`), parseado con `std::strtoul`. Valores fuera de rango son rechazados.

## Estructura del proyecto

```
INFO145-Huffman-Grupo7/
├── main.cpp                    # CLI y orquestación de los 2 modos
├── include/
│   ├── common.hpp              # Elem, constantes, Timer
│   ├── generators.hpp          # gen_linear, gen_normal
│   ├── caso1_explicit.hpp      # ExplicitArray<T>
│   ├── caso2_gap_sample.hpp    # GapCodedArray<T>  (GC + Sample)
│   ├── bitstream.hpp           # BitWriter / BitReader
│   ├── huffman_canonical.hpp   # HuffmanCanonical (build + decode tables)
│   └── caso3_huffman.hpp       # HuffmanCompressedArray
├── bench/                       # CSVs generados por --benchmark
├── data/                        # CSVs de entrada para -i
├── Makefile                     # all, clean, valgrind
└── README.md

## Algoritmo asignado: Huffman canónico

A diferencia de la versión "plana" del árbol de Huffman (que requiere mantener punteros), usamos la versión **canónica**: se conservan solo los **largos de código** y se reconstruyen códigos canónicos en orden `(largo ascendente, símbolo ascendente)`.

Esto permite:

- Modelo compacto: solo `Σ` enteros por símbolo, sin nodos del árbol.
- Decodificación en `O(log h)` por símbolo (con `h ≈ log Σ`), usando búsqueda binaria sobre la tabla `C[ℓ]` (primer código de cada largo `ℓ`).

El bitstream usa **empaquetado estricto bit a bit** sobre un buffer de `uint64_t` (no la simplificación byte-packed permitida por el enunciado).

## Referencias

- Enunciado: `Tarea/Tarea_2026.pdf`
- Material de clase: `Unidad-II/8_Entropia_Hufman.pdf`
