#include <iostream>
#include <regex>
#include "Bulkmlt.h"
#include "log/ConsoleLogger.h"
#include "log/FileLogger.h"
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>

std::mutex lockAppendMessage;
std::mutex lockAppendFile;

std::mutex lockPrint;
std::mutex lockLoggerQueue;

std::mutex lockFileQueue;

std::condition_variable threadLogCheck;
std::condition_variable threadFileCheck;

bool isDone = false;

std::queue<std::future<void>> loggerQueue;
std::queue<std::future<int>> fileQueue;

void FileLogWorker(Metrics& fileMetrics)
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

        while (!fileQueue.empty())
        {
            auto file = std::move(fileQueue.front());
            fileQueue.pop();

            fileMetrics.blockCount++;
            fileMetrics.commandCount += file.get();
        }
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
        while (!isDone)
        {
            std::unique_lock<std::mutex> locker(lockLoggerQueue);
            threadLogCheck.wait(locker, [&]()
            {
                return !loggerQueue.empty() || isDone;
            });

            while (!loggerQueue.empty())
            {
                auto content = std::move(loggerQueue.front());
                loggerQueue.pop();
                content.get();
            }
        }
    });

    FileLogger fileLogger;
    Metrics fileMetricsOne;
    std::thread file1(FileLogWorker, std::ref(fileMetricsOne));

    Metrics fileMetricsTwo;
    std::thread file2(FileLogWorker, std::ref(fileMetricsTwo));

    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        bulkmlt.mainMetrics.blockCount++;
        bulkmlt.mainMetrics.commandCount += group->Size();
    });

    ConsoleLogger consoleLogger;
    bulkmlt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        {
            std::lock_guard<std::mutex> lock(lockAppendMessage);
            loggerQueue.emplace(std::async(std::launch::deferred, [&](const auto& group)
            {
                logMetrics.blockCount++;
                logMetrics.commandCount += group->Size();

                auto commands = group->Merge();

                std::stringstream output;
                std::for_each(commands.begin(), std::prev(commands.end()), [&output](auto& item)
                {
                    output << Utils::FactorialNaive(std::stoi(item->value)) << ", ";
                });

                output << Utils::FactorialNaive(std::stoi(commands.back()->value));

                consoleLogger.Log("bulkmlt: " + output.str());
            }, group));
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
            fileQueue.emplace(std::async(std::launch::deferred, [&](const auto& filename, const auto& content)->int
            {
                fileLogger.Log(filename, content);
                return content->Size();
            }, fileLogger.GetFileName(), group));
        }
        threadFileCheck.notify_one();
    });

    bulkmlt.Run();

    while (!loggerQueue.empty() || !fileQueue.empty())
    {
        threadLogCheck.notify_one();
        threadFileCheck.notify_one();
    }

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