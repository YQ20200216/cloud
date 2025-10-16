// data.hpp
#pragma once
#include <unordered_map>
#include <shared_mutex>
#include "util.hpp"
#include "config.hpp"
namespace myspace
{
    // 数据信息结构体
    typedef struct BackupInfo
    {
        std::string _real_path; // 文件实际存储路径名
        std::string _pack_path; // 压缩报存储路径名
        bool _pack_flag;        // 是否压缩标志
        size_t _fsize;          // 文件大小
        time_t _mtime;          // 最后修改时间
        time_t _atime;          // 最后访问时间
        std::string _url_path;  // 请求的资源路径
        // 数据填充（获取各项属性信息，存储到BackupInfo结构体）
        bool NewBackupInfo(const std::string &realpath)
        {
            FileUtil file(realpath);
            if (!file.Exists())
            {
                std::cout << "new backupfile file not find\n";
                return false;
            }
            Config config = Config::GetInstance();
            std::string packdir = config.GetPackDir();
            std::string packsuffix = config.GetPackFileSuffix();
            std::string downloadprefix = config.GetDownloadPrefix();
            _real_path=realpath;
            _fsize = file.FileSize();
            _mtime = file.LastModifyTime();
            _atime = file.LastAcccessTime();
            // ./backdir/a.txt --> ./packdir/a.txt.lz
            _pack_path = packdir + file.FileName() + packsuffix;
            // ./backdir/a.txt --> /download/a.txt
            _url_path = downloadprefix + file.FileName();
            return true;
        }
    } BackupInfo;

    // 数据管理类
    class DataManager
    {
    public:
        DataManager()
        {
            _backup_file = Config::GetInstance().GetBackupFile();
            InitLoad();//启动时加载cloud.data中备份信息
        }
        // 初始化加载，每次系统重启都要加载以嵌的数据
        bool InitLoad()
        {
            // 1.读取backup_file备份信息的存放文件中的数据
            FileUtil f(_backup_file);
            if (!f.Exists())
            {
                return true;
            }
            std::string body;
            f.GetContent(&body);
            // 2.反序列化
            Json::Value root;
            JsonUtil::UnSerialize(body, &root);
            // 3.将反序列化得到的Json::Value中的数据添加到table中
            for (int i = 0; i < root.size(); i++)
            {
                BackupInfo info;
                info._pack_flag = root[i]["pack_flag"].asBool();
                info._fsize = root[i]["fsize"].asInt64();
                info._atime = root[i]["atime"].asInt64();
                info._mtime = root[i]["mtime"].asInt64();
                info._pack_path = root[i]["pack_path"].asString();
                info._real_path = root[i]["real_path"].asString();
                info._url_path = root[i]["url_path"].asString();
                Insert(info);
            }
            return true;
        }
        // 把信息存档到json当中
        bool Storage()
        {
            // 1.获取所有数据
            std::vector<BackupInfo> arr;
            this->GetAll(&arr);
            // 2.填充到Json::Value中
            Json::Value root;
            for (int i = 0; i < arr.size(); i++)
            {
                Json::Value val;
                val["pack_flag"] = arr[i]._pack_flag;
                val["fsize"] = static_cast<Json::Int64>(arr[i]._fsize);
                val["atime"] = static_cast<Json::Int64>(arr[i]._atime);
                val["real_path"] = arr[i]._real_path;
                val["pack_path"] = arr[i]._pack_path;
                val["url_path"] = arr[i]._url_path;
                root.append(val);
            }
            // 3.对Json::Value序列化
            std::string body;
            JsonUtil::Serialize(root, &body);
            // 4.写文件
            FileUtil f(_backup_file);
            f.SetContent(body);
            return true;
        }
        // 新增
        bool Insert(const BackupInfo &info)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            _table[info._url_path] = info;
            Storage();
            std::cout << "插入的数据 for URL: " << info._url_path << std::endl;
            return true;
        }
        // 修改
        bool Update(const BackupInfo &info)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            _table[info._url_path] = info;
            Storage();
            return true;
        }
        // 根据请求url获取对应文件信息（用户根据url请求下载文件）
        bool GetOneByURL(const std::string &url, BackupInfo &info)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            auto it = _table.find(url);      // url是key值直接find查找
            if (it == _table.end())
            {
                return false;
            }
            info = it->second;
            return true;
        }
        // 根据真实路径获取文件信息，(服务器端测备份文件 热点文件判断）
        bool GetOneByRealPath(const std::string &realpath, BackupInfo& info)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            auto it = _table.begin();
            // 真实路径需要遍历unordered_map 中second的real_path
            while (it != _table.end())
            {
                if (it->second._real_path == realpath)
                {
                    info = it->second;
                    return true;
                }
                it++;
            }
            return false;
        }
        // 获取所有文件信息
        bool GetAll(std::vector<BackupInfo> *arry)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            // 遍历
            auto it = _table.begin();
            for (; it != _table.end(); it++)
            {
                arry->push_back(it->second);
            }
            return true;
        }
        private:
            std::string _backup_file;                           // 备份信息存放文件
            std::shared_mutex _rwlock;                           // 读写锁，读共享，写互斥
            std::unordered_map<std::string, BackupInfo> _table; // 内存中hash存储的文件信息管理表
    };
}
