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

    static const std::list<Ptr>& GetAll() { return custom_mats; }
    static Ptr AddCustomMaterial(std::string&& name, IMaterial* material) {
        return custom_mats.emplace_back(std::make_shared<CustomMaterial>(std::move(name), material));
    }

protected:
    friend class StreamEditor;
    static void CreateDefaultMaterials();

private:
    static inline std::list<Ptr> custom_mats;
    
    std::string m_name;
    // TODO: Destroy this when CustomMaterial is destroyed.
    IMaterial* m_material;
};