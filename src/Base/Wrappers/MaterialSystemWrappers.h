#include "Wrappers.h"

class IMaterialSystem081;
class IMaterialSystemGMod;

class IMaterialSystemWrapperSDK : public IMaterialSystemWrapper
{
    IMaterialSystem* m_int;

public:
    IMaterialSystemWrapperSDK(void* instance) : m_int((IMaterialSystem*)instance) {}

	void* Inst() override { return m_int; };
	IMatRenderContext* GetRenderContext() override;
	IMaterial* CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) override;
	MaterialHandle_t FirstMaterial() override;
	MaterialHandle_t NextMaterial(MaterialHandle_t handle) override;
	MaterialHandle_t InvalidMaterial() override;
	IMaterial* GetMaterial(MaterialHandle_t handle) override;
	ITexture* FindTexture(char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) override;
};

class IMaterialSystemWrapper081 : public IMaterialSystemWrapper
{
    IMaterialSystem081* m_int;

public:
    IMaterialSystemWrapper081(void* instance) : m_int((IMaterialSystem081*)instance) {}

	void* Inst() override { return m_int; };
	IMatRenderContext* GetRenderContext() override;
	IMaterial* CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) override;
	MaterialHandle_t FirstMaterial() override;
	MaterialHandle_t NextMaterial(MaterialHandle_t handle) override;
	MaterialHandle_t InvalidMaterial() override;
	IMaterial* GetMaterial(MaterialHandle_t handle) override;
	ITexture* FindTexture(char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) override;
};

class IMaterialSystemWrapperGMod : public IMaterialSystemWrapper
{
    IMaterialSystemGMod* m_int;

public:
    IMaterialSystemWrapperGMod(void* instance) : m_int((IMaterialSystemGMod*)instance) {}

	void* Inst() override { return m_int; };
	IMatRenderContext* GetRenderContext() override;
	IMaterial* CreateMaterial(const char *pMaterialName, KeyValues *pVMTKeyValues) override;
	MaterialHandle_t FirstMaterial() override;
	MaterialHandle_t NextMaterial(MaterialHandle_t handle) override;
	MaterialHandle_t InvalidMaterial() override;
	IMaterial* GetMaterial(MaterialHandle_t handle) override;
	ITexture* FindTexture(char const* pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags) override;
};

