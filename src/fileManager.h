#pragma once

#include <iostream>
#include <map>
#include <vector>
enum class CfgType
{
        Basic,
        Net,
};

class FileManager
{

private:
        FileManager() {}
        ~FileManager() {}

public:
        static FileManager *getInstance();

        bool createDir(const std::string &_path) const;
        bool createDir(const char *_path) const;

        void writeBinaryFile(const void *_pData, int _len, const char *_file) const;
        bool writeTxtFile(std::vector<std::string> &vecPos, const char *_file) const;

        std::vector<std::string> &getDetPicFile(const std::string &_path);
        std::vector<std::string> &getDetPicFile(const char *_path);

        bool parserCfgFile(const char *_file, CfgType _type) const;
        char **parserClassFile(const char *_file);

private:
        char *classArr[80];
        std::vector<std::string> vecDetFile;
        bool parserBasicCfg(const char *_file) const;
        void parserCfg(const char *_file) const;
};