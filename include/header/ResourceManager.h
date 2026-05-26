#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <iostream>

class ResourceManager {
public:
    static ResourceManager& get() {
        static ResourceManager instance;
        return instance;
    }

    const sf::Texture& getTexture(const std::string& filename) {
        auto it = textures.find(filename);
        if (it != textures.end()) return it->second;

        sf::Texture tex;
        if (!tex.loadFromFile("../assets/images/" + filename)) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
        }
        textures[filename] = std::move(tex);
        return textures[filename];
    }

private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::unordered_map<std::string, sf::Texture> textures;
};