## Vector & Shell Tokenizer in C

A systems programming project implementing a **resizable vector (dynamic array)** and a **command-line tokenizer** in C. The project focuses on efficient memory management, ownership semantics, and lexical analysis similar to Unix shells.

## Overview

This project consists of two main components:

1. A **vector data structure** that provides dynamic resizing with O(1) average-time access.
2. A **shell-style tokenizer** built on top of the vector, capable of splitting command-line input into meaningful tokens.

Both components emphasize correctness, memory safety, and clear API design.

## Vector (Dynamic Array)

The vector is a resizable, contiguous array of strings (`char *`), similar to `ArrayList` in Java or `std::vector` in C++.

### Key Properties
- Stores elements contiguously for efficient indexing
- Automatically grows using a configurable growth factor
- Separates **logical size** from **capacity**
- Owns its elements (strings are copied on insertion)
- Safely frees memory when elements are removed or the vector is destroyed

The API supports adding, retrieving, copying, and deleting elements, and is validated using unit tests built with the μunit framework.

## Tokenizer

The tokenizer processes a single line of input and splits it into tokens following shell-like syntax rules.

### Supported Tokens
- Words (sequences of non-special characters)
- Special characters: `(` `)` `<` `>` `;` `|`
- Quoted strings, which suppress special character meaning
- Whitespace as a separator (not a token)

The tokenizer is implemented as a **deterministic finite-state automaton (DFA)** and outputs one token per line.
