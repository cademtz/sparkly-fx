#pragma once
#include <unordered_map>
#include <functional>
#include <optional>
#include <atomic>

namespace EventReturnFlags
{
    /// @brief No effect - calls the rest of the listeners
    static constexpr int Normal = 0;
    /// @brief Skip calling other modules/listeners after the current function returns
    static constexpr int Skip = (1 << 0);
    /// @brief For hooks. Prevents calling the original function
    static constexpr int NoOriginal = (1 << 1);
}

template <typename>
struct EventSource;

template <typename R>
struct EventReturn
{
    int Flags = EventReturnFlags::Normal;
    std::conditional_t<!std::is_same_v<R, void>, std::optional<R>, std::nullopt_t> ReturnValue = std::nullopt;
};

template <typename R, typename... Args>
struct EventSource<R(Args...)>
{
    using Proto = R(Args...);
    using Param = EventReturn<R>;
    using CallbackType = std::function<void(Param&, Args...)>;

    void Listen(const CallbackType& callback)
    {
        m_Callbacks.push_back(callback);
    }

    void Listen(auto callback)
    {
        m_Callbacks.push_back([callback](Param& p, Args&&... args)
        {
            callback(p, std::forward<Args>(args)...);
        });
    }

    void Listen(auto callback, auto instance)
    {
        m_Callbacks.push_back([callback, instance](Param& p, Args&&... args)
        {
            (instance->*callback)(p, std::forward<Args>(args)...);
        });
    }

    void ListenNoArgs(auto callback)
    {
        m_Callbacks.push_back([callback](const Param& p, Args&&... args)
        {
            (void)p;
            (void)std::tuple<Args...>(args...);
            callback();
        });
    }

    void ListenNoArgs(auto callback, auto instance)
    {
        m_Callbacks.push_back([callback, instance](const Param& p, Args&&... args)
        {
            (void)p;
            (void)std::tuple<Args...>(args...);
            (instance->*callback)();
        });
    }

    template <typename... TArgs>
    Param DispatchEvent(TArgs&&... args) const
    {
        if (m_ShutDown)
            return {};

        Param out;
        for (auto& callback : m_Callbacks)
        {
            callback(out, std::forward<TArgs>(args)...);
            if (out.Flags & EventReturnFlags::Skip)
                break;
        }

        return out;
    }

    static void Shutdown()
    {
        m_ShutDown = true;
    }

private:
    static inline std::atomic_bool m_ShutDown{false};
    std::vector<CallbackType> m_Callbacks;
};
