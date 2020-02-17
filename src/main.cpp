#include <iostream>
#include <regex>
#include "Bulk.h"
#include "log/ConsoleLogger.h"
#include "log/FileLogger.h"
#include <ctime>
#include "IInterpreterState.h"

//! Main app function
int main(int, char const* argv[])
{
    Bulk bulk{atoi(argv[1])};

    ConsoleLogger consoleLogger;
    bulk.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        consoleLogger.Log("bulk: " + static_cast<std::string>(group));
    });

    FileLogger fileLogger;
    bulk.eventFirstCommand.Subscribe([&](auto&& timestamp)
    {
        std::stringstream currentTime;
        currentTime << timestamp;
        fileLogger.PrepareFilename("bulk" + currentTime.str());
    });
    bulk.eventSequenceComplete.Subscribe([&](auto& group)
    {
        fileLogger.Log(Utils::Join(group.expressions, "\n"));
    });

    bulk.Run();
    return 0;
}