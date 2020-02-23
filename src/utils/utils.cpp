#include "utils.h"

namespace Utils
{
    int FibonacciNaive(const size_t value)
    {
        if (value <= 2)
        {
            return 1;
        }
        return static_cast<int>(FibonacciNaive(value - 1) + FibonacciNaive(value - 2));
    }

    int FactorialNaive(size_t value)
    {
        if (value <= 1)
        {
            return 1;
        }
        return static_cast<int>(FactorialNaive(value - 1) * value);
    }
}