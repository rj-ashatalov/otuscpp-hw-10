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
#include <functional>
#include <condition_variable>

std::mutex lockPrint;
std::mutex lockLoggerQueue;

std::mutex lockFileWrite;
std::mutex lockFileQueue;

std::condition_variable threadLogCheck;
std::condition_variable threadFileCheck;

bool logNotified = false;
bool fileNotified = false;
bool isDone = false;

std::queue<std::string> loggerQueue;
std::queue<FileLogger::File> fileQueue;

void FileLogging(FileLogger& fileLogger)
{
    {
        std::unique_lock<std::mutex> locker(lockPrint);
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
    while (!isDone)
    {
        std::unique_lock<std::mutex> locker(lockFileQueue);
        while (!fileNotified)
        {
            threadFileCheck.wait(locker);
        }

        while (!fileQueue.empty())
        {
            std::unique_lock<std::mutex> locker(lockFileWrite);
            fileLogger.Log(fileQueue.front());
            fileQueue.pop();
        }
        fileNotified = false;
    }
}

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
        while (!isDone)
        {
            std::unique_lock<std::mutex> locker(lockLoggerQueue);
            while (!logNotified)
            {
                threadLogCheck.wait(locker);
            }

            while (!loggerQueue.empty())
            {
                std::unique_lock<std::mutex> locker(lockPrint);
                consoleLogger.Log(loggerQueue.front());
                loggerQueue.pop();
            }
            logNotified = false;
        }
    });

    FileLogger fileLogger;
    std::thread file1(FileLogging, std::ref(fileLogger));
    std::thread file2(FileLogging, std::ref(fileLogger));

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        bulkmlt.mainMetrics.blockCount++;
        bulkmlt.mainMetrics.commandCount += group.Size();
    });

    Metrics logMetrics;
    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        logMetrics.blockCount++;
        logMetrics.commandCount += group.Size();

        loggerQueue.push("bulkmlt: " + static_cast<std::string>(group));
        logNotified = true;
        threadLogCheck.notify_one();
    });

    bulkmlt.eventFirstCommand.Subscribe([&](auto&& timestamp)
    {
        std::stringstream currentTime;
        currentTime << timestamp;
        fileLogger.PrepareFilename("bulkmlt" + currentTime.str());
    });

    Metrics fileMetrics;
    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        fileMetrics.blockCount++;
        fileMetrics.commandCount += group.Size();

        FileLogger::File file{fileLogger.GetFileName(), Utils::Join(group.expressions, "\n")};
        fileQueue.push(std::move(file));
        fileNotified = true;
        threadFileCheck.notify_all();
    });

    bulkmlt.Run();

    isDone = true;
    log.join();
    file1.join();
    file2.join();
    return 0;
}