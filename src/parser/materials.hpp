#pragma once

#include "values.hpp"
#include "scene.hpp"
#include "parser/common.hpp"

#include <optional>
#include <string>
#include <vector>

namespace raytracer::parser {

std::optional<Material::Type> materialType(const std::string& token) {
    if (token == "diffuse") { return Material::Type::Diffuse; }
    if (token == "specular") { return Material::Type::Specular; }
    if (token == "shininess") { return Material::Type::Shininess; }
    if (token == "emission") { return Material::Type::Emission; }
    if (token == "refraction") { return Material::Type::Refraction; }
    return {};
}

bool parseMaterial(const std::vector<std::string>& tokens, Material& mat) {
    auto type = materialType(tokens[0]);
    if (!type) {
        return false;
    }
    if (type == Material::Type::Shininess) {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'shininess s'");
        }
        mat.shininess = parseNum<Float>(tokens[1]);
        return true;
    }
    if (type == Material::Type::Refraction) {
        if (tokens.size() != 2) {
            throw ParseException("Expected 'refraction r'");
        }
        mat.refraction = parseNum<Float>(tokens[1]);
        return true;
    }
    // parse specular or diffuse or emission
    if (tokens.size() != 4) {
        throw ParseException("Expected 'material r g b'");
    }
    auto r = parseNum<Float>(tokens[1]);
    auto g = parseNum<Float>(tokens[2]);
    auto b = parseNum<Float>(tokens[3]);
    switch (*type) {
        case Material::Type::Specular:
            mat.specular = {r, g, b, 1};
            return true;
        case Material::Type::Diffuse:
            mat.diffuse = {r, g, b, 1};
            return true;
        case Material::Type::Emission:
            mat.emission = {r, g, b, 1};
            return true;
        default:
            throw ParseException("Expected material type");
    }
}

}  // namespace raytracer::parser
