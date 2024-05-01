#pragma once
#include <string>
#include <list>
#include <memory>
#include <utility>

class IMaterial;

/**
 * @brief A custom material with a display-friendly name. Also manages a list of custom materials.
 */
class CustomMaterial
{
public:
    using Ptr = std::shared_ptr<CustomMaterial>;

    CustomMaterial(std::string name, IMaterial* material) : m_name(std::move(name)), m_material(material) {}
    ~CustomMaterial();

    const std::string& GetName() const { return m_name; }
    IMaterial* GetMaterial() const { return m_material; }

    static const std::list<Ptr>& GetAll() { return custom_mats; }
    static Ptr GetMatte();
    static Ptr GetSolid();
    static Ptr AddCustomMaterial(const std::string& name, IMaterial* material);

protected:
    friend class StreamEditor;
    static void CreateDefaultMaterials();

private:
    static inline std::list<Ptr> custom_mats;
    
    std::string m_name;
    IMaterial* m_material;
};