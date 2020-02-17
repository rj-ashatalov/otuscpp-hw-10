#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <memory>
#include <algorithm>

namespace Utils
{
    template<typename T>
    std::string ToString(T value)
    {
        return static_cast<std::string>(value);
    }

    template<typename T>
    std::string ToString(std::shared_ptr<T> value)
    {
        return static_cast<std::string>(*value);
    }
    //TODO @a.shatalov: implement string traits

    template<class T>
    std::string Join(std::vector<T> container, std::string&& delimiter)
    {
        if (container.size() <= 0)
        {
            return "";
        }

        std::stringstream output;
        std::for_each(container.begin(), std::prev(container.end()), [&output, &delimiter](auto& item)
        {
            output << Utils::ToString(item) << delimiter;
        });
        output << static_cast<std::string>(*container.back());
        return output.str();
    }
}