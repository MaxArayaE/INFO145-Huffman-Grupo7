# INFO145 Huffman -- Grupo 7

Proyecto semestral del ramo INFO145 *Diseño y Análisis de Algoritmos*. Implementa y compara tres representaciones de arreglos ordenados de gran magnitud:

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

El binario `./main` soporta tres modos.

### 1. Modo benchmark (automático)

```bash
./main --benchmark
```

Genera `bench/results.csv` con tiempos de construcción, tiempos promedio de búsqueda y espacio en bytes para los 3 Casos, las 4 distribuciones (lineal + normal con 3 desviaciones) y los 3 tamaños (10⁶, 10⁷, 10⁸). Tarda aproximadamente 2 minutos en una máquina con 16 GB de RAM.

Columnas del CSV:
```
distribution, n, sigma_factor, case, build_time_ns,
total_search_time_ns, avg_search_time_ns, bytes, bits_per_element
```

### 2. Modo interactivo (REPL)

```bash
./main -i ruta/archivo.csv
```

Lee un CSV de enteros ordenados ascendentemente (uno por línea o coma-separados), construye las 3 estructuras y abre un REPL.

**Comandos del REPL:**

| Comando | Descripción |
|---|---|
| `<num> 1` / `<num> 2` / `<num> 3` | Busca `<num>` en el Caso indicado |
| `<num> all` | Busca en los 3 Casos y compara |
| `stats` | Muestra bytes y bits/elemento de cada Caso |
| `quit` | Salir |

### 3. Modo one-shot (no interactivo)

```bash
./main -q <num> -c <caso> archivo.csv
```

Útil para automatización y tests. `<caso>` puede ser `1`, `2`, `3` o `all`. Ejemplos:

```bash
./main -q 47 -c 1 data/small.csv
./main -q 47 -c all data/small.csv
```

El orden de las flags es libre; el CSV es el único argumento posicional.

### Tipo numérico de entrada

`uint32_t` (rango `0` a `4 294 967 295`), parseado con `std::strtoul`. Valores fuera de rango son rechazados con un warning.

## Estructura del proyecto

```
INFO145-Huffman-Grupo7/
├── main.cpp                    # CLI (3 modos) y orquestación
├── include/
│   ├── common.hpp              # Elem, constantes, Timer
│   ├── generators.hpp          # gen_linear, gen_normal
│   ├── caso1_explicit.hpp      # ExplicitArray<T>
│   ├── caso2_gap_sample.hpp    # GapCodedArray<T>  (GC + Sample)
│   ├── bitstream.hpp           # BitWriter / BitReader (MSB-first)
│   ├── huffman_canonical.hpp   # HuffmanCanonical (build + decode_one)
│   └── caso3_huffman.hpp       # HuffmanCompressedArray
├── bench/                       # CSVs generados por --benchmark
├── data/                        # CSVs de entrada para -i (incluye small.csv)
├── Makefile                     # all, clean, valgrind
└── README.md
```

## Algoritmo asignado: Huffman canónico

Empleamos la variante **canónica** del árbol de Huffman: se conservan solo los largos de código y se reconstruyen códigos canónicos en orden `(largo ascendente, símbolo ascendente)`. Esto permite:

- Modelo compacto: solo `σ` enteros por símbolo, sin nodos del árbol.
- Decodificación en `O(log h)` por símbolo (con `h ≈ log σ`), usando búsqueda binaria sobre la tabla `C[ℓ]` (primer código de cada largo `ℓ`).

El bitstream usa **empaquetado estricto bit a bit** sobre un buffer de `uint64_t` con convención **MSB-first** (el primer bit escrito ocupa el bit más significativo del primer `uint64_t`).

## Resultados resumidos

| Distribución | n | Caso 1 | Caso 2 | Caso 3 (bpe) | Compresión |
|---|---|---|---|---|---|
| lineal | 10⁸ | 32 bpe / 677 ns | 32.003 bpe / 5682 ns | **5.49 bpe** / 50609 ns | 5.8× |
| normal σ=0.05μ | 10⁸ | 32 bpe / 655 ns | 32.003 bpe / 5639 ns | **4.04 bpe** / 119995 ns | **7.9×** |

## Validación

```bash
make valgrind
```

Reporta: 64 allocs, 64 frees, 0 errors, 0 leaks (sobre `data/small.csv`).

## Referencias

- Enunciado: `Tarea/Tarea_2026.pdf`
- Material de clase: `Unidad-II/8_Entropia_Hufman.pdf`
