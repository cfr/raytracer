#pragma once

#include "material.hpp"
#include "parser/common.hpp"
#include "values.hpp"

#include <glm/common.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace aktis::parser {

enum class MaterialType : std::uint8_t {
    Diffuse,
    Specular,
    Shininess,
    Emission,
    Ambient,
    Refraction,
    Roughness
};

inline std::optional<MaterialType> materialType(std::string_view token) {
    if (token == "diffuse") {
        return MaterialType::Diffuse;
    }
    if (token == "specular") {
        return MaterialType::Specular;
    }
    if (token == "shininess") {
        return MaterialType::Shininess;
    }
    if (token == "emission") {
        return MaterialType::Emission;
    }
    if (token == "ambient") {
        return MaterialType::Ambient;
    }
    if (token == "refraction") {
        return MaterialType::Refraction;
    }
    if (token == "roughness") {
        return MaterialType::Roughness;
    }
    return {};
}

inline bool parseMaterial(Tokens const& tokens, Material& mat) {
    if (tokens[0] == "brdf") {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'brdf <phong/ggx>'");
        }
        auto const& b = tokens[1];
        if (b != "phong" && b != "ggx") {
            throw ParseException("Expected 'brdf <phong/ggx>'");
        }
        mat.brdfType = b == "ggx" ? brdf::Type::GGX : brdf::Type::Phong;
        return true;
    }
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
        if (mat.refraction != 0 && mat.refraction <= Float(1)) {
            throw ParseException("Expected 'refraction <r>' where r = 0 or r > 1");
        }
        return true;
    }
    if (type == MaterialType::Roughness) {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'roughness <r>'");
        }
        Float const r = parseNum<Float>(tokens[1]);
        if (r < 0 || r > 1) {
            throw ParseException("Expected 'roughness <r>', r in [0, 1]");
        }
        mat.roughness = glm::max(minRoughness, r);
        return true;
    }
    // parse specular or diffuse or emission or ambient
    if (tokens.size() != 4) {
        throw ParseException("Expected '<emission/diffuse/specular/ambient> <r> <g> <b>'");
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
    case MaterialType::Ambient:
        mat.ambient = {r, g, b};
        return true;
    default:
        throw ParseException("Expected material type");
    }
}

}  // namespace aktis::parser
