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

        void Log(File file)
        {
            if (file.name == "")
            {
                return;
            }

            std::cout << __PRETTY_FUNCTION__ << " Creating file: " << file.name << std::endl;
            std::ofstream fileStream(file.name + ".log");
            fileStream << Utils::Join(file.content->expressions, "\n") << std::endl;
            fileStream.close();
        };

        std::string& GetFileName()
        {
            return _fileName;
        }

    private:
        std::string _fileName = "";
};