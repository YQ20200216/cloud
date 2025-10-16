#pragma once
#include <mysql_driver.h>              // MySQL 驱动
#include <mysql_connection.h>          // MySQL 连接
#include <cppconn/statement.h>         // SQL 语句
#include <cppconn/resultset.h>         // 查询结果
#include <cppconn/exception.h>         // 异常处理
#include <iostream>
#include <memory>

namespace myspace
{
    // MySQL 数据库连接管理类（简单版）
    class MySQLManager
    {
    public:
        MySQLManager() : _conn(nullptr) {}

        // 连接数据库
        bool Connect(const std::string& host, 
                     const std::string& user, 
                     const std::string& password, 
                     const std::string& database)
        {
            try {
                // 1. 获取 MySQL 驱动
                sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
                
                // 2. 连接数据库：tcp://127.0.0.1:3306
                std::string url = "tcp://" + host + ":3306";
                _conn.reset(driver->connect(url, user, password));
                
                // 3. 选择要使用的数据库
                _conn->setSchema(database);
                
                std::cout << " MySQL 连接成功：" << database << std::endl;
                return true;
            }
            catch (sql::SQLException& e) {
                std::cerr << " MySQL 连接失败：" << e.what() << std::endl;
                return false;
            }
        }

        // 获取数据库连接（用于执行 SQL）
        std::unique_ptr<sql::Connection>& GetConn()
        {
            return _conn;
        }

    private:
        std::unique_ptr<sql::Connection> _conn;  // 数据库连接对象
    };
}