#pragma once
#include <list>
#include "Base/Event.h"

// - Use this to initialize CBaseHook
#define BASEHOOK(Type) CBaseHook(#Type)
#define GETHOOK(Type) CBaseHook::GetHook<Type>(#Type)

class CBaseHook : public CEventManager
{
public:
	// - Use BASEHOOK(your_custom_class_here) to initialize a CBaseHook
	template<size_t N>
	CBaseHook(const char(&Name)[N]) : m_name(Name) { m_hooks.push_back(this); }
	virtual ~CBaseHook();

	virtual void Hook() = 0;
	virtual void Unhook() { }
	const char* Name() { return m_name; }
	static std::list<CBaseHook*>& Hooks() { return m_hooks; }
	static void HookAll();

	template<class T, size_t N>
	static inline T* GetHook(const char(&Name)[N])
	{
		for (auto hook : m_hooks)
			if (hook->m_name == Name)
				return (T*)hook;
		return nullptr;
	}

private:
	const char* m_name;
	inline static std::list<CBaseHook*> m_hooks;
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