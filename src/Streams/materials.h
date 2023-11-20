#pragma once
#include <string>
#include <list>

/// @brief A custom material with a display-friendly name
class CustomMaterial
{
public:
    CustomMaterial(std::string&& name, class IMaterial* material) : name(name), material(material) {}
    std::string name;
    // TODO: Destroy this when CustomMaterial is destroyed.
    class IMaterial* material;
};