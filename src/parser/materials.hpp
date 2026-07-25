#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <optional>
#include <string>
#include <vector>

namespace raytracer::parser {

enum class MaterialType: int {
    Diffuse,
    Specular,
    Shininess,
    Emission,
    Refraction,
    Roughness
};

std::optional<MaterialType> materialType(const std::string& token) {
    if (token == "diffuse") { return MaterialType::Diffuse; }
    if (token == "specular") { return MaterialType::Specular; }
    if (token == "shininess") { return MaterialType::Shininess; }
    if (token == "emission") { return MaterialType::Emission; }
    if (token == "refraction") { return MaterialType::Refraction; }
    if (token == "roughness") { return MaterialType::Roughness; }
    return {};
}

bool parseMaterial(const std::vector<std::string>& tokens, Material& mat) {
    auto type = materialType(tokens[0]);
    if (!type) {
        return false;
    }
    if (type == MaterialType::Shininess) {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'shininess <s>'");
        }
        mat.shininess = parseNum<Float>(tokens[1]);
        return true;
    }
    if (type == MaterialType::Refraction) {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'refraction <r>'");
        }
        mat.refraction = parseNum<Float>(tokens[1]);
        return true;
    }
    if (type == MaterialType::Roughness) {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'roughness <r>'");
        }
        mat.refraction = parseNum<Float>(tokens[1]);
        return true;
    }
    // parse specular or diffuse or emission
    if (tokens.size() != 4) {
        throw ParseException("Expected '<emission/diffuse/specular> <r> <g> <b>'");
    }
    auto r = parseNum<Float>(tokens[1]);
    auto g = parseNum<Float>(tokens[2]);
    auto b = parseNum<Float>(tokens[3]);
    switch (*type) {
        case MaterialType::Specular:
            mat.specular = {r, g, b};
            return true;
        case MaterialType::Diffuse:
            mat.diffuse = {r, g, b};
            return true;
        case MaterialType::Emission:
            mat.emission = {r, g, b};
            return true;
        default:
            throw ParseException("Expected material type");
    }
}

}  // namespace raytracer::parser
