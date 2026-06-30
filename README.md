# Neural Network Art Images

A free/open-source C++ desktop app for exploring art images with a pretrained neural network.

The first version is designed around:

- **Qt 6 Community/Open Source** for the desktop UI.
- **OpenCV DNN** for CPU-only ONNX inference.
- **mlpack** as an optional open-source clustering backend.
- **CMake** for building.
- A local image library rooted at `C:\Users\jmars\OneDrive\Pictures\Art Images`.

## What It Does

- Scans the art image folder tree.
- Treats folder names as optional labels when they look useful.
- Runs a pretrained ONNX image model with OpenCV DNN.
- Shows image thumbnails in a Qt desktop app.
- Lets you select an image and find similar images by cosine similarity.
- Shows category predictions and confidence scores.
- Clusters images using embeddings.
- Exports predictions, embeddings, and cluster data to CSV or JSON.

## Model Choice

Use an open-source pretrained ONNX image model, such as MobileNetV2, EfficientNet-Lite, or ResNet-50 trained on ImageNet. The app does not bundle model weights, because model files can be large and each model has its own license. Put your chosen model and label file in `models/`, for example:

```text
models/
  mobilenetv2.onnx
  imagenet_labels.txt
```

OpenCV DNN runs these models on CPU, so no CUDA/NVIDIA setup is required.

## Dependencies

All recommended dependencies are free/open source:

- Qt 6 Community/Open Source: LGPL/GPL
- OpenCV: Apache 2.0
- mlpack: BSD 3-Clause
- CMake: BSD 3-Clause

On Windows, the easiest dependency setup is usually `vcpkg`:

```powershell
vcpkg install qtbase:x64-windows opencv4:x64-windows mlpack:x64-windows
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -DARTNET_USE_MLPACK=ON
cmake --build build --config Release
```

If vcpkg cannot provide a CMake package for mlpack yet, the app can still build with the built-in clustering fallback:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -DARTNET_USE_MLPACK=OFF
cmake --build build --config Release
```

To force CMake to fail when mlpack is requested but unavailable:

```powershell
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -DARTNET_USE_MLPACK=ON -DARTNET_REQUIRE_MLPACK=ON
```

If Visual Studio/MSBuild reports `Item has already been added. Key in dictionary: 'Path' Key being added: 'PATH'`, open a fresh **Developer PowerShell for Visual Studio** and rerun the build. That error comes from duplicate `Path`/`PATH` environment variables in the current shell process, not from the project source.

If the app starts with `Could not find the Qt platform plugin "windows"`, deploy the Qt runtime files beside the executable:

```powershell
New-Item -ItemType Directory -Force .\build\Release\platforms
Copy-Item C:\vcpkg\installed\x64-windows\Qt6\plugins\platforms\qwindows.dll .\build\Release\platforms\
```

The project also tries to copy this plugin automatically after successful Windows builds. If your Qt install includes `windeployqt.exe`, you can use that instead:

```powershell
windeployqt .\build\Release\artnet_desktop.exe
```

## Build Core Tests Without GUI/ML Dependencies

Unit tests live in the unified `tests/` directory. The current core test executable is `tests/CoreTests.cpp`, and it avoids Qt and OpenCV so the data/index/export logic can be checked early:

```powershell
cmake -S . -B build-core -DARTNET_BUILD_APP=OFF
cmake --build build-core
ctest --test-dir build-core --output-on-failure
```

To collect coverage with GCC or Clang, enable the coverage option and run `gcovr`:

```powershell
cmake -S . -B build-core -DARTNET_BUILD_APP=OFF -DARTNET_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core
ctest --test-dir build-core --output-on-failure
gcovr --root . --filter src/ --exclude tests/ --print-summary --fail-under-line 90
```

## CI Pipeline

The GitHub Actions workflow in `.github/workflows/ci.yml` runs on pushes, pull requests, and manual dispatches. It is organized into:

- **Unit Tests**: configures a GUI-free core build, runs `ctest`, produces coverage with `gcovr`, uploads the coverage report, and fails if line coverage drops below 90%.
- **Code Scanning / Security**: runs GitHub CodeQL for C++ with the `security-extended` query suite.
- **Code Scanning / Quality**: runs GitHub CodeQL for C++ with the `security-and-quality` query suite and publishes results under a separate quality category.

CodeQL code scanning is free for public repositories. Private repository availability depends on the repository's GitHub plan and Advanced Security settings.

## Current Scope

This is a scaffolded first version. It has the architecture and core behavior in place, with a Qt/OpenCV app path ready for real models once dependencies and ONNX weights are installed.
