# mysql_manager.hpp 代码流程详解

## 📚 文件概述

`mysql_manager.hpp` 是一个简洁的 MySQL 数据库连接管理类，封装了 MySQL C++ Connector 的基本操作。

---

## 🎯 核心功能

1. **连接管理**：创建和管理 MySQL 数据库连接
2. **智能指针**：自动管理连接生命周期
3. **异常处理**：捕获并友好提示连接错误

---

## 📝 完整代码结构

```cpp
namespace myspace
{
    class MySQLManager
    {
    public:
        MySQLManager();                    // 构造函数
        bool Connect(...);                 // 连接数据库
        std::unique_ptr<sql::Connection>& GetConn();  // 获取连接
    
    private:
        std::unique_ptr<sql::Connection> _conn;  // 数据库连接对象
    };
}
```

---

## 🔍 详细代码流程

### 1. 构造函数

```cpp
MySQLManager() : _conn(nullptr) {}
```

**功能**：
- 初始化 `_conn` 智能指针为 `nullptr`
- 准备接收数据库连接对象

**时机**：
- 在 `DataManager` 构造函数中调用
- 程序启动时创建

---

### 2. Connect() 函数 - 连接数据库

```cpp
bool Connect(const std::string& host, 
             const std::string& user, 
             const std::string& password, 
             const std::string& database)
```

#### **完整流程图**：

```
┌──────────────────────────────────────┐
│  1. try { 开始异常捕获               │
└────────────┬─────────────────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│  2. 获取 MySQL 驱动实例               │
│  sql::mysql::MySQL_Driver* driver    │
│    = get_mysql_driver_instance();    │
└────────────┬─────────────────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│  3. 构建连接 URL                      │
│  std::string url                      │
│    = "tcp://" + host + ":3306";      │
│  示例: "tcp://127.0.0.1:3306"        │
└────────────┬─────────────────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│  4. 创建数据库连接                    │
│  _conn.reset(                         │
│    driver->connect(url, user, pwd)   │
│  );                                   │
└────────────┬─────────────────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│  5. 选择要使用的数据库                │
│  _conn->setSchema(database);          │
│  示例: 选择 "cloud_backup" 数据库    │
└────────────┬─────────────────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│  6. 输出成功信息                      │
│  std::cout << "MySQL 连接成功"        │
│  return true;                         │
└────────────┬─────────────────────────┘
             │
             ▼
┌──────────────────────────────────────┐
│  } catch(sql::SQLException& e) {     │
│    输出错误信息                       │
│    return false;                      │
│  }                                    │
└──────────────────────────────────────┘
```

#### **详细步骤解析**：

##### **步骤 1：获取 MySQL 驱动（第 26 行）**
```cpp
sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
```

**作用**：
- 获取 MySQL C++ Connector 驱动的单例实例
- 这是连接 MySQL 的入口点

**类比**：
- 就像使用打印机前，先安装打印机驱动

##### **步骤 2：构建连接 URL（第 29-30 行）**
```cpp
std::string url = "tcp://" + host + ":3306";
_conn.reset(driver->connect(url, user, password));
```

**URL 格式**：
```
tcp://主机地址:端口号
```

**示例**：
```
tcp://127.0.0.1:3306       // 本地连接
tcp://192.168.1.100:3306   // 远程连接
```

**参数说明**：
- `host`：数据库服务器地址
- `user`：数据库用户名（如：root）
- `password`：数据库密码（如：123456）

##### **步骤 3：选择数据库（第 33 行）**
```cpp
_conn->setSchema(database);
```

**作用**：
- 选择要操作的数据库
- 等价于 SQL: `USE cloud_backup;`

##### **步骤 4：异常处理（第 38-41 行）**
```cpp
catch (sql::SQLException& e) {
    std::cerr << "MySQL 连接失败：" << e.what() << std::endl;
    return false;
}
```

**捕获的常见错误**：
- 用户名或密码错误
- 数据库不存在
- MySQL 服务未启动
- 网络连接失败

---

### 3. GetConn() 函数 - 获取连接

```cpp
std::unique_ptr<sql::Connection>& GetConn()
{
    return _conn;
}
```

#### **功能**：
- 返回数据库连接的智能指针引用
- 供 `data.hpp` 调用执行 SQL 语句

#### **返回类型解析**：
```cpp
std::unique_ptr<sql::Connection>&
│       │               │        │
│       │               │        └─ 引用（避免拷贝）
│       │               └─ 连接对象类型
│       └─ 智能指针（自动管理内存）
└─ C++ 标准库
```

#### **使用示例**：
```cpp
// 在 data.hpp 中使用
auto& conn = _mysql->GetConn();  // 获取连接引用
if (!conn) {
    // 连接无效
    return false;
}

// 创建 SQL 语句执行器
std::unique_ptr<sql::Statement> stmt(conn->createStatement());

// 执行 SQL 查询
stmt->executeQuery("SELECT * FROM backup_info");
```

---

## 🔄 完整使用流程示例

### 场景：程序启动时连接数据库

```cpp
// 在 DataManager 构造函数中（data.hpp）

// 1. 创建 MySQLManager 实例
_mysql = new MySQLManager();

// 2. 连接数据库
if (!_mysql->Connect("127.0.0.1", "root", "123456", "cloud_backup")) {
    std::cerr << "数据库连接失败！" << std::endl;
    exit(1);
}
// 此时已成功连接，输出：✓ MySQL 连接成功：cloud_backup

// 3. 加载数据
InitLoad();  // 从数据库加载历史数据
```

### 场景：插入数据到数据库

```cpp
// 在 DataManager::Insert() 中

// 1. 获取数据库连接
auto& conn = _mysql->GetConn();
if (!conn) return false;

// 2. 创建语句执行器
std::unique_ptr<sql::Statement> stmt(conn->createStatement());

// 3. 构造并执行 SQL
std::stringstream sql;
sql << "INSERT INTO backup_info (url_path, real_path, fsize) "
    << "VALUES ('/download/test.txt', '/backup/test.txt', 1024)";

stmt->executeUpdate(sql.str());

// 4. SQL 执行完成，数据已插入数据库
```

---

## 🎯 关键设计特点

### 1. 智能指针管理

```cpp
std::unique_ptr<sql::Connection> _conn;
```

**优点**：
- ✅ 自动释放资源（无需手动 `delete`）
- ✅ 防止内存泄漏
- ✅ 异常安全

**原理**：
```cpp
{
    MySQLManager mysql;
    mysql.Connect(...);  // _conn 指向数据库连接
    // ... 使用连接 ...
}  // ← 离开作用域，_conn 自动释放连接
```

### 2. 简洁的接口

**只提供两个核心方法**：
- `Connect()` - 建立连接
- `GetConn()` - 获取连接

**好处**：
- 易于理解
- 不易出错
- 专注核心功能

### 3. 异常处理机制

```cpp
try {
    // 连接操作
} catch (sql::SQLException& e) {
    // 友好提示错误
}
```

**捕获所有 MySQL 异常**：
- 连接失败
- 认证失败
- 数据库不存在
- 网络错误

---

## 💡 与其他模块的协作

```
┌─────────────────────────────────────────┐
│           DataManager (data.hpp)         │
│                                          │
│  1. 创建 MySQLManager 实例               │
│     _mysql = new MySQLManager()          │
│                                          │
│  2. 连接数据库                           │
│     _mysql->Connect(...)                 │
│                                          │
│  3. 执行 SQL 操作                        │
│     auto& conn = _mysql->GetConn()       │
│     stmt = conn->createStatement()       │
│     stmt->executeQuery(...)              │
└─────────────┬───────────────────────────┘
              │ 使用
              ▼
┌─────────────────────────────────────────┐
│      MySQLManager (mysql_manager.hpp)    │
│                                          │
│  • Connect() - 连接数据库                │
│  • GetConn() - 返回连接对象              │
│  • _conn     - 智能指针管理连接          │
└─────────────┬───────────────────────────┘
              │ 封装
              ▼
┌─────────────────────────────────────────┐
│     MySQL C++ Connector                  │
│     (libmysqlcppconn)                    │
│                                          │
│  • MySQL_Driver                          │
│  • Connection                            │
│  • Statement                             │
│  • ResultSet                             │
└─────────────┬───────────────────────────┘
              │ 底层通信
              ▼
┌─────────────────────────────────────────┐
│         MySQL 数据库服务器               │
│         (mysqld)                         │
└─────────────────────────────────────────┘
```

---

## 📊 时序图

```
程序启动    MySQLManager    MySQL C++ Connector    MySQL 服务器
   │              │                 │                    │
   ├─new──────────>│                 │                    │
   │              │                 │                    │
   ├─Connect()────>│                 │                    │
   │              │                 │                    │
   │              ├─get_driver()───>│                    │
   │              │<────driver──────┤                    │
   │              │                 │                    │
   │              ├─connect()───────>│                    │
   │              │                 ├─TCP连接───────────>│
   │              │                 │<──连接成功──────────┤
   │              │<───Connection───┤                    │
   │              │                 │                    │
   │              ├─setSchema()────>│                    │
   │              │                 ├─USE database──────>│
   │              │                 │<──OK────────────────┤
   │              │                 │                    │
   │<─true────────┤                 │                    │
   │              │                 │                    │
   ├─GetConn()───>│                 │                    │
   │<─Connection──┤                 │                    │
   │              │                 │                    │
```

---

## ⚙️ 配置参数说明

### 连接参数（data.hpp 中配置）

```cpp
std::string db_host = "127.0.0.1";      // MySQL 服务器地址
std::string db_user = "root";           // 数据库用户名
std::string db_password = "123456";     // 数据库密码
std::string db_name = "cloud_backup";   // 数据库名称
```

### 常见配置示例

| 场景 | host | user | database |
|------|------|------|----------|
| 本地开发 | `127.0.0.1` | `root` | `cloud_backup` |
| 远程服务器 | `192.168.1.100` | `cloud_user` | `cloud_backup` |
| Docker 容器 | `mysql` | `root` | `cloud_backup` |

---

## 🔧 故障排查

### 1. 连接失败常见原因

```
错误信息 → 可能原因 → 解决方法
```

**Access denied**
- 原因：用户名或密码错误
- 解决：重置密码 `ALTER USER ...`

**Unknown database**
- 原因：数据库不存在
- 解决：创建数据库 `CREATE DATABASE cloud_backup`

**Can't connect to MySQL server**
- 原因：MySQL 服务未启动
- 解决：`sudo systemctl start mysql`

**Unknown host**
- 原因：主机地址错误
- 解决：检查 `db_host` 配置

---

## 📚 总结

### MySQLManager 的职责
1. ✅ 封装 MySQL 连接细节
2. ✅ 提供简洁的连接接口
3. ✅ 管理连接生命周期
4. ✅ 处理连接异常

### 使用流程
```
创建实例 → 连接数据库 → 获取连接 → 执行 SQL
```

### 核心优势
- **简单**：只有两个方法
- **安全**：智能指针自动管理
- **可靠**：完善的异常处理

