# Hybrid C++/Python Simulation Framework
### Academic Software Project — Bachelor's Thesis

This repository contains the source code and development environment for a hybrid simulation application. The project combines the performance and deterministic control of a native C++17 core with the dynamic flexibility of high-level Python scripting for AI strategies.

## 🏛️ System Architecture & Core Concepts

The system is designed around a decoupled, multi-layered architecture to separate physics, simulation logic, UI representation, and decision-making agents:

*   **Game Engine Layer (C++):** Handles real-time simulation updates, physics processing, and collision detection via a centralized `CollisionManager`, `GameMap`, and `GameHandler`.
*   **UI & Visualization Layer (SFML 3.0 + ImGui):** Manages the simulation window, real-time rendering, and input events using `imgui-sfml` bindings.
*   **Abstraction Layer (MVVM Style):** Utilizes a `ViewModel` architecture to abstract business logic from scenes (e.g., `TeamSelectionScene`, `TournamentScene`), keeping views platform-agnostic.
*   **AI Layer (C++ & Python Plugin Architecture):** Native C++ strategies inherit from `StrategyBase`. Python strategies are loaded dynamically at runtime via `pybind11` folder scanning, enabling hot-swapping without recompilation.

## 📁 Repository Structure

| Directory / File | Description & Core Responsibility |
| :--- | :--- |
| `src/` | Core C++ engine implementation, native simulation logic, UI, and C++ strategies. |
| `include/` | Public C++ header files defining engine APIs and module interfaces. |
| `src_python/` | Python-based team/player strategies, helper scripts, and ML modules. |
| `assets/` | Game resources including texture images, typography fonts, and map configurations. |
| `external/` | Third-party source dependencies and platform-specific portable Python runtimes. |
| `logs/` | Target directory for structured runtime telemetries and diagnostics (`JSONL` files). |
| `build/` | Out-of-source compilation directory (**excluded from version control**). |

## 🛠️ Development Environment & Setup

### Prerequisites & Dependencies
The build system utilizes **CMake (v3.10+)**. External frameworks are fetched via submodules or `FetchContent`. The build environment automatically bootstraps an isolated **Python 3.13 Virtual Environment** inside the build directory and installs all dependencies listed in `requirements.txt`.

### Compilation & Execution
```bash
mkdir build
cd build
cmake ..
make
./GameProject [arguments]
```

## 🚀 Command-Line Arguments (CLI)

The compiled `GameProject` executable supports several runtime arguments to configure the simulation settings and control machine learning data acquisition:

*   **`--ml`**  
    Enables standard gameplay data collection from rule-based agents for machine learning training. Active C++ player strategies automatically log gameplay samples to the dataset at runtime.
*   **`--userMl`**  
    Extends the `--ml` functionality by additionally capturing raw input from human-controlled agents, creating a more diverse dataset based on human tactical behavior.
*   **`--width [px]`**  
    Allows for a custom resolution width configuration of the user client window as a fallback for legacy hardware or specific display configurations.
*   **`--height [px]`**  
    Allows for a custom resolution height configuration of the user client window.
*   **`--debug`**  
    Activates the multi-level system logger for diagnostic purposes. Records all events from the `DEBUG` level upwards.

### Example Run Configurations
```bash
# Data Collection Mode with active Logging:
./GameProject --ml --debug

# Human Gameplay Data Collection on a Custom 1080p Window:
./GameProject --userMl --width 1920 --height 1080
```

## 🪵 Diagnostics & Logging Architecture

When executing with the `--debug` flag, the application pipes structured runtime data into highly decoupled asynchronous file channels. Logs are formatted in `JSONL` (JSON Lines) appended with precise Unix timestamps:

*   `logs/game.jsonl`: Captures low-level engine updates, engine telemetry, and general simulation lifecycle events.
*   `logs/ai.jsonl`: Stores high-level decision-making processes routed from Team and Player AI layers to facilitate focused debugging without engine telemetry interference.

## 🧠 Machine Learning Workflow & Training

### 1. Data Collection
Generate training samples by executing the simulation with the tracking flags:
```bash
./GameProject --ml
```

### 2. Model Training & Feature Extraction
The core training logic is encapsulated in `train_ml_model.py`. This script utilizes a `FeatureBuilder` component to transform raw JSONL samples into structured feature vectors and filters suboptimal samples based on the `evaluationScore`. Run the script manually by specifying the required team count argument:
```bash
cd src_python/ml_python
python train_ml_model.py [team_count]
```

### 3. Runtime Integration
During the initialization of an ML strategy, the system automatically invokes the `train_ml_model` logic to ensure the model is instantiated with the most recent data. At runtime, the Player AI utilizes the same `FeatureBuilder` to translate the current game state into the required feature vector format for real-time predictions.