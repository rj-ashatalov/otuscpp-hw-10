#include "utils.h"

namespace Utils
{
    std::mutex lockPrint;

    size_t FibonacciNaive(const size_t value)
    {
        if (value <= 2)
        {
            return 1;
        }
        return FibonacciNaive(value - 1) + FibonacciNaive(value - 2);
    }

    size_t FactorialNaive(size_t value)
    {
        if (value <= 1)
        {
            return 1;
        }
        return FactorialNaive(value - 1) * value;
    }
}