# bitWise Browser

bitWise Browser is an experimental, lightweight web browser engine built from scratch in C++. It features a custom DOM implementation, a hardware-accelerated visual rendering engine powered by **Raylib**, and an integrated **Lua 5.4** runtime that acts as the "Director" for UI and DOM manipulation.

## Features

- **Custom DOM Implementation:** A lightweight C++ document object model (`JSONDocument`).
- **Raylib Visual Engine:** Hardware-accelerated UI rendering for fast and efficient graphics.
- **Lua Scripting Runtime:** A built-in Lua 5.4 Director script environment allowing programmatic access to the browser's DOM and UI elements (e.g., fetching elements by ID, modifying styles).
- **Custom Parser & Network Stack:** Modular C++ architecture separating parsing, networking, and UI rendering.

## Prerequisites

To build and run bitWise Browser, you need the following dependencies installed on your system:

- **C++ Compiler** supporting C++17 (e.g., `g++`)
- **Make**
- **Raylib** (`libraylib`)
- **Lua 5.4** (`liblua5.4`)
- Standard Linux libraries: `libGL`, `libm`, `libpthread`, `libdl`, `librt`, `libX11`

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential libraylib-dev liblua5.4-dev
```
*(Note: Raylib might need to be built from source depending on your package manager version).*

## Building the Browser

You can build the project using the provided `Makefile`.

1. Clone the repository and navigate to the project directory:
   ```bash
   cd bitWise_browser
   ```

2. Compile the project:
   ```bash
   make
   ```

Alternatively, you can use the included build script:
```bash
./run.sh
```

## Running the Browser

After building, a `browser` executable will be created in the current directory. Run it using:

```bash
./browser
```

When started, the engine initializes the C++ DOM, connects the Lua Director via the `script.lua` file, and launches the visual engine.

## Project Structure

- `main.cpp` - The entry point that initializes the DOM, Lua state, and visual engine.
- `ui/` - Contains the Raylib-based visual rendering engine.
- `parser/` - Contains the custom document parser.
- `network/` - Handles networking capabilities.
- `includes/` - C++ header files exposing internal APIs.
- `script.lua` - The default Lua Director script that interacts with the browser's DOM API.
- `Browser.pdf` / `LuaBrowser Runtime.pdf` - Project documentation and architecture details.
