#pragma once
#include <unordered_map>
#include <Base/Event.h>
#include <Base/fnv1a.h>

// - Use this to initialize CBaseHook
#define BASEHOOK(Type) CBaseHook(HookHandle { #Type##_hash })
//#define GETHOOK(Type) CBaseHook::GetHook<Type>(HookHandle { #Type##_hash })

#ifdef _WIN64
#define UNCRAP void*,
#define UNCRAP_VOID void*
#else
#define UNCRAP
#define UNCRAP_VOID
#endif

struct HookHandle
{
    const uint32_t hash;
};

class CBaseHook
{
public:
    // - Use BASEHOOK(your_custom_class_here) to initialize a CBaseHook
    explicit CBaseHook(const HookHandle Hook) : m_hash(Hook.hash) { m_hooks[m_hash] = this; }
    virtual ~CBaseHook();

    virtual void Hook() = 0;

    virtual void Unhook() {}

    uint32_t Hash() const { return m_hash; }
    static std::unordered_map<uint32_t, CBaseHook*>& Hooks() { return m_hooks; }
    static void HookAll();
    static void UnHookAll();

    template <class T>
    static T* GetHook(const HookHandle Hook) { return (T*)m_hooks[Hook.hash]; }

private:
    uint32_t m_hash;
    inline static std::unordered_map<uint32_t, CBaseHook*> m_hooks;
};

class CVMTHook
{
public:
    CVMTHook() = default;

    // Initializes and immediately hooks
    explicit CVMTHook(void* Instance, bool AllInstances = false) { Hook(Instance, AllInstances); }
    ~CVMTHook() { Unhook(); }

    bool IsHooked() const { return m_inst; }
    void Hook(void* Instance, bool AllInstances = false);
    void Unhook();

    template <class T>
    T Get(const size_t Index) { return static_cast<T>(m_oldvmt[Index]); }

    void Set(size_t Index, void* Function);

private:
    void* m_inst = nullptr;
    void **m_oldvmt = nullptr, **m_newvmt = nullptr;
    size_t m_count = 0;
    bool m_all = false;
};

class CJumpHook
{
public:
    ~CJumpHook() { Unhook(); }

    template <class T = void*>
    T Location() { return (T)m_from; }

    template <class T = void*>
    T Original() { return (T)m_original; }

    void Hook(void* From, void* To);
    void Unhook();

private:
    void* m_original = nullptr;
    void* m_from = nullptr;
    void* m_to = nullptr;
};
