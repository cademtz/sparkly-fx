#include <Modules/BaseModule.h>
#include <Modules/Menu.h>
#include <Base/Interfaces.h>
#include <SDK/client_class.h>
#include <imgui/imgui_internal.h>
#include <Helper/str.h>
#include <array>

class DevModule : public CModule {
protected:
    void StartListening() override;

private:
    static void DoPropertyTree(RecvProp* prop);
    static void DoDataTableTree(RecvTable* table);

    int OnMenu();
};

DevModule g_devmodule;

void DevModule::StartListening() {
    Listen(EVENT_MENU, [this] { return OnMenu(); });
}

void DevModule::DoPropertyTree(RecvProp* prop)
{
    if (prop->GetType() == SendPropType::DPT_DataTable)
        DoDataTableTree(prop->GetDataTable());
    else
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        window->DC.CursorPos.x += ImGui::GetTreeNodeToLabelSpacing();
        ImGui::Text("%s", prop->GetName());
    }
}

void DevModule::DoDataTableTree(RecvTable* table)
{
    if (ImGui::TreeNode(table->GetName()))
    {
        for (size_t i = 0; i < table->m_nProps; ++i)
            DoPropertyTree(&table->m_pProps[i]);
        ImGui::TreePop();
    }
}

int DevModule::OnMenu()
{
    if (!ImGui::CollapsingHeader("Dev tools"))
        return 0;
    
    static std::array<char, 64> filter_text = {0};
    ImGui::InputText("Class filter", filter_text.data(), filter_text.size() - 1);

    if (ImGui::TreeNode("Classes"))
    {
        ClientClass* next_class = Interfaces::hlclient->GetAllClasses();
        for (; next_class; next_class = next_class->m_pNext)
        {
            if (filter_text[0] && !Helper::strcasestr(next_class->m_pNetworkName, filter_text.data()))
                continue;
            
            if (ImGui::TreeNode(next_class->m_pNetworkName))
            {
                DoDataTableTree(next_class->m_pRecvTable);
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }
    return 0;
}