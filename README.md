# SQTerm

> ⚠️ **Note:** This project is quite old and is being published out of inertia for another user. It may still be useful for someone, but no active development is planned. Use at your own discretion.

**SQTerm** is a lightweight, embeddable serial terminal widget library for Qt5/C++20 applications. It provides a ready-to-use terminal component that can be easily integrated into any Qt project.

## Features

- 🔌 **Embeddable** – Use as a standalone widget in your Qt applications
- 📡 **Serial port support** – Built on Qt5SerialPort
- 🎨 **Customizable styling** – Supports QSS theming (example with DarkStyle included)
- ⚡ **Lightweight** – Minimal dependencies, just Qt5 Core, Widgets, and SerialPort
- 🧩 **Modern CMake** – Easy integration with `find_package` and `target_link_libraries`

## Quick Start with CMakePresets.json

This project uses `CMakePresets.json` for easy configuration. To build:

```bash
# 1. Clone the repository
git clone <repo-url>
cd sqterm

# 2. Create your user preset from the example
cp CMakeUserPresets.example.json CMakeUserPresets.json

# 3. Edit CMakeUserPresets.json and set your Qt path:
#    "CMAKE_PREFIX_PATH": "/home/your_user/Qt/5.15.2/gcc_64"

# 4. Configure and build
cmake --preset qt-linux-user
cmake --build --preset qt-linux-release
