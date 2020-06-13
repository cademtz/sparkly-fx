#pragma once
#include <unordered_map>
#include "Base/Event.h"
#include "Base/fnv1a.h"

// - Use this to initialize CBaseHook
#define BASEHOOK(Type) CBaseHook(HookHandle { #Type##_hash })
#define GETHOOK(Type) CBaseHook::GetHook<Type>(HookHandle { #Type##_hash })

struct HookHandle {
	const uint32_t hash;
};

class CBaseHook : public CEventManager
{
public:
	// - Use BASEHOOK(your_custom_class_here) to initialize a CBaseHook
	CBaseHook(const HookHandle Hook) : m_hash(Hook.hash) { m_hooks[m_hash] = this; }
	virtual ~CBaseHook();

	virtual void Hook() = 0;
	virtual void Unhook() { }
	const uint32_t Hash() { return m_hash; }
	static std::unordered_map<uint32_t, CBaseHook*>& Hooks() { return m_hooks; }
	static void HookAll();

	template<class T>
	static inline T* GetHook(const HookHandle Hook) { return (T*)m_hooks[Hook.hash]; }

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
	inline T Get(size_t Index) { return (T)m_oldvmt[Index]; }
	void Set(size_t Index, void* Function);

private:
	void* m_inst = 0;
	void** m_oldvmt = 0, ** m_newvmt = 0;
	size_t m_count = 0;
	bool m_all = false;
};

class CJumpHook
{
public:
	~CJumpHook() { UnHook(); }

	template<class T = void*>
	inline T Location() { return (T)m_loc; }
	template<class T = void*>
	inline T Original() { return (T)m_original; }


	template <class T = void*>
	void Hook(T From, void* To, size_t Length = 0) { Hook((void*)From, To, Length); }
	void Hook(void* From, void* To, size_t Length = 0);
	void Hook(const char* Module, const char* Function, void* To, size_t Length = 0);

	void UnHook();

private:
	bool m_hooked = false;
	void* m_loc = nullptr;
	BYTE* m_original = nullptr;
	size_t m_hooklen = 0;

	// - Length: 5
	void RelJmp(UINT_PTR From, UINT_PTR To);

	// - Length: 14
	void AbsJmp(UINT_PTR From, UINT_PTR To);
};