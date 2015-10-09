#pragma once

#include "platform.h"
#include "scene/styleParam.h"

#include <string>
#include <vector>

namespace Tangram {

class StyleContext;

struct DrawRule {

    int name;
    int style;
    std::vector<StyleParam> parameters;

    DrawRule(int _name, int _style, const std::vector<StyleParam>& _parameters,
             bool _sorted = false);

    DrawRule merge(DrawRule& _other) const;
    std::string toString() const;

    void eval(const StyleContext& _ctx);

    const StyleParam& findParameter(StyleParamKey _key) const;

    bool isJSFunction(StyleParamKey _key) const;

    template<typename T>
    bool get(StyleParamKey _key, T& _value) const {
        auto& param = findParameter(_key);
        if (!param) { return false; }
        if (!param.value.is<T>()) {
            LOGE("wrong type '%d'for StyleParam '%d'",
                   param.value.which(), _key);
            return false;
        }
        _value = param.value.get<T>();
        return true;
    }
    bool contains(StyleParamKey _key) const {
        return findParameter(_key) != false;
    }

    bool operator<(const DrawRule& _rhs) const;
    //int compare(const DrawRule& _rhs) const { return name.compare(_rhs.name); }

};

}
