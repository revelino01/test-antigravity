# Client-Side Renderer Benchmark (C + WASM + Django)

This project contains a high-performance, client-side rendering benchmark built with **C, WebAssembly (WASM), WebGL 2.0, and Django**.

The goal of this web application is to test the raw GPU rendering performance of any device that connects to it (such as a laptop, desktop, or mobile phone) natively within their web browser.

## Architecture & Technical Details

Because modern web browsers operate in a sandbox, they cannot securely execute raw C-compiled native graphical APIs like `vulkan.h` or `d3d12.h`. To test the client device natively, we have to bridge gaps using modern web standards:

1. **The Native C Engine (`benchmark_src/main.c`)**:
   - We utilize **C** to write a computationally heavy Mandelbrot fractal rendering loop.
   - We utilize the **WebGL 2.0** (GLES2) API standard directly within C. (Note: Originally designed for WebGPU, we opted for WebGL due to current instability and API breakages within the Emscripten `emdawnwebgpu` ports).
   - This script runs a mathematical operation (Mandelbrot) within a GLSL Shader directly on the client's GPU, measuring frames per second (FPS).

2. **The Emscripten Compiler (`emcc`)**:
   - Rather than compiling the C code to an `.exe`, we compile it to a **WebAssembly (`.wasm`)** binary.
   - WebAssembly executes at near-native speeds synchronously within the browser, providing highly accurate hardware benchmark metrics.

3. **The Django Backend (`config/`, `benchmark/`)**:
   - Django is used exclusively as the delivery mechanism (Web Server).
   - It hosts the compiled `.wasm` binary securely and serves a beautifully designed, hardware-accelerated HTML dashboard to initialize the benchmark testing UI.

---

## Project Structure

```text
/cpy proj
│
├── benchmark/                      # Django App for the dashboard
│   ├── templates/benchmark/        # Contains the majestic UI (index.html)
│   ├── static/benchmark/           # Stores the compiled C WebAssembly files
│   ├── views.py                    # Django View to serve the dashboard
│   └── urls.py                     # App Routing
│
├── benchmark_src/                  # The actual Native C Benchmark code
│   └── main.c                      # Contains C logic, WebGL initialization, and Shader definitions
│
├── config/                         # Core Django HTTP Settings
│   ├── settings.py                 # Configured to run `benchmark` and serve static files
│   └── urls.py                     # Main Routing configuration
│
├── emsdk/                          # Emscripten SDK Toolchain (Auto-installed locally)
├── manage.py                       # Django CLI execution script
└── README.md                       # Documentation
```

---

## Prerequisites

If you intend to modify the C code and recompile, you need **Python (3.x)** and the **Emscripten SDK (`emsdk`)**.
We have already cloned and configured `emsdk` within this directory.

---

## How to Compile the C Benchmark

If you make any changes to `benchmark_src/main.c` (such as increasing the Mandelbrot iterations for a heavier load), you must recompile the WebAssembly binary.

From the root project directory in PowerShell, execute:
```powershell
# 1. Activate the Emscripten Compiler Environment
. .\emsdk\emsdk_env.ps1

# 2. Compile C to WebAssembly using WebGL 2 bindings
emcc benchmark_src\main.c -s USE_WEBGL2=1 -s WASM=1 -o benchmark\static\benchmark\main.js
```
*(This will overwrite the `main.js` and `main.wasm` files served by Django)*

---

## How to Run the Web Dashboard

To view and interact with the application, you simply need to start the Django web server.

From the root project directory, execute:
```powershell
# Start the local development server unconditionally
python manage.py runserver 0.0.0.0:8000
```

1. Open a Web Browser on your device.
2. Navigate to `http://localhost:8000/` or `http://127.0.0.1:8000/`.
3. Press **"Run Benchmark"** to trigger the C WebAssembly engine and calculate your device's graphic capabilities!

> If you wish to test a completely different device (like your phone), find the IPv4 address of your computer running the server (`ipconfig`), ensure they are on the same Wi-Fi network, and visit `http://[YOUR_IP]:8000/` on the mobile device.
