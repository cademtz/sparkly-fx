#pragma once
#include <Helper/json.h>
#include "rendertweak.h"
#include <string>
#include <memory>

/**
 * @brief A combination of render tweaks to be used while rendering a frame.
 */
class Stream : public Helper::JsonConfigurable
{
public:
    using Ptr = std::shared_ptr<Stream>;
    using ConstPtr = std::shared_ptr<const Stream>;
    using ElementType = RenderTweak::Ptr;

    /// @brief Construct a stream
    /// @param name A non-empty ASCII name
    explicit Stream(std::string name) : m_name(std::move(name)) {}

    Ptr Clone(const std::string& new_name) const;
    std::string& GetName() { return m_name; }
    const std::string& GetName() const { return m_name; }
    std::vector<ElementType>& GetRenderTweaks() { return m_tweaks; }
    nlohmann::json ToJson() const override;
    void FromJson(const nlohmann::json* json) override;
    /// @brief Create a new instance from JSON
    /// @return `nullptr` if the json object does not specify the stream name
    static Ptr CreateFromJson(const nlohmann::json* json);

    static const std::vector<ConstPtr>& GetPresets();
    
    template <class T>
    class const_type_iterator
    {
    public:
        static const_type_iterator begin(const Stream& r) {
            return const_type_iterator::const_type_iterator(get_begin_ptr(r), get_end_ptr(r));
        }
        static const_type_iterator end(const Stream& r) {
            return const_type_iterator::const_type_iterator(get_end_ptr(r), get_end_ptr(r));
        }
        
        const_type_iterator& operator++() {
            do {
                ++ptr;
            } while (ptr < ptr_end && std::dynamic_pointer_cast<T>(*ptr) == nullptr);
            return *this;
        }

        std::shared_ptr<T> operator*() const { return std::dynamic_pointer_cast<T>(*ptr); }
        std::shared_ptr<T> operator->() const { return std::dynamic_pointer_cast<T>(*ptr); }
        bool operator==(const const_type_iterator<T>& other) const {
            return this->ptr == other.ptr;
        }
        bool operator!=(const const_type_iterator<T>& other) const {
            return !(*this == other);
        }

    private:
        const_type_iterator(const ElementType* next, const ElementType* end)
            : ptr(next), ptr_end(end) {}
        
        static const ElementType* get_begin_ptr(const Stream& r)
        {
            auto& tweaks = r.GetRenderTweaks();
            for (auto& tweak : tweaks)
            {
                if (std::dynamic_pointer_cast<T>(tweak) != nullptr)
                    return &tweak;
            }
            return get_end_ptr(r);
        }
        
        static const ElementType* get_end_ptr(const Stream& r) {
            auto& tweaks = r.GetRenderTweaks();
            return tweaks.empty() ? nullptr : (&tweaks.back() + 1);
        }

        const ElementType* ptr;
        const ElementType* ptr_end;
    };

    template <class T>
    const_type_iterator<const T> begin() const { return const_type_iterator<const T>::begin(*this); }
    template <class T>
    const const_type_iterator<const T> end() const { return const_type_iterator<const T>::end(*this); }

    template <class T>
    const_type_iterator<T> begin() { return const_type_iterator<T>::begin(*this); }
    template <class T>
    const const_type_iterator<T> end() { return const_type_iterator<T>::end(*this); }

protected:
    const std::vector<ElementType>& GetRenderTweaks() const { return m_tweaks; }
    
    std::string m_name;
    std::vector<ElementType> m_tweaks;

private:
    static std::vector<ConstPtr> MakePresets();
};
