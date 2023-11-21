#include "materials.h"
#include <SDK/KeyValues.h>
#include <Base/Interfaces.h>

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
    IMaterial* mat = Interfaces::mat_system->CreateMaterial("sparklyfx_solid.vmt", vmt_values);
    if (mat)
        CustomMaterial::AddCustomMaterial("Solid", mat);
    else
        vmt_values->deleteThis();

    vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$nofog", true);
    vmt_values->SetBool("$znearer", true);
    
    // The KeyValues instance is owned by the new material, and should not be deleted
    mat = Interfaces::mat_system->CreateMaterial("sparklyfx_matte.vmt", vmt_values);
    if (mat)
        CustomMaterial::AddCustomMaterial("Matte", mat);
    else
        vmt_values->deleteThis();

    vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    //vmt_values->SetBool("$nocull", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$nofog", true);
    vmt_values->SetBool("$znearer", true);
    vmt_values->SetBool("$wireframe", true);

    mat = Interfaces::mat_system->CreateMaterial("sparklyfx_matte_wireframe.vmt", vmt_values);
    if (mat)
        CustomMaterial::AddCustomMaterial("Matte wireframe", mat);
    else
        vmt_values->deleteThis();
    
    vmt_values = new KeyValues("UnlitGeneric");
    vmt_values->SetString("$basetexture", "vgui/white_additive");
    vmt_values->SetBool("$model", true);
    vmt_values->SetBool("$flat", true);
    //vmt_values->SetBool("$nocull", true);
    vmt_values->SetBool("$selfillum", true);
    vmt_values->SetBool("$znearer", true);
    vmt_values->SetBool("$wireframe", true);

    mat = Interfaces::mat_system->CreateMaterial("sparklyfx_wireframe.vmt", vmt_values);
    if (mat)
        CustomMaterial::AddCustomMaterial("Wireframe", mat);
    else
        vmt_values->deleteThis();
}
