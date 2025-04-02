
//util.hpp
#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include <sys/stat.h>
#include <filesystem>

namespace myspace {
    class FileUtil {
    private:
        std::string _filename;
    public:
        FileUtil(const std::string& filename) :_filename(filename) {}
        //获取文件大小
        size_t FileSize() {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return -1;
            }
            return st.st_size;
        }
        //获取文件最后一次修改时间
        time_t LastModifyTime() {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return 0;
            }
            return st.st_mtime;
        }
        //获取文件最后一次访问时间
        time_t LastAcccessTime() {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return 0;
            }
            return st.st_atime;
        }
        //获取文件路径名中的文件名
        std::string FileName() {
            size_t pos = _filename.find_last_of("/\\");//查找最后一个 / 或 \ 符号
            if (pos == std::string::npos)
            {
                return _filename;
            }
            return _filename.substr(pos + 1);
        }
        //向文件写入数据
        bool SetContent(const std::string& body) {
            std::ofstream ofs;
            ofs.open(_filename, std::ios::binary);
            if (!ofs.is_open()) {
                std::cout << "打开文件失败" << _filename << std::endl;
                return false;
            }
            ofs.write(body.c_str(), static_cast<std::streamsize>(body.size()));
            if (!ofs.good()) {
                std::cout << "写入失败" << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }
        //获取文件所有数据
        bool GetContent(std::string* body) {
            size_t fsize = this->FileSize();
            return GetPosLen(body, 0, fsize);
        }
        //获取文件指定位置 指定长度的数据
        bool GetPosLen(std::string* body, size_t pos, size_t len) {
            size_t fsize = this->FileSize();
            if (pos + len > fsize)
            {
                std::cout << "长度错误\n";
                return false;
            }
            std::ifstream ifs;
            ifs.open(_filename, std::ios::binary);
            if (!ifs.is_open()) {
                std::cout << "打开文件失败" << _filename << std::endl;
                return false;
            }
            //从文件开头偏移pos长度
            ifs.seekg(pos, std::ios::beg);
            body->resize(len);
            ifs.read(&(*body)[0], len);
            if (!ifs.good()) {
                std::cout << "读取失败" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }
        
        //移除
        bool Remove()
        {
            if (!Exists()) return true;
            remove(_filename.c_str());
            return true;
        }
        //判断文件是否存在
        bool Exists() {
            return std::filesystem::exists(_filename);
        }
        //创建目录
        bool CreateDirectory() {
            if (this->Exists())return true;
            return std::filesystem::create_directory(_filename);
        }
        //获取目录下所有文件名
        bool GetDirectory(std::vector<std::string>& arry) {
            for (auto& p : std::filesystem::directory_iterator(_filename))
            {
                if (std::filesystem::is_directory(p)) continue;
                arry.push_back(std::filesystem::path(p).relative_path().string());
            }
            return true;
        }
    };
}
