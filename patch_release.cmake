cmake_minimum_required(VERSION 3.12)

message(STATUS "--- Starting Master Post-Build Deployment ---")

# ==============================================================================
# ALWAYS UP-TO-DATE: Assets & Eigene Python-Strategien
# (Wird bei jedem Build kopiert, geht blitzschnell und hält deinen Code aktuell)
# ==============================================================================

# 1. Assets kopieren
if(EXISTS "${SRC_DIR}/assets")
    file(COPY "${SRC_DIR}/assets" DESTINATION "${BIN_DIR}")
endif()

# 2. Python-Strategien kopieren
if(EXISTS "${SRC_DIR}/src_python")
    file(COPY "${SRC_DIR}/src_python" DESTINATION "${BIN_DIR}")
endif()


# ==============================================================================
# PERFORMANCE-BOOST: Core-Python, DLLs, PYDs & Site-Packages NUR EINMAL DEPLOYEN
# ==============================================================================
# Wir prüfen, ob NumPy bereits im Site-Packages-Ordner existiert.
# Wenn ja, überspringen wir das Kopieren von tausenden Systemdateien vollständig.
if(NOT EXISTS "${TARGET_DIR}/Lib/site-packages/numpy")
    message(STATUS "[SPEED-UP] Python-Dependencies, .pyd-Dateien und DLLs werden frisch deployt... Bitte warten.")

    # 3. Python Lib & Core Standard-Dateien kopieren
    if(EXISTS "${PY_ROOT}/Lib")
        file(COPY "${PY_ROOT}/Lib" DESTINATION "${TARGET_DIR}")
    endif()
    if(EXISTS "${PY_ROOT}/python314.dll")
        file(COPY "${PY_ROOT}/python314.dll" DESTINATION "${TARGET_DIR}")
    endif()
    if(EXISTS "${PY_ROOT}/python314._pth")
        file(COPY "${PY_ROOT}/python314._pth" DESTINATION "${TARGET_DIR}")
    endif()

    # ==========================================================================
    # AUTOMATISCHER PYD & DEPENDENCY FIX (Jetzt geschützt im einmaligen Block!)
    # ==========================================================================
    if(EXISTS "${PY_ROOT}/DLLs")
        message(STATUS "Deploying core Python extensions and dependencies from project external/python/windows/DLLs...")
        # Sucht nach .pyd UND .dll Dateien (wie _ctypes.pyd und libffi-8.dll) 
        # und wirft sie lose neben deine GameProject.exe
        file(GLOB NEW_LIBS "${PY_ROOT}/DLLs/*.pyd" "${PY_ROOT}/DLLs/*.dll")
        foreach(LIB ${NEW_LIBS})
            file(COPY "${LIB}" DESTINATION "${TARGET_DIR}")
        endforeach()
    endif()

    # 4. Site-Packages (NumPy, Scikit-Learn etc.) kopieren & aufräumen
    file(MAKE_DIRECTORY "${TARGET_DIR}/Lib/site-packages")
    if(EXISTS "${WHEELS_DIR}")
        file(COPY "${WHEELS_DIR}/" DESTINATION "${TARGET_DIR}/Lib/site-packages")
    endif()

    file(REMOVE_RECURSE "${TARGET_DIR}/Lib/site-packages/numpy/tests")
    file(REMOVE_RECURSE "${TARGET_DIR}/Lib/site-packages/scipy/tests")

    # 6. TEXT-PATCH: python314._pth konfigurieren
    set(PTH_FILE "${TARGET_DIR}/python314._pth")
    if(EXISTS "${PTH_FILE}")
        file(READ "${PTH_FILE}" CONTENT)
        string(REPLACE "#import site" "import site" CONTENT "${CONTENT}")
        
        if(NOT CONTENT MATCHES "^\\.")
            set(CONTENT ".\nLib\nLib\\site-packages\nimport site\n")
        endif()
        
        file(WRITE "${PTH_FILE}" "${CONTENT}")
        message(STATUS "Successfully configured python314._pth")
    endif()

    # 7. TEXT-PATCH: NumPy Source-Tree Bypass patchen
    set(NUMPY_FILE "${TARGET_DIR}/Lib/site-packages/numpy/__init__.py")
    if(EXISTS "${NUMPY_FILE}")
        file(READ "${NUMPY_FILE}" CONTENT)
        string(REPLACE "raise ImportError(msg)" "# raise ImportError(msg)" CONTENT "${CONTENT}")
        file(WRITE "${NUMPY_FILE}" "${CONTENT}")
        message(STATUS "Successfully patched NumPy source tree bypass")
    endif()

else()
    # Fallback für alle zukünftigen Recompiles: Spart massig Lebenszeit!
    message(STATUS "[SPEED-UP] Python-Umgebung, PYDs und Core-DLLs bereits vorhanden. Überspringe System-Deployment.")
endif()
# ==============================================================================


# ==============================================================================
# ALWAYS CHECK: SFML DLLs (Falls sich Verlinkungen im C++ Build ändern)
# ==============================================================================
# 5. SFML DLLs kopieren
if(EXISTS "${SFML_BIN}")
    file(GLOB SFML_DLLS "${SFML_BIN}/*.dll")
    foreach(DLL ${SFML_DLLS})
        file(COPY "${DLL}" DESTINATION "${TARGET_DIR}")
    endforeach()
endif()

message(STATUS "--- Master Post-Build Deployment Finished Successfully ---")