#include "PythonEnvironment.h"
#include <Python.h>
#include <iostream>
#include "Logger.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <mach-o/dyld.h>
#endif

void PythonEnvironment::initialize() {
    if (Py_IsInitialized()) return;

    // 1. Ermittle den absoluten Pfad zu dem Ordner, in dem deine GameProject.exe liegt
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::filesystem::path exeDir = std::filesystem::path(buffer).parent_path();

    // 2. Setze PYTHONHOME starr auf diesen Ordner.
    // Python sucht dadurch automatisch in:
    // - exeDir/Lib (für die Standard-Bibliotheken)
    // - exeDir/Lib/site-packages (für deine extrahierten Module wie numpy)
    Py_SetPythonHome(exeDir.wstring().c_str());

    // 3. Initialisiere den Python-Interpreter
    Py_Initialize();

    // 4. Füge den Pfad für deine eigenen Python-Skripte (Strategien) hinzu
    // Wir konvertieren Backslashes zu Forward-Slashes, damit Python unter Windows keine Probleme macht
    std::string pythonScriptsPath = (exeDir / "src_python").string();
    std::replace(pythonScriptsPath.begin(), pythonScriptsPath.end(), '\\', '/');

    std::string appendScriptPathCmd = 
        "import sys\n"
        "sys.path.append('" + pythonScriptsPath + "')\n";

    PyRun_SimpleString(appendScriptPathCmd.c_str());

    std::cout << "[INFO][Python] Python successfully initialized in portable mode." << std::endl;
    std::cout << "[DEBUG][Python] Python Home set to: " << exeDir.string() << std::endl;
}

void PythonEnvironment::finalize() {
    if (Py_IsInitialized()) {
        Py_Finalize();
        LOG_INFO("Setup", "Python finalized");
    }
}
