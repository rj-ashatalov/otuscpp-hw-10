#pragma once

#include <string>
#include <fstream>

struct FileLogger
{
        void PrepareFilename(std::string fileName)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            _fileName = fileName;
        };

        void Log(std::string message)
        {
            if (_fileName == "")
            {
                return;
            }

            std::cout << __PRETTY_FUNCTION__ << " Creating file: " << _fileName << std::endl;
            std::ofstream file(_fileName + ".log");
            file << message << std::endl;
            file.close();
            _fileName = "";
        };
    private:
        std::string _fileName = "";
};