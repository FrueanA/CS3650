## Mini Shell in C

A Unix-style command-line shell implemented in C using low-level system calls.  
The shell supports executing external programs, built-in commands, and common shell operators such as sequencing, redirection, and pipes.

## Overview

The shell runs in an interactive loop: it displays a prompt, reads user input, parses it into tokens, and executes commands in the foreground. Input parsing supports quoted arguments and multiple operators, enabling behavior similar to standard Unix shells.

## Features

### Core Shell Functionality
- Interactive prompt (`shell $`)
- Executes commands using `fork`, `exec`, and `wait`
- Argument parsing with support for quoted strings
- Graceful handling of unknown commands
- Clean exit via `exit` command or EOF (Ctrl-D)

### Built-in Commands
- `cd` — change working directory
- `source` — execute commands from a script file
- `prev` — re-run the previous command
- `help` — display built-in command descriptions

### Shell Operators
- Sequencing (`;`)
- Input redirection (`<`)
- Output redirection (`>`)
- Pipes (`|`)

Operators can be combined, and execution order follows standard shell precedence.

## Implementation Notes

- Written entirely in C using system calls
- Tokenization and parsing based on a defined shell grammar
- Built-ins handled directly by the shell, not via `exec`
- External commands resolved using the system `PATH`
