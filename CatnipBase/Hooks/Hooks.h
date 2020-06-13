#pragma once
#include <unordered_map>
#include "Base/Event.h"
#include "Base/fnv1a.h"

// - Use this to initialize CBaseHook
#define BASEHOOK(Type) CBaseHook(#Type##_hash)
#define GETHOOK(Type) CBaseHook::GetHook<Type>(#Type##_hash)

class CBaseHook : public CEventManager
{
public:
	// - Use BASEHOOK(your_custom_class_here) to initialize a CBaseHook
	CBaseHook(const uint32_t Hash) : m_hash(Hash) { m_hooks[Hash] = this; }
	virtual ~CBaseHook();

	virtual void Hook() = 0;
	virtual void Unhook() { }
	const uint32_t Hash() { return m_hash; }
	static std::unordered_map<uint32_t, CBaseHook*>& Hooks() { return m_hooks; }
	static void HookAll();

	template<class T>
	static inline T* GetHook(const uint32_t Hash)
	{
		return (T*)m_hooks[Hash];
	}

private:
	uint32_t m_hash;
	inline static std::unordered_map<uint32_t, CBaseHook*> m_hooks;
};

class CVMTHook
{
public:
	CVMTHook() { }

	// Initializes and immediately hooks
	CVMTHook(void* Instance, bool AllInstances = false) { Hook(Instance, AllInstances); }
	~CVMTHook() { Unhook(); }

	inline bool IsHooked() { return m_inst; }
	void Hook(void* Instance, bool AllInstances = false);
	void Unhook();

	template <class T>
	inline T* Get(size_t Index) { return (T*)m_oldvmt[Index]; }
	void Set(size_t Index, void* Function);

private:
	void* m_inst = 0;
	void** m_oldvmt = 0, ** m_newvmt = 0;
	size_t m_count = 0;
	bool m_all = false;
};