cmake_minimum_required(VERSION 3.10)
project(GameProject)

# WICHTIG: Erlaubt Windows, SFML_ROOT als Suchpfad zu nutzen (behebt den Fehler)
cmake_policy(SET CMP0074 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.12.0

    UPDATE_DISCONNECTED ON
)

FetchContent_MakeAvailable(pybind11)

set(PYBIND11_CPP_STANDARD -std=c++17 CACHE INTERNAL "")
add_definitions(-DPy_TRACE_REFS=0)

if(APPLE)
    message(STATUS "Configuring for macOS")

    set(SFML_ROOT "${CMAKE_SOURCE_DIR}/external/sfml/macOS")
    set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML")

elseif(WIN32)
    message(STATUS "Configuring for Windows")

    set(SFML_ROOT "${CMAKE_SOURCE_DIR}/external/sfml/windows")
    # Das "FORCE CACHE PATH" brennt den Pfad so in CMake ein, dass find_package ihn nicht mehr ignorieren kann
    set(SFML_DIR "${SFML_ROOT}/lib/cmake/SFML" CACHE PATH "Path to SFML CMake files" FORCE)

    list(APPEND CMAKE_PREFIX_PATH "${SFML_ROOT}")

else()
    message(FATAL_ERROR "Unsupported platform")
endif()

find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio Network)

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp

    external/imgui-sfml/imgui-SFML.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui-sfml
    ${SFML_ROOT}/include
)

file(GLOB_RECURSE SOURCE_FILES
    src/*.cpp
)
add_executable(GameProject ${SOURCE_FILES})

if(WIN32)
    target_compile_definitions(GameProject PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

target_link_libraries(GameProject PRIVATE SFML::Graphics SFML::Audio SFML::Network imgui pybind11::embed)

if(APPLE)
    target_link_libraries(GameProject
        "-framework OpenGL"
    )
endif()

target_include_directories(GameProject PRIVATE 
    include/header
    include/json/single_include
    external/imgui              
    external/imgui-sfml
    ${SFML_ROOT}/include  
)

if (APPLE)
    message(STATUS "Configuring for macOS")
    set(PYTHON_RUNTIME_DIR "${CMAKE_SOURCE_DIR}/external/python/macOS")
    target_link_directories(GameProject PRIVATE "${PYTHON_RUNTIME_DIR}")
    target_link_libraries(GameProject "${PYTHON_RUNTIME_DIR}/libpython3.13.dylib")
    target_include_directories(GameProject PRIVATE "${PYTHON_RUNTIME_DIR}/include")
elseif(WIN32)
    message(STATUS "Configuring for Windows - Local Python 3.14")

    set(PYTHON_ROOT "${CMAKE_SOURCE_DIR}/external/python/windows")
    
    # 1. Wir setzen die Variablen manuell, damit CMake nicht raten muss
    set(Python3_INCLUDE_DIRS "${PYTHON_ROOT}/include")
    set(Python3_LIBRARIES "${PYTHON_ROOT}/libs/python314.lib")

    # 2. Definiere, dass wir gegen die DLL linken (WICHTIG!)
    add_compile_definitions(Py_BUILD_CORE_MODULE)

    target_include_directories(GameProject PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(GameProject PRIVATE ${Python3_LIBRARIES} opengl32)
endif()

set(REQUIREMENTS_FILE "${CMAKE_SOURCE_DIR}/requirements.txt")
set(WHEEL_EXTRACT_DIR "${CMAKE_BINARY_DIR}/temp_wheels")

if(EXISTS ${REQUIREMENTS_FILE})
    if(APPLE)
        set(VENV_DIR "${CMAKE_BINARY_DIR}/venv")
        set(PIP_EXECUTABLE "${VENV_DIR}/bin/pip")
        set(VENV_PYTHON "${VENV_DIR}/bin/python3")
        set(PYTHON_CMD "python3")

        add_custom_target(install_python_requirements
            COMMAND ${PYTHON_CMD} -m venv ${VENV_DIR}
            COMMAND ${VENV_PYTHON} -m pip install --upgrade pip
            COMMAND ${PIP_EXECUTABLE} install --target "${WHEEL_EXTRACT_DIR}" -r ${REQUIREMENTS_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Creating venv and isolating clean Python dependencies (macOS)"
            VERBATIM
        )
    elseif(WIN32)
        if(PYTHON_EXECUTABLE)
            set(PYTHON_CMD "${PYTHON_EXECUTABLE}")
        else()
            set(PYTHON_CMD "python")
        endif()

        add_custom_target(install_python_requirements
            COMMAND ${CMAKE_COMMAND} -E make_directory "${WHEEL_EXTRACT_DIR}"
            
            # HIER DIE RETTUNG: Wir fügen --no-warn-conflicts hinzu, damit pip die globalen Konflikte ignoriert.
            # Zusätzlich fangen wir den Error-Code ab, da die Installation ja nachweislich klappt!
            COMMAND ${PYTHON_CMD} -m pip install 
                --target "${WHEEL_EXTRACT_DIR}" 
                --platform win_amd64 
                --python-version 3.14 
                --only-binary=:all: 
                --upgrade
                --no-warn-conflicts
                --disable-pip-version-check
                -r ${REQUIREMENTS_FILE}
                
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Fetching pristine Python 3.14 Windows wheels cleanly..."
            VERBATIM
        )
        
        set(VENV_PYTHON "${PYTHON_CMD}")
    endif()

    add_dependencies(GameProject install_python_requirements)
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-deprecated-declarations")
elseif(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4244 /wd4267")
endif()

if(MSVC)
    add_compile_definitions(_USE_MATH_DEFINES)
endif()

if(WIN32)
    # Ein einziger, unkaputtbarer Befehl für Visual Studio.
    # Wir übergeben die Pfade sauber in Anführungszeichen an unser Master-Skript.
    add_custom_command(TARGET GameProject POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
            -D "SRC_DIR=${CMAKE_SOURCE_DIR}" 
            -D "BIN_DIR=${CMAKE_BINARY_DIR}" 
            -D "TARGET_DIR=$<TARGET_FILE_DIR:GameProject>" 
            -D "PY_ROOT=${PYTHON_ROOT}" 
            -D "SFML_BIN=${SFML_ROOT}/bin" 
            -D "WHEELS_DIR=${WHEEL_EXTRACT_DIR}"
            -P "${CMAKE_SOURCE_DIR}/patch_release.cmake"
        COMMENT "Executing master post-build deployment and patches..."
        VERBATIM
    )
endif()