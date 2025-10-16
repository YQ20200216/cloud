//util.hpp
#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include <sys/stat.h>
#include <experimental/filesystem>
#include<jsoncpp/json/json.h>
#include"bundle.h"

namespace myspace{

    class JsonUtil{
        public:
            //序列化
            static bool Serialize(const Json::Value &root, std::string *str)
            {
                Json::StreamWriterBuilder swb;//流写入构建器
                std::unique_ptr<Json::StreamWriter> writer(swb.newStreamWriter());//用writer指向实例化的对象
                std::stringstream sstream;
                writer->write(root,&sstream);//将json数据写入字节流
                *str=sstream.str();//将字节流转为字符串
                return true;
            }
            //反序列化
            static bool UnSerialize(const std::string &str, Json::Value *root)
            {
                std::string err;
                Json::CharReaderBuilder crb;//字符读取构建器
                std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
                cr->parse(str.c_str(),str.c_str()+str.size(),root,&err);//参数：字符串起始，字符串结束，输出Value指针，错误信息指针
                return true;
            }
    };

    class FileUtil{
    private:
        std::string _filename;//文件路径
    public:
        FileUtil(const std::string& filename):_filename(filename) {}
        //获取文件大小
        size_t FileSize(){
            struct stat st;//保存文件详细属性信息
            if(stat(_filename.c_str(),&st)<0)//将文件元数据存入结构体
            {
                std::cout<<"get file size failed"<<std::endl;
                return -1;
            }
            return st.st_size;
        }
        //获取文件最后一次修改时间
        time_t LastModifyTime(){
            struct stat st;
            if(stat(_filename.c_str(),&st)<0)
            {
                std::cout<<"get file modify time failed"<<std::endl;
                return 0;
            }
            return st.st_mtime;
        }
        //获取文件最后一次访问时间
        time_t LastAcccessTime(){
            struct stat st;
            if(stat(_filename.c_str(),&st)<0)
            {
                std::cout<<"get file last access time failed"<<std::endl;
                return 0;
            }
            return st.st_atime;
        }
        //获取文件路径名中的文件名
        std::string FileName(){
            size_t pos = _filename.find_last_of("/\\");//查找最后一个 / 或 \ 符号
            if (pos == std::string::npos)
            {
                return _filename;
            }
            return _filename.substr(pos + 1);
        }
        //向文件写入数据
        bool SetContent(const std::string& body){
            std::ofstream ofs;
            ofs.open(_filename,std::ios::binary);
            if (!ofs.is_open()) {
                std::cout << "打开文件失败" << _filename << std::endl;
                return false;
            }
            ofs.write(body.c_str(),body.size());
            if (!ofs.good()) {
                std::cout << "写入失败" << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }
        //获取文件所有数据
        bool GetContent(std::string *body){
            size_t fsize=this->FileSize();
            return GetPosLen(body,0,fsize);
        }
        //获取文件指定位置 指定长度的数据，断电续传有关
        bool GetPosLen(std::string *body, size_t pos, size_t len){
            size_t fsize=this->FileSize();
            if(pos+len>fsize)
            {
                std::cout<<"长度错误\n";
                return false;
            }
            std::ifstream ifs;
            ifs.open(_filename,std::ios::binary);
            if (!ifs.is_open()) {
                std::cout << "打开文件失败" << _filename << std::endl;
                return false;
            }
            ifs.seekg(pos, std::ios::beg);//将文件指针移动到pos位置
            body->resize(len);
            ifs.read(&(*body)[0],len);//从当前位置读取 len 个字节
            if (!ifs.good()) {
                std::cout << "读取失败" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }
        //压缩文件
        bool Compress(const std::string& packname){
            //1.获取原文件数据
            std::string body;
            if(!this->GetContent(&body))
            {
                std::cout<<"压缩失败"<<std::endl;
                return false;
            }
            //2.对数据进行压缩bundle::pack
            std::string packed=bundle::pack(bundle::LZIP, body);
            //3.压缩后的数据存储到压缩文件中
            FileUtil fu(packname);
            if(!fu.SetContent(packed))
            {
                std::cout<<"压缩失败"<<std::endl;
                return false;
            }
            return true;
        }
        //解压文件
        bool UnCompress(const std::string& filename){
            //1.获取当前压缩包得数据
            std::string body;
            if(!this->GetContent(&body))
            {
                std::cout<<"解压失败"<<std::endl;
                return false;
            }
            //2.对数据进行解压bundle::unpack
            std::string unpacked=bundle::unpack(body);
            //3.将解压后的数据保存到新文件中
            FileUtil fu(filename);
            if(!fu.SetContent(unpacked))
            {
                std::cout<<"解压失败"<<std::endl;
                return false;
            }
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
        bool Exists(){
            return std::experimental::filesystem::exists(_filename);
        }
        //创建目录
        bool CreateDirectory(){
            if(this->Exists())return true;
            return std::experimental::filesystem::create_directory(_filename);
        }
        //获取目录下所有文件名
        bool GetDirectory(std::vector<std::string>& arry){
            for(auto& p:std::experimental::filesystem::directory_iterator(_filename))
            {
                if(std::experimental::filesystem::is_directory(p)) continue;
                arry.push_back(std::experimental::filesystem::path(p).relative_path().string());
            }
            return true;
        }
    };
}
