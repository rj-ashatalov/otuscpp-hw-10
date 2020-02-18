#include <iostream>
#include <regex>
#include "Bulkmlt.h"
#include "log/ConsoleLogger.h"
#include "log/FileLogger.h"
#include <ctime>
#include <queue>
#include "IInterpreterState.h"
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex lockPrint;
std::mutex lockQueue;
std::condition_variable logCheck;
bool logNotified = false;
bool logDone = false;

std::queue<std::string> logMessages;

//! Main app function
int main(int, char const* argv[])
{
    Bulkmlt bulkmlt{atoi(argv[1])};

    std::thread log([&]()
    {
        {
            std::unique_lock<std::mutex> locker(lockPrint);
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
        ConsoleLogger consoleLogger;
        while(!logDone)
        {
            std::unique_lock<std::mutex> locker(lockQueue);
            while(!logNotified)
            {
                logCheck.wait(locker);
            }

            while(!logMessages.empty())
            {
                std::unique_lock<std::mutex> locker(lockPrint);
                consoleLogger.Log(logMessages.front());
                logMessages.pop();
            }
            logNotified = false;
        }
    });
    std::thread file1([&]()
    {
        {
            std::unique_lock<std::mutex> locker(lockPrint);
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
        //TODO @a.shatalov:
    });
    std::thread file2([&]()
    {
        {
            std::unique_lock<std::mutex> locker(lockPrint);
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
        //TODO @a.shatalov:
    });

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        logMessages.push("bulkmlt: " + static_cast<std::string>(group));
        logNotified = true;
        logCheck.notify_one();
    });

    FileLogger fileLogger;
    bulkmlt.eventFirstCommand.Subscribe([&](auto&& timestamp)
    {
        std::stringstream currentTime;
        currentTime << timestamp;
        fileLogger.PrepareFilename("bulkmlt" + currentTime.str());
    });
    bulkmlt.eventSequenceComplete.Subscribe([&](auto& group)
    {
        fileLogger.Log(Utils::Join(group.expressions, "\n"));
    });

    bulkmlt.Run();

    logDone = true;
    log.join();
    file1.join();
    file2.join();
    return 0;
}