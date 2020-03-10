#include <iostream>
#include <regex>
#include "Bulkmt.h"
#include "log/ConsoleLogger.h"
#include "log/FileLogger.h"
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <future>
#include <atomic>

std::mutex lockLoggerQueue;

std::mutex lockFileQueue;

std::condition_variable threadLogCheck;
std::condition_variable threadFileCheck;

std::atomic_bool isDone = false;

std::queue<std::future<void>> loggerQueue;
std::queue<std::future<int>> fileQueue;

void FileLogWorker(Metrics& fileMetrics)
{
    {
        std::unique_lock<std::mutex> locker(Utils::lockPrint);
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
            locker.unlock();

            fileMetrics.blockCount++;
            fileMetrics.commandCount += file.get();
        }
    }
}

//! Main app function
int main(int, char const* argv[])
{
    Bulkmt bulkmt{atoi(argv[1])};

    Metrics logMetrics;
    std::thread log([&]()
    {
        {
            std::unique_lock<std::mutex> locker(Utils::lockPrint);
            std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
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
                locker.unlock();

                content.get();
            }
        }
    });

    FileLogger fileLogger;
    Metrics fileMetricsOne;
    std::thread file1(FileLogWorker, std::ref(fileMetricsOne));

    Metrics fileMetricsTwo;
    std::thread file2(FileLogWorker, std::ref(fileMetricsTwo));

    bulkmt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        bulkmt.mainMetrics.blockCount++;
        bulkmt.mainMetrics.commandCount += group->Size();
    });

    ConsoleLogger consoleLogger;
    bulkmt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        {
            std::lock_guard<std::mutex> lock(lockLoggerQueue);
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

                consoleLogger.Log("bulkmt: " + output.str());
            }, group));
        }
        threadLogCheck.notify_one();
    });

    bulkmt.eventFirstCommand.Subscribe([&](auto&& timestamp)
    {
        std::lock_guard<std::mutex> lock(lockFileQueue);

        std::stringstream currentTime;
        currentTime << timestamp << "_" << std::to_string(bulkmt.mainMetrics.lineCount);
        fileLogger.PrepareFilename("bulkmt" + currentTime.str());
    });

    bulkmt.eventSequenceComplete.Subscribe([&](auto&& group)
    {
        {
            std::lock_guard<std::mutex> lock(lockFileQueue);
            fileQueue.emplace(std::async(std::launch::deferred, [&](const auto& filename, const auto& content) -> int
            {
                fileLogger.Log(filename, content);
                return content->Size();
            }, fileLogger.GetFileName(), group));
        }
        threadFileCheck.notify_one();
    });

    bulkmt.Run();

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

    {
        std::unique_lock<std::mutex> locker(Utils::lockPrint);
        std::cout << "main поток - " << bulkmt.mainMetrics.lineCount << " строк, "
                  << bulkmt.mainMetrics.commandCount << " команд, "
                  << bulkmt.mainMetrics.blockCount << " блок" << std::endl;

        std::cout << "log поток - " << logMetrics.blockCount << " блок, "
                  << logMetrics.commandCount << " команд, " << std::endl;

        std::cout << "file1 поток - " << fileMetricsOne.blockCount << " блок, "
                  << fileMetricsOne.commandCount << " команд, " << std::endl;

        std::cout << "file2 поток - " << fileMetricsTwo.blockCount << " блок, "
                  << fileMetricsTwo.commandCount << " команд, " << std::endl;
    }

    return 0;
}
