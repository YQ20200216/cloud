
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
        //��ȡ�ļ���С
        size_t FileSize() {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return -1;
            }
            return st.st_size;
        }
        //��ȡ�ļ����һ���޸�ʱ��
        time_t LastModifyTime() {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return 0;
            }
            return st.st_mtime;
        }
        //��ȡ�ļ����һ�η���ʱ��
        time_t LastAcccessTime() {
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0)
            {
                std::cout << "get file size failed" << std::endl;
                return 0;
            }
            return st.st_atime;
        }
        //��ȡ�ļ�·�����е��ļ���
        std::string FileName() {
            size_t pos = _filename.find_last_of("/\\");//�������һ�� / �� \ ����
            if (pos == std::string::npos)
            {
                return _filename;
            }
            return _filename.substr(pos + 1);
        }
        //���ļ�д������
        bool SetContent(const std::string& body) {
            std::ofstream ofs;
            ofs.open(_filename, std::ios::binary);
            if (!ofs.is_open()) {
                std::cout << "���ļ�ʧ��" << _filename << std::endl;
                return false;
            }
            ofs.write(body.c_str(), static_cast<std::streamsize>(body.size()));
            if (!ofs.good()) {
                std::cout << "д��ʧ��" << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }
        //��ȡ�ļ���������
        bool GetContent(std::string* body) {
            size_t fsize = this->FileSize();
            return GetPosLen(body, 0, fsize);
        }
        //��ȡ�ļ�ָ��λ�� ָ�����ȵ�����
        bool GetPosLen(std::string* body, size_t pos, size_t len) {
            size_t fsize = this->FileSize();
            if (pos + len > fsize)
            {
                std::cout << "���ȴ���\n";
                return false;
            }
            std::ifstream ifs;
            ifs.open(_filename, std::ios::binary);
            if (!ifs.is_open()) {
                std::cout << "���ļ�ʧ��" << _filename << std::endl;
                return false;
            }
            //���ļ���ͷƫ��pos����
            ifs.seekg(pos, std::ios::beg);
            body->resize(len);
            ifs.read(&(*body)[0], len);
            if (!ifs.good()) {
                std::cout << "��ȡʧ��" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }
        
        //�Ƴ�
        bool Remove()
        {
            if (!Exists()) return true;
            remove(_filename.c_str());
            return true;
        }
        //�ж��ļ��Ƿ����
        bool Exists() {
            return std::filesystem::exists(_filename);
        }
        //����Ŀ¼
        bool CreateDirectory() {
            if (this->Exists())return true;
            return std::filesystem::create_directory(_filename);
        }
        //��ȡĿ¼�������ļ���
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
