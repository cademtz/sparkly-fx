#pragma once
#include <mutex>

namespace Helper
{
    /// @brief A reference paired with a lock guard
    template <class T, class LockT = std::scoped_lock<std::mutex>>
    class LockedRef
    {
    public:
        template <class MutexT>
        LockedRef(T& ref, MutexT& mutex) : m_ref(ref), m_lock(mutex) {}
        T& operator*() const { return m_ref; }
        T* operator->() const { return &m_ref; }
    
    private:
        T& m_ref;
        LockT m_lock;
    };
}