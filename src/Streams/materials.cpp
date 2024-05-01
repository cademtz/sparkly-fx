#include "materials.h"
#include <SDK/KeyValues.h>
#include <Base/Interfaces.h>
#include <SDK/imaterial.h>

CustomMaterial::~CustomMaterial()
{
    if (m_material)
        m_material->DecrementReferenceCount();
}

CustomMaterial::Ptr CustomMaterial::AddCustomMaterial(const std::string& name, IMaterial* material) {
    return custom_mats.emplace_back(std::make_shared<CustomMaterial>(name, material));
}

CustomMaterial::Ptr CustomMaterial::GetMatte()
{
    static Ptr m = AddCustomMaterial("Matte", nullptr);
    return m;
}

CustomMaterial::Ptr CustomMaterial::GetSolid()
{
    static Ptr m = AddCustomMaterial("Solid", nullptr);
    return m;
}

void CustomMaterial::CreateDefaultMaterials()
{
    assert(custom_mats.empty() && "Function should only be called once");

    KeyValues* vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$znearer", true);
    
    // The KeyValues instance is owned by the new material, and should not be deleted
    {
        IMaterial* mat = Interfaces::mat_system->CreateMaterial("sparklyfx_solid.vmt", vmt_values);
        if (mat)
            GetSolid()->m_material = mat;
        else
            vmt_values->deleteThis();
    }

    vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$nofog", true);
    vmt_values->SetBool("$znearer", true);
    
    {
        IMaterial* mat = Interfaces::mat_system->CreateMaterial("sparklyfx_matte.vmt", vmt_values);
        if (mat)
            GetMatte()->m_material = mat;
        else
            vmt_values->deleteThis();
    }

    vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    //vmt_values->SetBool("$nocull", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$nofog", true);
    vmt_values->SetBool("$znearer", true);
    vmt_values->SetBool("$wireframe", true);

    {
        IMaterial* mat = Interfaces::mat_system->CreateMaterial("sparklyfx_matte_wireframe.vmt", vmt_values);
        if (mat)
            AddCustomMaterial("Matte wireframe", mat);
        else
            vmt_values->deleteThis();
    }
    
    vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    //vmt_values->SetBool("$nocull", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$znearer", true);
    vmt_values->SetBool("$wireframe", true);

    {
        IMaterial* mat = Interfaces::mat_system->CreateMaterial("sparklyfx_wireframe.vmt", vmt_values);
        if (mat)
            AddCustomMaterial("Wireframe", mat);
        else
            vmt_values->deleteThis();
    }
}
