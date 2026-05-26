#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#endif

#include <SFML/Graphics.hpp>

namespace WindowUtils {

    inline void minimize(sf::RenderWindow& window) {
    #ifdef _WIN32
        HWND hwnd = window.getNativeHandle();
        ShowWindow(hwnd, SW_MINIMIZE);
    #elif __APPLE__

        window.setVisible(false);
    #elif __linux__
        window.setVisible(false);
    #else
        window.setVisible(false);
    #endif
    }

    inline void restore(sf::RenderWindow& window) {
    #ifdef _WIN32
        HWND hwnd = window.getNativeHandle();
        ShowWindow(hwnd, SW_RESTORE);
    #elif __APPLE__
        window.setVisible(true);
    #elif __linux__
        window.setVisible(true);
    #else
        window.setVisible(true);
    #endif
    }
}
