# Custom Shell with Variable, Alias, and Command Execution Support

This repository contains the source code for a custom shell implementation in C. The shell supports various functionalities, including variable management, aliasing, command execution, input/output redirection, and basic globbing. 

## Features

### 1. Shell Variables
- **Set Variables:** Users can set shell variables to store key-value pairs using `set_var`.
- **Retrieve Variables:** Retrieve variable values with `get_var`.
- **List Variables:** Print all defined variables using `print_vars`.

### 2. Command Aliases
- **Set Alias:** Define an alias for frequently used commands with `set_alias`.
- **Retrieve Alias:** Get the command associated with an alias using `get_alias`.
- **List Aliases:** Display all aliases with `print_aliases`.

### 3. Command Parsing
- Supports parsing user input into arguments, handling spaces and quoted strings properly.
- Handles up to 20 arguments for a single command.

### 4. Command Execution
- Supports the following:
  - **Simple Commands:** Executes commands using `execvp`.
  - **Output Redirection (`>`):** Redirects output to a file.
  - **Input Redirection (`<`):** Reads input from a file.
  - **Pipes (`|`):** Chains commands together, passing the output of one command as input to the next.

### 5. Globbing
- Supports basic globbing with `*`:
  - `*` matches all files in the current directory.
  - `*.extension` matches files with a specific extension.

### 6. Aliases Expansion
- Automatically expands aliases when a matching alias is found for a command.

---

## Getting Started

### Prerequisites
- GCC or any C compiler
- A Unix-like operating system

### Compilation
Compile the program using the following command:

gcc -o custom_shell custom_shell.c


Run the shell using:

./custom_shell

Example Commands

Set a Variable:
set_var("myVar", "Hello, World!")

Retrieve a Variable:
get_var("myVar")  # Output: Hello, World!

Set an Alias:
set_alias("ls", "ls -al")

Execute a Command:
ls

Built-in Functionalities
Define Variables: Store and retrieve shell variables.
Alias Commands: Shorten long or frequently used commands.
Command Execution: Execute programs with optional redirection and pipes.
