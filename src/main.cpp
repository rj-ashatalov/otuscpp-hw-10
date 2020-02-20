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

std::queue<std::shared_ptr<Group>> loggerQueue;
std::queue<FileLogger::File> fileQueue;

void FileLogging(FileLogger& fileLogger, Metrics& fileMetrics)
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
            if (fileQueue.empty())
            {
                break;
            }

            const auto& file = fileQueue.front();
            fileMetrics.blockCount++;
            fileMetrics.commandCount += file.content->Size();

            fileLogger.Log(file);
            fileQueue.pop();
        }
        fileNotified = false;
    }
}

//! Main app function
int main(int, char const* argv[])
{
    Bulkmlt bulkmlt{atoi(argv[1])};

    Metrics logMetrics;
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
                if (loggerQueue.empty())
                {
                    break;
                }

                const auto& content = loggerQueue.front();
                logMetrics.blockCount++;
                logMetrics.commandCount += content->Size();

                consoleLogger.Log("bulkmlt: " + static_cast<std::string>(*content));
                loggerQueue.pop();
            }
            logNotified = false;
        }
    });

    Metrics fileMetrics;
    FileLogger fileLogger;
    std::thread file1(FileLogging, std::ref(fileLogger), std::ref(fileMetrics));
    std::thread file2(FileLogging, std::ref(fileLogger), std::ref(fileMetrics));

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        bulkmlt.mainMetrics.blockCount++;
        bulkmlt.mainMetrics.commandCount += group->Size();
    });

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        loggerQueue.push(group);
        logNotified = true;
        threadLogCheck.notify_one();
    });

    bulkmlt.eventFirstCommand.Subscribe([&](auto&& timestamp)
    {
        std::stringstream currentTime;
        currentTime << timestamp;
        fileLogger.PrepareFilename("bulkmlt" + currentTime.str());
    });

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        FileLogger::File file{fileLogger.GetFileName(), group};
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