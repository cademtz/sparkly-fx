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

inline bool _g_event_shutdown = false;

template <typename>
class EventSource;

/**
 * @brief Allows callbacks to be added. Callbacks can then be invoked all at once.
 * 
 * Callbacks must return a combination of values from @ref EventReturnFlags.
 * - Use `Listen` to add a static function callback.
 * - Use `ListenNoArgs` as a shorthand if you want to ignore all the event's parameters.
 * 
 * Example:
 * ```
 * void MyClass::StartListening()
 * {
 *   CoolEvent.Listen(&MyClass::StaticMethod);
 *   CoolEvent.Listen(&MyClass::InstanceMethod, this);
 * }
 * ```
 */
template <typename R, typename... Args>
class EventSource<R(Args...)>
{
public:
    using CallbackType = std::function<int(Args...)>;

    void Listen(const CallbackType& callback) {
        m_Callbacks.push_back(callback);
    }

    void Listen(auto callback)
    {
        m_Callbacks.push_back(
            [callback](Args&&... args) -> int {
                return callback(std::forward<Args>(args)...);
            }
        );
    }

    void ListenNoArgs(auto callback)
    {
        m_Callbacks.push_back(
            [callback](Args&&... args) -> int
            {
                (void)std::tuple<Args...>(args...);
                return callback();
            }
        );
    }

    
    void Listen(auto callback, auto instance)
    {
        m_Callbacks.push_back(
            [callback, instance](Args&&... args) -> int {
                return (instance->*callback)(std::forward<Args>(args)...);
            }
        );
    }

    void ListenNoArgs(auto callback, auto instance)
    {
        m_Callbacks.push_back(
            [callback, instance](Args&&... args) -> int
            {
                (void)std::tuple<Args...>(args...);
                return (instance->*callback)();
            }
        );
    }

    /// @brief Invoke all listeners in order
    /// @return One or more values from @ref EventReturnFlags 
    template <typename... TArgs>
    int DispatchEvent(TArgs&&... args) const
    {
        if (IsShutdown())
            return {};
        
        int flags = 0;
        for (const auto& callback : m_Callbacks)
        {
            flags |= callback(std::forward<TArgs>(args)...);
            if (flags & EventReturnFlags::Skip)
                break;
        }

        return flags;
    }

    /// @brief Permanently disable all dispatching. Useful during ejection.
    static void Shutdown() { _g_event_shutdown = true; }
    static bool IsShutdown() { return _g_event_shutdown; }

private:
    std::vector<CallbackType> m_Callbacks;
};