# ♟️ Chess Engine (Bitboard-Based)

[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Build](https://img.shields.io/badge/build-Clang%20%7C%20GCC-lightgrey.svg)](https://clang.llvm.org/)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![Status](https://img.shields.io/badge/status-early--development-yellow.svg)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A **high-performance chess engine** written in **pure C**, built to explore **bitboard techniques**, **move generation**, and **low-level performance optimization**.  
Focused on **clarity**, **speed**, and **systems-level design**.

## ⚡ Core Highlights

- 🧩 **Bitboard architecture** — 64-bit board representation for all pieces  
- ⚔️ **Precomputed attack masks** for pawns, knights, and kings  
- ♙ **Pawn structure evaluation** — doubled, isolated, and blocked pawns  
- ♜ **Material and mobility scoring**  
- ⚙️ **Optimized bitwise operations** — popcount, bit scans, and masks  
- 🧠 *Planned:* sliding attacks, move generation, minimax + alpha-beta, UCI protocol  


## 🎯 Goal

This project aims to understand chess engine internals — from efficient bitboard representation to search algorithms — and to build a clean, extensible base for future experiments in engine evaluation and AI search.

## 🛠️ Build System

The project uses a custom **build script** with multiple modes:

| Command | Description |
|----------|--------------|
| `build` or `build debug` | No optimization, full debug info (`-O0 -g`) |
| `build optdebug` | Optimized build with debug symbols (`-O2 -g`) |
| `build release` | Fully optimized, stripped binary (`-O3 -DNDEBUG`) |

### 🧩 Example

```bash
build release

# or simple

./build.bat
```

Output binaries are placed in the build/ directory.