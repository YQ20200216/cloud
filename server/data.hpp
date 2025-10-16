// data.hpp - MySQL 版本（简化版）
#pragma once
#include <unordered_map>
#include <shared_mutex>
#include <vector>
#include <sstream>
#include "util.hpp"
#include "config.hpp"
#include "mysql_manager.hpp"

namespace myspace
{
    // 数据信息结构体
    typedef struct BackupInfo
    {
        int _id = 0;            // 数据库主键ID
        std::string _real_path; // 文件实际存储路径名
        std::string _pack_path; // 压缩包存储路径名
        bool _pack_flag;        // 是否压缩标志
        size_t _fsize;          // 文件大小
        time_t _mtime;          // 最后修改时间
        time_t _atime;          // 最后访问时间
        std::string _url_path;  // 请求的资源路径
        
        // 数据填充
        bool NewBackupInfo(const std::string &realpath)
        {
            FileUtil file(realpath);
            if (!file.Exists())
            {
                std::cout << "文件不存在: " << realpath << std::endl;
                return false;
            }
            Config config = Config::GetInstance();
            _real_path = realpath;
            _fsize = file.FileSize();
            _mtime = file.LastModifyTime();
            _atime = file.LastAcccessTime();
            _pack_flag = false;
            _pack_path = config.GetPackDir() + file.FileName() + config.GetPackFileSuffix();
            _url_path = config.GetDownloadPrefix() + file.FileName();
            return true;
        }
    } BackupInfo;

    // 数据管理类
    class DataManager
    {
    public:
        DataManager()
        {
            // 数据库连接信息（修改为您的实际信息）
            std::string db_host = "127.0.0.1";
            std::string db_user = "root";
            std::string db_password = "123456";
            std::string db_name = "cloud_backup";
            
            // 创建并连接数据库
            _mysql = new MySQLManager();
            if (!_mysql->Connect(db_host, db_user, db_password, db_name))
            {
                std::cerr << "数据库连接失败！" << std::endl;
                exit(1);
            }
            
            // 启动时从数据库加载数据到内存
            InitLoad();
        }
        
        ~DataManager()
        {
            if (_mysql) delete _mysql;
        }
        
        // 从 MySQL 加载所有数据到内存
        bool InitLoad()
        {
            std::unique_lock<std::shared_mutex> lock(_rwlock);
            
            try {
                auto& conn = _mysql->GetConn();
                if (!conn) return false;
                
                // 查询所有数据
                std::unique_ptr<sql::Statement> stmt(conn->createStatement());//创建一个 SQL 语句执行器对象
                std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM backup_info"));//执行 SELECT 查询语句，返回到结果集指针
                
                int count = 0;
                
                // 加载到内存
                while (res->next()) {
                    BackupInfo info;
                    info._id = res->getInt("id");
                    info._url_path = res->getString("url_path");
                    info._real_path = res->getString("real_path");
                    info._pack_path = res->getString("pack_path");
                    info._pack_flag = res->getBoolean("pack_flag");
                    info._fsize = res->getUInt64("fsize");
                    info._mtime = res->getInt64("mtime");
                    info._atime = res->getInt64("atime");
                    
                    _table[info._url_path] = info;
                    count++;
                }
                
                std::cout << "加载了 " << count << " 条备份记录" << std::endl;
                return true;
            }
            catch (sql::SQLException& e) {
                std::cerr << "加载失败: " << e.what() << std::endl;
                return false;
            }
        }
        
        // 插入数据
        bool Insert(const BackupInfo &info)
        {
            std::unique_lock<std::shared_mutex> lock(_rwlock);
            
            try {
                auto& conn = _mysql->GetConn();
                if (!conn) return false;
                
                // 构造 SQL 语句
                std::stringstream sql;
                sql << "INSERT INTO backup_info "
                    << "(url_path, real_path, pack_path, pack_flag, fsize, mtime, atime) "
                    << "VALUES ('"
                    << info._url_path << "', '"
                    << info._real_path << "', '"
                    << info._pack_path << "', "
                    << (info._pack_flag ? 1 : 0) << ", "
                    << info._fsize << ", "
                    << info._mtime << ", "
                    << info._atime << ") "
                    << "ON DUPLICATE KEY UPDATE "
                    << "real_path='" << info._real_path << "', "
                    << "pack_path='" << info._pack_path << "', "
                    << "pack_flag=" << (info._pack_flag ? 1 : 0) << ", "
                    << "fsize=" << info._fsize << ", "
                    << "mtime=" << info._mtime << ", "
                    << "atime=" << info._atime;
                
                // 执行 SQL
                std::unique_ptr<sql::Statement> stmt(conn->createStatement());
                stmt->executeUpdate(sql.str());
                
                // 更新内存缓存
                _table[info._url_path] = info;
                
                std::cout << "插入成功: " << info._url_path << std::endl;
                return true;
            }
            catch (sql::SQLException& e) {
                std::cerr << "插入失败: " << e.what() << std::endl;
                return false;
            }
        }
        
        // 更新数据
        bool Update(const BackupInfo &info)
        {
            std::unique_lock<std::shared_mutex> lock(_rwlock);
            
            try {
                auto& conn = _mysql->GetConn();
                if (!conn) return false;
                
                // 构造 SQL 语句
                std::stringstream sql;
                sql << "UPDATE backup_info SET "
                    << "real_path='" << info._real_path << "', "
                    << "pack_path='" << info._pack_path << "', "
                    << "pack_flag=" << (info._pack_flag ? 1 : 0) << ", "
                    << "fsize=" << info._fsize << ", "
                    << "mtime=" << info._mtime << ", "
                    << "atime=" << info._atime << " "
                    << "WHERE url_path='" << info._url_path << "'";
                
                // 执行 SQL
                std::unique_ptr<sql::Statement> stmt(conn->createStatement());
                int rows = stmt->executeUpdate(sql.str());
                
                if (rows > 0) {
                    // 更新内存缓存
                    _table[info._url_path] = info;
                    std::cout << "更新成功: " << info._url_path << std::endl;
                    return true;
                } else {
                    std::cerr << "更新失败: 未找到记录" << std::endl;
                    return false;
                }
            }
            catch (sql::SQLException& e) {
                std::cerr << "更新失败: " << e.what() << std::endl;
                return false;
            }
        }
        
        // 根据 URL 获取信息（从内存获取，速度快）
        bool GetOneByURL(const std::string &url, BackupInfo &info)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            auto it = _table.find(url);
            if (it == _table.end()) {
                return false;
            }
            info = it->second;
            return true;
        }
        
        // 根据真实路径获取信息（从内存遍历查找）
        bool GetOneByRealPath(const std::string &realpath, BackupInfo& info)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            for (const auto& pair : _table) {
                if (pair.second._real_path == realpath) {
                    info = pair.second;
                    return true;
                }
            }
            return false;
        }
        
        // 获取所有信息（从内存获取）
        bool GetAll(std::vector<BackupInfo> *arry)
        {
            std::shared_lock<std::shared_mutex> lock(_rwlock);
            for (const auto& pair : _table) {
                arry->push_back(pair.second);
            }
            return true;
        }
        
    private:
        MySQLManager* _mysql;                                // MySQL 连接管理
        std::shared_mutex _rwlock;                           // 读写锁
        std::unordered_map<std::string, BackupInfo> _table; // 内存缓存
    };
}
