#pragma once

#include "scene.hpp"
#include "parser/common.hpp"

#include <optional>
#include <string>
#include <vector>

namespace raytracer::parser {

inline std::optional<Material::Type> materialType(const std::string& token) {
    if (token == "diffuse") { return Material::Type::Diffuse; }
    if (token == "specular") { return Material::Type::Specular; }
    if (token == "shininess") { return Material::Type::Shininess; }
    if (token == "emission") { return Material::Type::Emission; }
    return {};
}

inline bool isMaterial(const std::vector<std::string>& tokens) {
    //if (tokens.empty()) return false;
    auto type = materialType(tokens[0]);
    if (!type) return false;
    if (type == Material::Type::Shininess) return tokens.size() >= 2; // name + s
    return tokens.size() >= 4; // name + rgb
}

inline void parseUpdateMaterial(const std::vector<std::string>& tokens, Material& latest) {
    auto type = *materialType(tokens[0]);
    if (type == Material::Type::Shininess) {
        latest.shininess = parseNum<float>(tokens[1]);
        return;
    }
    float r = parseNum<float>(tokens[1]);
    float g = parseNum<float>(tokens[2]);
    float b = parseNum<float>(tokens[3]);
    switch (type) {
        case Material::Type::Specular:
            latest.specular = {r, g, b};
            return;
        case Material::Type::Diffuse:
            latest.diffuse = {r, g, b};
            return;
        case Material::Type::Emission:
            latest.emission = {r, g, b};
            return;
        default:
            throw ParseException("Expected material type");
    }
}

}
