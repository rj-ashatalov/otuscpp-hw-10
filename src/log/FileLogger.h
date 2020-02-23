#pragma once

#include <string>
#include <fstream>

struct FileLogger
{
        struct File
        {
            std::string name;
            std::shared_ptr<Group> content;
        };

        void PrepareFilename(std::string fileName)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            _fileName = fileName;
        };

        void Log(const std::string& fileName, const std::shared_ptr<Group> group)
        {
            if (fileName == "")
            {
                return;
            }

            std::cout << __PRETTY_FUNCTION__ << " Creating file: " << fileName << std::endl;
            std::ofstream fileStream(fileName + ".log");

            auto commands = group->Merge();
            std::for_each(commands.begin(), commands.end(), [&fileStream](auto& item)
            {
                fileStream << Utils::FibonacciNaive(std::stoi(item->value)) << "\n";
            });
            fileStream << std::endl;
            fileStream.close();
        };

        std::string GetFileName()
        {
            return _fileName;
        }

    private:
        std::string _fileName = "";
};