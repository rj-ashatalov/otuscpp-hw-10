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

std::mutex lockAppendMessage;
std::mutex lockAppendFile;

std::mutex lockPrint;
std::mutex lockLoggerQueue;

std::mutex lockFileQueue;

std::condition_variable threadLogCheck;
std::condition_variable threadFileCheck;

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
        threadFileCheck.wait(locker, [&]()
        {
            return !fileQueue.empty() || isDone;
        });

        if (!fileQueue.empty())
        {
            auto file = std::move(fileQueue.front());
            fileQueue.pop();

            fileMetrics.blockCount++;
            fileMetrics.commandCount += file.content->Size();

            fileLogger.Log(file);
        }
    }

    {
        std::unique_lock<std::mutex> locker(lockPrint);
        std::cout << __PRETTY_FUNCTION__ << " Complete" << std::endl;
    }
}

/*void Worker()
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
            threadFileCheck.wait(locker, [&](){
                return false;
            });
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

    {
        std::unique_lock<std::mutex> locker(lockPrint);
        std::cout << __PRETTY_FUNCTION__ << " Complete"<< std::endl;
    }

}*/

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
            threadLogCheck.wait(locker, [&]()
            {
                return !loggerQueue.empty() || isDone;
            });

            if (!loggerQueue.empty())
            {
                auto content = std::move(loggerQueue.front());
                loggerQueue.pop();

                logMetrics.blockCount++;
                logMetrics.commandCount += content->Size();
                consoleLogger.Log("bulkmlt: " + static_cast<std::string>(*content));
            }
        }

        {
            std::unique_lock<std::mutex> locker(lockPrint);
            std::cout << __PRETTY_FUNCTION__ << " Complete" << std::endl;
        }
    });

    FileLogger fileLogger;
    Metrics fileMetricsOne;
    std::thread file1(FileLogging, std::ref(fileLogger), std::ref(fileMetricsOne));

    Metrics fileMetricsTwo;
    std::thread file2(FileLogging, std::ref(fileLogger), std::ref(fileMetricsTwo));

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        bulkmlt.mainMetrics.blockCount++;
        bulkmlt.mainMetrics.commandCount += group->Size();
    });

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        {
            std::lock_guard<std::mutex> lock(lockAppendMessage);
            loggerQueue.push(group);
        }
        threadLogCheck.notify_one();
    });

    bulkmlt.eventFirstCommand.Subscribe([&](auto&& timestamp)
    {
        std::lock_guard<std::mutex> lock(lockAppendFile);

        std::stringstream currentTime;
        currentTime << timestamp << "_" << std::to_string(bulkmlt.mainMetrics.lineCount);
        fileLogger.PrepareFilename("bulkmlt" + currentTime.str());
    });

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        {
            std::lock_guard<std::mutex> lock(lockAppendFile);
            FileLogger::File file{fileLogger.GetFileName(), group};
            fileQueue.push(std::move(file));
        }
        threadFileCheck.notify_one();
    });

    bulkmlt.Run();

    isDone = true;
    threadLogCheck.notify_all();
    threadFileCheck.notify_all();

    log.join();
    file1.join();
    file2.join();

    std::cout << "main поток - " << bulkmlt.mainMetrics.lineCount << " строк, "
              << bulkmlt.mainMetrics.commandCount << " команд, "
              << bulkmlt.mainMetrics.blockCount << " блок" << std::endl;

    std::cout << "log поток - " << logMetrics.blockCount << " блок, "
              << logMetrics.commandCount << " команд, " << std::endl;

    std::cout << "file1 поток - " << fileMetricsOne.blockCount << " блок, "
              << fileMetricsOne.commandCount << " команд, " << std::endl;

    std::cout << "file2 поток - " << fileMetricsTwo.blockCount << " блок, "
              << fileMetricsTwo.commandCount << " команд, " << std::endl;

    return 0;
}