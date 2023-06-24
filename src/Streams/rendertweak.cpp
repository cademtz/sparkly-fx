#include "rendertweak.h"

bool EntityFilterTweak::IsEntityAffected(const std::string& class_name) const
{
    if (filter_choice == FilterChoice::ALL)
        return true;
    
    bool is_in_list = classes.find(class_name) != classes.end();
    return filter_choice == FilterChoice::WHITELIST ? is_in_list : !is_in_list;
}

bool EntityFilterTweak::IsEffectInvisible() const {
    return render_effect == RenderEffect::INVISIBLE || color_multiply[3] == 0;
}

const char* EntityFilterTweak::FilterChoiceName(FilterChoice value)
{
    switch (value)
    {
    case FilterChoice::ALL: return "All";
    case FilterChoice::WHITELIST: return "Whitelist";
    case FilterChoice::BLACKLIST: return "Blacklist";
    }
    return nullptr;
}

const char* EntityFilterTweak::RenderEffectName(RenderEffect value)
{
    switch (value)
    {
    case RenderEffect::NORMAL: return "Normal";
    case RenderEffect::MATTE: return "Matte";
    case RenderEffect::INVISIBLE: return "Invisible";
    }
    return nullptr;
}
