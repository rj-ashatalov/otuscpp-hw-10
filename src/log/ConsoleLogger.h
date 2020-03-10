#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include "utils/utils.h"

struct ConsoleLogger
{
    void Log(std::string message)
    {
        std::lock_guard<std::mutex> lockPrint(Utils::lockPrint);
        std::cout << message << std::endl;
    };
};