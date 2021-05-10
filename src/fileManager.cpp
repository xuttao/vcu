#include "fileManager.h"
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <ostream>

#include "common.h"
#include "log.h"

namespace
{
        inline void rmStrSpace(std::string &str)
        {
                for (;;)
                {
                        size_t pos = str.find_first_of(' ');
                        if (pos != std::string::npos)
                        {
                                str.erase(pos, 1);
                        }
                        else
                                break;
                }
        }
} // namespace

FileManager *FileManager::getInstance()
{
        static FileManager ins;
        return &ins;
}

bool FileManager::createDir(const char *_path) const
{
        QDir dir(_path);
        if (!dir.exists())
        {
                return dir.mkpath(_path);
        }
        return true;
}

bool FileManager::createDir(const std::string &_path) const
{
        return createDir(_path.c_str());
}

void FileManager::writeBinaryFile(const void *_pData, int _len, const char *_file) const
{
        // QFile binFile(_file);
        // binFile.open(QIODevice::WriteOnly);
        // binFile.write(_pData,_len);
        // binFile.close();

        std::ofstream out(_file, std::ios::out | std::ios::binary);
        out.write((char *)_pData, _len);
        out.close();
}

bool FileManager::writeTxtFile(std::vector<std::string> &vecRes, const char *_file) const
{
        std::ofstream out(_file, std::ios::out);
        for (int i = 0; i < vecRes.size(); i++)
        {
                out << vecRes[i] << "\n";
        }
        out.close();

        return true;
}

std::vector<std::string> &FileManager::getDetPicFile(const std::string &_pathDir)
{
        return getDetPicFile(_pathDir.c_str());
}

std::vector<std::string> &FileManager::getDetPicFile(const char *_pathDir)
{
        vecDetFile.clear();

        QDir picDir(_pathDir);
        QFileInfo file(_pathDir);
        if (!picDir.exists() || file.isFile())
                return vecDetFile;

        QFileInfoList ls = picDir.entryInfoList(QDir::Files | QDir::CaseSensitive);
        foreach (QFileInfo fileInfo, ls)
        {
                if (fileInfo.suffix() == "png" || fileInfo.suffix() == "jpg" || fileInfo.suffix() == "bmp")
                {
                        vecDetFile.push_back(fileInfo.absoluteFilePath().toStdString());
                }
        }
        return vecDetFile;
}

bool FileManager::parserCfgFile(const char *_file, CfgType _type) const
{
        if (_type == CfgType::Basic)
        {
                return parserBasicCfg(_file);
        }
        else if (_type == CfgType::Net)
        {
                parserCfg(_file);
                return true;
        }
        else
                return false;
}

bool FileManager::parserBasicCfg(const char *_file) const
{
        std::ifstream in(_file, std::ios::in);
        if (!in.is_open())
        {
                return false;
        }

        while (!in.eof() && !in.fail())
        {
                std::string lineStr;
                try
                {
                        getline(in, lineStr);
                        if (lineStr.empty())
                                continue;
                        if (lineStr[0] == '[')
                        {
                                std::string headName = lineStr.substr(1, lineStr.find_last_of(']') - 1);
                                std::string temp;
                                std::map<std::string, std::string> mpVal;
                                getline(in, temp);
                                while (!temp.empty())
                                {
                                        if (temp[0] == '#')
                                        {
                                                getline(in, temp);
                                                continue;
                                        }
                                        int pos = temp.find('=');
                                        int pos2 = temp.find('#');
                                        int len = temp.length();
                                        if (pos2 != std::string::npos)
                                        {
                                                len = pos2;
                                                int postemp = temp.find_first_of(' ');
                                                if (postemp != std::string::npos)
                                                {
                                                        len -= (pos2 - postemp);
                                                }
                                        }

                                        std::string key = temp.substr(0, pos);
                                        std::string val = temp.substr(pos + 1, len - pos - 1);
                                        LOG_DEBUG("%s:%s", key.c_str(), val.c_str());
                                        mpVal.insert(std::make_pair(key, val));
                                        getline(in, temp);
                                }
                                Common::setCfg(headName, mpVal);
                        }
                        else
                        {
                                LOG_WARN("parser cfg file warn:%s:%s", _file, lineStr.c_str());
                        }
                }
                catch (...)
                {
                        LOG_ERR("parser cfg file fail:%s:%s", _file, lineStr.c_str());
                        exit(-1);
                }
        }
        return true;
}

void FileManager::parserCfg(const char *_file) const
{
        std::ifstream in(_file);

        FSERVO_CHECK(in.is_open());

        std::stringstream strStream;
        strStream << in.rdbuf();
        in.close();

        while (!strStream.eof() && !strStream.fail())
        {
                std::string strLine;
                try
                {
                        getline(strStream, strLine);
                        if (strLine.empty())
                                continue;

                        static std::string key;
                        static std::string val;

                        rmStrSpace(strLine); //去除空格

                        if (strLine.empty()) continue;
                        if (strLine[0] == '#') continue; //跳过注释

                        if (strLine[0] == '[')
                        {
                                key = strLine.substr(1, strLine.find_first_of(']') - 1);
                        }
                        else
                        {
                                size_t pos = strLine.find_first_of('#');
                                if (pos != std::string::npos)
                                {
                                        strLine = strLine.substr(0, pos);
                                }
                                size_t posMid = strLine.find_first_of('=');
                                auto str1 = strLine.substr(0, posMid);
                                auto str2 = strLine.substr(posMid + 1, strLine.length() - posMid - 1);
                                Common::setCfg(key, std::make_pair(str1, str2));
                        }
                }
                catch (...)
                {
                        LOG_ERR("parser cfg fail:%s in line:%s", _file, strLine.c_str());
                        exit(-1);
                }
        }
}

char **FileManager::parserClassFile(const char *_file)
{
        std::ifstream in(_file);
        if (!in.is_open())
        {
                LOG_ERR("open file fail:%s", _file);
                exit(-1);
        }
        int index = 0;
        while (!in.eof() && !in.fail())
        {
                std::string lineStr;
                getline(in, lineStr);
                if (lineStr.empty())
                        continue;
                //classArr[index++]=lineStr.c_str();
                classArr[index] = new char[lineStr.length() + 1];
                memset(classArr[index], '\0', lineStr.length() + 1);
                memcpy(classArr[index], lineStr.c_str(), lineStr.length() + 1);
                LOG_DEBUG("%s", classArr[index]);
                index++;
        }

        return classArr;
}