#pragma once

#include <iostream>
#include <string>

struct ConsoleLogger
{
    void Log(std::string message)
    {
        std::cout << message << std::endl;
    };
};