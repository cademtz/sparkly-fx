#pragma once
#include "rendertweak.h"
#include <string>

/// @brief A combination of render tweaks to be used while rendering a frame
class RenderConfig
{
public:
    using Ptr = std::shared_ptr<RenderConfig>;
    using ElementType = RenderTweak::Ptr;

    RenderConfig(std::string&& name) : m_name(std::move(name)) {} 

    std::string& GetName() { return m_name; }
    const std::string& GetName() const { return m_name; }
    const std::vector<ElementType>& GetRenderTweaks() const { return m_tweaks; }
    std::vector<ElementType>& GetRenderTweaks() { return m_tweaks; }
    
    template <class T>
    class const_type_iterator
    {
    public:
        static const_type_iterator begin(const RenderConfig& r) {
            return const_type_iterator::const_type_iterator(get_begin_ptr(r), get_end_ptr(r));
        }
        static const_type_iterator end(const RenderConfig& r) {
            return const_type_iterator::const_type_iterator(get_end_ptr(r), get_end_ptr(r));
        }
        
        const_type_iterator& operator++() {
            do {
                ++ptr;
            } while (ptr < ptr_end && std::dynamic_pointer_cast<T>(*ptr) != nullptr);
            return *this;
        }

        std::shared_ptr<T> operator*() const { return std::dynamic_pointer_cast<T>(*ptr); }
        std::shared_ptr<T> operator->() const { return std::dynamic_pointer_cast<T>(*ptr); }
        bool operator==(const const_type_iterator<T>& other) const {
            return this->ptr == other.ptr;
        }
        bool operator!=(const const_type_iterator<T> other) const {
            return !(*this == other);
        }

    private:
        const_type_iterator(const ElementType* next, const ElementType* end)
            : ptr(next), ptr_end(end) {}
        
        static const ElementType* get_begin_ptr(const RenderConfig& r)
        {
            auto& tweaks = r.GetRenderTweaks();
            for (auto& tweak : tweaks)
            {
                if (std::dynamic_pointer_cast<T>(tweak) != nullptr)
                    return &tweak;
            }
            return get_end_ptr(r);
        }
        
        static const ElementType* get_end_ptr(const RenderConfig& r) {
            auto& tweaks = r.GetRenderTweaks();
            return tweaks.empty() ? tweaks.data() : (&tweaks.back() + 1);
        }

        const ElementType* ptr;
        const ElementType* ptr_end;
    };

    template <class T>
    const_type_iterator<T> begin() { return const_type_iterator<T>::begin(*this); }
    template <class T>
    const const_type_iterator<T> end() { return const_type_iterator<T>::end(*this); }

protected:
    std::string m_name;
    std::vector<ElementType> m_tweaks;
};
