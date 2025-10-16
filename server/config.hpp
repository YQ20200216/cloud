// config.hpp
//服务器ip，端口等参数需要灵活配置，对应需要一个配置文件cloud.json
#pragma once
#include "util.hpp"
#include <mutex>
namespace myspace
{
    //单例避免重复读取
    class Config
    {
    public:
        bool ReadConfig(const std::string &filename);
        int GetHotTime()
        {
            return _hot_time;
        }
        int GetServerPort()
        {
            return _server_port;
        }
        std::string GetServerIp()
        {
            return _server_ip;
        }
        std::string GetDownloadPrefix()
        {
            return _download_prefix;
        }
        std::string GetPackFileSuffix()
        {
            return _packfile_suffix;
        }
        std::string GetPackDir()
        {
            return _pack_dir;
        }
        std::string GetBackDir()
        {
            return _back_dir;
        }
        std::string GetBackupFile()
        {
            return _backup_file;
        }
        // 读取配置文件
        void ReadConfigFile()
        {
            FileUtil file("./cloud.json");
            std::string body;
            if (!file.GetContent(&body))
            {
                std::cout << "load config failed\n";
            }
            Json::Value root;
            if (!JsonUtil::UnSerialize(body, &root))
            {
                std::cout << "parse config failed\n";
            }
            _hot_time = root["hot_time"].asInt();
            _server_port = root["server_port"].asInt();
            _server_ip = root["server_ip"].asString();
            _download_prefix = root["download_prefix"].asString();
            _packfile_suffix = root["packfile_suffix"].asString();
            _pack_dir = root["pack_dir"].asString();
            _back_dir = root["back_dir"].asString();
            _backup_file = root["backup_file"].asString();
        }
        static Config &GetInstance()
        {
            static Config _instance;  // 声明
            return _instance;
        }

    private:
        time_t _hot_time;             // 热点判断时间
        int _server_port;             // 服务器监听端口
        std::string _server_ip;       // 服务器ip
        std::string _download_prefix; // 下载的url前缀路径
        std::string _packfile_suffix; // 压缩包的后缀
        std::string _pack_dir;        // 压缩包存放路径
        std::string _back_dir;        // 备份文件存放路径
        std::string _backup_file;     // 备份信息的存放文件

        // 构造函数私有化
        Config() { ReadConfigFile(); }
    };
}
