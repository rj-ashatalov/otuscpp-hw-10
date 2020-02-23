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

/*    template<class T, class V>
    void Concat(std::vector<T>& dest, const V& src)
    {
//        dest.push_back(src);
    }

    template<class T>
    void Concat(std::vector<T>& dest, const T& src)
    {
        dest.push_back(src);
    }

    template<class T, class V>
    void Concat(std::vector<T>& dest, const std::vector<V>& src)
    {
        if (src.size() <= 0)
        {
            return;
        }

        std::for_each(src.begin(), src.end(), [&dest](auto& item)
        {
            Concat(dest, item);
        });
    }

    template<class T, class Rv>
    std::vector<Rv> Merge(std::vector<T> container)
    {
        std::vector<Rv> result{};
        if (container.size() > 0)
        {
            Concat(result, container);
        }

        return result;
    }*/

    int FibonacciNaive(const size_t value);
    int FactorialNaive(size_t value);
}