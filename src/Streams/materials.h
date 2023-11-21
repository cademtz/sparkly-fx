#pragma once
#include <string>
#include <list>
#include <memory>

class IMaterial;

/**
 * @brief A custom material with a display-friendly name. Also manages a list of custom materials.
 */
class CustomMaterial
{
public:
    using Ptr = std::shared_ptr<CustomMaterial>;

    CustomMaterial(std::string&& name, IMaterial* material) : m_name(name), m_material(material) {}
    const std::string& GetName() const { return m_name; }
    IMaterial* GetMaterial() const { return m_material; }

    static const std::list<Ptr>& GetAll() { return m_custom_mats; }
    static Ptr AddCustomMaterial(std::string&& name, IMaterial* material) {
        return m_custom_mats.emplace_back(std::make_shared<CustomMaterial>(std::move(name), material));
    }

private:
    static inline std::list<Ptr> m_custom_mats;
    
    std::string m_name;
    // TODO: Destroy this when CustomMaterial is destroyed.
    IMaterial* m_material;
};