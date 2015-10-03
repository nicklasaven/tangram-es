#pragma once
#include <vector>
#include <string>

namespace Tangram {

struct Range {
    int start;
    int length;
};

// TODO
// - key builder for faster string comparison
// see also http://www.boost.org/doc/libs/1_59_0/doc/html/boost/container/flat_map.html
//

template<typename K, typename T>
struct fastmap {
    std::vector<std::pair<K, T>> map;

    T& operator[](const K& key) {
        for (auto& a : map)
            if (key == a.first)
                return a.second;

        map.emplace_back(key, T{});

        return map.back().second;
        // auto& result = map.back().second;
        // std::sort(map.begin(), map.end())
        // return result;
    }
    void clear() { map.clear(); }
};

}
