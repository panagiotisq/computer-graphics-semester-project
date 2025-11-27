# 🏔️ Mountain Hiking: Procedural Graphics Semester Project

## 🎯 Project Overview

This is a comprehensive computer graphics project, developed in C++ and OpenGL, focusing on procedurally generating a dynamic, high-altitude hiking experience. The project emphasizes advanced techniques in environment simulation, atmospheric effects, and visual distortion.

## 💡 Features & Graphical Techniques

The project is structured into two main parts, demonstrating a broad range of graphics concepts:

### Part A: Environmental Simulation

| Feature | Graphics Concept |
| :--- | :--- |
| **1. High-Altitude Terrain** | Random generation of detailed, large-scale mountainous geometry. |
| **2. Textured Ground & Vegetation** | Applying tiling textures with **random rotation/flipping** to eliminate repetitiveness. Sparse vegetation (trees/plants) is added for detail. |
| **3. First-Person Controller** | Implementation of a character controller, allowing ground-based movement (walking/climbing, not flying). |
| **4. Directional Lighting** | Simulating the sun's position and global light direction. |
| **5. Skybox** | Creating an infinite, surrounding backdrop for the scene. |

### Part B: Dynamic and Advanced Effects

| Feature | Graphics Concept |
| :--- | :--- |
| **6. Wind Simulation** | Player **movement speed is reduced based on altitude** (stronger wind). Accompanied by **wind howling sound effects** tied to the character's height. |
| **7. View Bobbing** | Camera movement simulating the character's footsteps, making the wind's slowing effect more prominent. |
| **8. Altitude Fog** | Implementation of a **fog effect concentrated near mountain peaks**. |
| **9. Flying Particles** | Particle system simulation of dust and debris carried by the wind. |
| **10. Rotational World Distortion** | Advanced visual effect where the entire world is **rotated and distorted** around the user, simulating a visual anomaly to be escaped. |
| **11. Voxelizing Scene Destruction** | An effect that simulates scene destruction by **voxelizing and quantizing** the geometry of the scene and objects. |

### 💎 Bonus Features

* **Swaying Vegetation:** Realistic plant movement implemented via the **vertex shader**, with the intensity of the sway increasing with altitude.
* **Screen Space Motion Blur:** Applying a directional blur effect aligned with the wind direction.

---

## 🛠️ Building the Project

The project uses the **CMake** build system to generate project files for cross-platform compatibility.

### Prerequisites

You will need:
* A **C++ Compiler** (e.g., Visual Studio Community Edition, GCC/G++, Clang).
* **CMake** (Download and install the latest version from [cmake.org](https://cmake.org/)).
* All dependencies (GLM, GLEW, GLFW, SFML, etc.) are included in the `external/` folder.

### Build Steps (Cross-Platform)

1.  **Navigate to the Project Root:** Open your terminal (PowerShell, CMD, or bash) and go to the `mountainhiking` folder (where `CMakeLists.txt` is located).
    ```bash
    cd "yourpath"
    ```

2.  **Create and Enter the Build Directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Generate Project Files:** Run CMake, specifying your desired build environment.

    ```bash
    # For Windows with Visual Studio 2022
    cmake .. -G "Visual Studio 17 2022" 

    # OR, for Linux/macOS command line build
    cmake .. 
    ```

4.  **Build the Project:**
    * **Visual Studio:** Open the generated **`.sln`** file in the `build` directory. Set the executable project (likely `lab07` or `mountainhiking`) as the **Startup Project** and run **Build All**.

5.  **Run:** The executable will be found within your chosen build output folder. Look for an executable named **`lab07.exe`** (or `./lab07` on Linux/Mac).
