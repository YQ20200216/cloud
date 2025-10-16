# C++ 云备份系统

## 项目介绍

基于 C++ 开发的云备份系统服务器端，支持文件自动备份、压缩存储和网络下载功能。系统运行在 Linux 平台，提供 HTTP API 接口供客户端访问。

### 主要功能

- **自动备份**：监控指定目录，自动备份文件
- **智能压缩**：对长时间未访问的热点文件进行自动压缩存储
- **HTTP 服务**：提供文件上传和下载的 HTTP 接口
- **数据持久化**：使用 MySQL 数据库存储备份信息，支持高并发访问
- **多线程处理**：热点管理和 HTTP 服务并行运行
- **断点续传**：支持大文件的断点续传下载

## 软件架构

```
cloud/server/            # 服务器端（Linux）
├── cloud.cpp            # 主程序入口
├── config.hpp           # 配置管理模块
├── data.hpp             # 数据管理模块（MySQL版）
├── mysql_manager.hpp    # MySQL 数据库连接管理
├── hot.hpp              # 热点文件管理
├── server.hpp           # HTTP 服务模块
├── util.hpp             # 工具类（文件操作、压缩）
├── bundle.h/cpp         # cpp-httplib 库
├── cloud.json           # 配置文件
├── create_tables.sql    # 数据库表结构
└── Makefile             # 构建文件
```

## 开发思路与设计理念

### 核心设计思想

本系统采用**分层架构设计**，遵循**模块化、低耦合、高内聚**的设计原则，通过合理的层次划分实现功能的清晰分离和代码的可维护性。

### 技术特点

1. **多线程并发处理**：热点文件管理和 HTTP 服务分离，互不阻塞
2. **智能存储优化**：基于访问时间的热点判断，自动压缩冷数据
3. **断点续传支持**：通过 ETag 和 Range 实现可靠的文件下载
4. **数据持久化**：JSON 格式存储备份信息，系统重启后自动恢复
5. **单例配置管理**：全局统一的配置访问接口
6. **线程安全设计**：使用读写锁保护共享数据结构

## 分层架构详解

系统采用自底向上的五层架构设计：

```
┌─────────────────────────────────────────────────────────────┐
│                      应用层 (Application)                    │
│                   cloud.cpp - 主程序入口                     │
│          • 初始化数据管理模块                                 │
│          • 启动热点管理线程                                   │
│          • 启动 HTTP 服务线程                                │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    业务逻辑层 (Business Logic)                │
│  ┌──────────────────────┐      ┌──────────────────────┐    │
│  │   hot.hpp            │      │   server.hpp         │    │
│  │   热点文件管理        │      │   HTTP 服务管理       │    │
│  ├──────────────────────┤      ├──────────────────────┤    │
│  │ • 定时扫描备份目录    │      │ • POST /upload       │    │
│  │ • 判断文件访问时间    │      │ • GET /listshow      │    │
│  │ • 压缩非热点文件      │      │ • GET /download/*    │    │
│  │ • 删除源文件          │      │ • 文件上传处理       │    │
│  │ • 更新备份信息        │      │ • 文件列表展示       │    │
│  └──────────────────────┘      │ • 文件下载 (断点续传) │    │
│                                 │ • 自动解压处理        │    │
│                                 └──────────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    数据管理层 (Data Management)               │
│                   data.hpp - 数据管理（MySQL版）              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ BackupInfo 结构体：文件备份信息                       │   │
│  │  • _id             - 数据库主键ID                     │   │
│  │  • _real_path      - 文件实际存储路径                 │   │
│  │  • _pack_path      - 压缩包存储路径                   │   │
│  │  • _pack_flag      - 压缩标志                         │   │
│  │  • _fsize          - 文件大小                         │   │
│  │  • _mtime          - 最后修改时间                     │   │
│  │  • _atime          - 最后访问时间                     │   │
│  │  • _url_path       - 请求资源路径                     │   │
│  └─────────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ DataManager 类：数据管理核心                          │   │
│  │  • Insert()        - 新增备份信息（写入MySQL+内存）   │   │
│  │  • Update()        - 更新备份信息（写入MySQL+内存）   │   │
│  │  • GetOneByURL()   - 根据URL查询（从内存读取）        │   │
│  │  • GetOneByRealPath() - 根据实际路径查询（从内存）    │   │
│  │  • GetAll()        - 获取所有备份信息（从内存）       │   │
│  │  • InitLoad()      - 启动时从MySQL加载到内存          │   │
│  └─────────────────────────────────────────────────────┘   │
│  • 使用 MySQL 数据库持久化存储                            │
│  • 使用 unordered_map 内存缓存 (O(1)查询效率)             │
│  • 使用 shared_mutex 实现读写锁（读共享，写互斥）         │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                  数据库管理层 (Database Management)           │
│               mysql_manager.hpp - MySQL 连接管理              │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ MySQLManager 类：数据库连接管理                       │   │
│  │  • Connect()       - 连接 MySQL 数据库                │   │
│  │  • GetConn()       - 获取数据库连接对象               │   │
│  └─────────────────────────────────────────────────────┘   │
│  • 封装 MySQL C++ Connector 操作                          │
│  • 提供简洁的数据库连接接口                                │
│  • 使用智能指针管理连接生命周期                            │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                    配置管理层 (Configuration)                 │
│                      config.hpp - 配置管理                    │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ Config 类 (单例模式)                                  │   │
│  │  • GetHotTime()        - 热点判断时间                 │   │
│  │  • GetServerPort()     - 服务器端口                   │   │
│  │  • GetServerIp()       - 服务器IP                     │   │
│  │  • GetDownloadPrefix() - 下载URL前缀                  │   │
│  │  • GetPackFileSuffix() - 压缩文件后缀                 │   │
│  │  • GetPackDir()        - 压缩包存放路径               │   │
│  │  • GetBackDir()        - 备份文件存放路径             │   │
│  │  • GetBackupFile()     - 备份信息持久化文件           │   │
│  └─────────────────────────────────────────────────────┘   │
│  • 从 cloud.json 读取配置                                  │
│  • 全局统一访问接口                                        │
└─────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────┐
│                      工具层 (Utility)                         │
│                      util.hpp - 基础工具类                    │
│  ┌──────────────────────┐      ┌──────────────────────┐    │
│  │   FileUtil           │      │   JsonUtil           │    │
│  │   文件操作工具        │      │   JSON序列化工具      │    │
│  ├──────────────────────┤      ├──────────────────────┤    │
│  │ • FileSize()         │      │ • Serialize()        │    │
│  │ • LastModifyTime()   │      │ • UnSerialize()      │    │
│  │ • LastAccessTime()   │      └──────────────────────┘    │
│  │ • FileName()         │                                   │
│  │ • SetContent()       │      第三方库                      │
│  │ • GetContent()       │      bundle.h/cpp (httplib)      │
│  │ • GetPosLen()        │      • LZIP 压缩算法             │
│  │ • Compress()         │      • HTTP 服务器框架           │
│  │ • UnCompress()       │                                   │
│  │ • Remove()           │                                   │
│  │ • Exists()           │                                   │
│  │ • CreateDirectory()  │                                   │
│  │ • GetDirectory()     │                                   │
│  └──────────────────────┘                                   │
└─────────────────────────────────────────────────────────────┘
```

## 核心模块详解

### 1. 工具层 (util.hpp)

**职责**：提供底层的文件操作和数据处理能力

- **FileUtil 类**：封装所有文件系统操作
  - 文件元数据获取（大小、时间戳）
  - 文件内容读写（支持指定位置和长度）
  - 文件压缩/解压（使用 LZIP 算法）
  - 目录操作（创建、遍历）
  
- **JsonUtil 类**：提供 JSON 序列化/反序列化
  - 使用 jsoncpp 库实现
  - 统一的数据持久化格式

**设计亮点**：
- 所有文件操作统一封装，便于错误处理和日志记录
- 支持断点续传所需的指定位置读取功能

### 2. 配置管理层 (config.hpp)

**职责**：统一管理系统配置参数

- **单例模式**：全局唯一的配置实例
- **懒加载**：首次访问时自动读取配置文件
- **集中管理**：所有配置项统一访问接口

**设计亮点**：
- 避免配置文件多次读取
- 便于后续扩展配置热更新功能

### 3. 数据库管理层 (mysql_manager.hpp)

**职责**：管理 MySQL 数据库连接

**MySQLManager 类**：简洁的数据库连接管理器

**核心方法**：
```cpp
class MySQLManager {
public:
    // 连接数据库
    bool Connect(const std::string& host, 
                 const std::string& user, 
                 const std::string& password, 
                 const std::string& database);
    
    // 获取数据库连接（返回智能指针引用）
    std::unique_ptr<sql::Connection>& GetConn();
};
```

**工作流程**：
```
1. 获取 MySQL 驱动实例
   ↓
2. 构建连接 URL (tcp://host:3306)
   ↓
3. 创建数据库连接
   ↓
4. 选择要使用的数据库
   ↓
5. 返回连接对象供 SQL 操作使用
```

**设计亮点**：
- 使用 `std::unique_ptr` 智能指针自动管理连接生命周期
- 简洁的接口设计，易于使用
- 异常捕获机制，连接失败时友好提示

### 4. 数据管理层 (data.hpp)

**职责**：管理所有文件的备份信息（MySQL 版本）

**核心数据结构**：
```cpp
struct BackupInfo {
    int _id;                  // 数据库主键ID
    std::string _real_path;   // 原始文件路径
    std::string _pack_path;   // 压缩包路径
    bool _pack_flag;          // 是否已压缩
    size_t _fsize;            // 文件大小
    time_t _mtime;            // 修改时间
    time_t _atime;            // 访问时间
    std::string _url_path;    // 下载URL
};
```

**存储架构（双层缓存）**：
```
┌─────────────────────────┐
│   内存缓存 (查询层)      │
│   unordered_map          │  ← 快速查询 O(1)
│   <url_path, BackupInfo> │
└────────────┬────────────┘
             │
             ↓ 读写同步
┌─────────────────────────┐
│   MySQL 数据库(持久化层) │
│   backup_info 表         │  ← 数据持久化
└─────────────────────────┘
```

**核心方法**：
- `InitLoad()` - 启动时从 MySQL 加载所有数据到内存
- `Insert()` - 插入数据（同时写入 MySQL 和内存缓存）
- `Update()` - 更新数据（同时更新 MySQL 和内存缓存）
- `GetOneByURL()` - 从内存缓存查询（高性能）
- `GetAll()` - 从内存获取所有数据

**数据操作流程**：
```cpp
// 插入数据
Insert() {
    1. 构造 SQL 插入语句
    2. 执行 SQL: INSERT INTO backup_info ...
    3. 更新内存缓存: _table[url_path] = info
}

// 查询数据（从内存，快速）
GetOneByURL() {
    return _table.find(url_path)  // O(1) 查询
}
```

**线程安全**：
- 使用 `shared_mutex` 实现读写锁
- 多个读操作可并发执行
- 写操作独占访问

**设计亮点**：
- **双层存储架构**：内存缓存 + MySQL 持久化
- **读写分离**：读从内存，写同步到数据库
- **高性能查询**：O(1) 内存查询，无需每次访问数据库
- **数据可靠性**：MySQL 保证数据不丢失
- **支持高并发**：MySQL 支持多进程/多服务器访问

### 4. 业务逻辑层

#### 4.1 热点文件管理 (hot.hpp)

**职责**：自动压缩长时间未访问的文件，节省存储空间

**工作流程**：
```
1. 定时扫描 backdir 目录
2. 遍历所有文件，获取最后访问时间
3. 判断：当前时间 - 最后访问时间 > 热点时间阈值
4. 压缩非热点文件到 packdir
5. 删除原始文件
6. 更新数据库中的压缩标志
7. 持久化备份信息
```

**热点判断算法**：
```cpp
bool HotJudge(const string &filename) {
    time_t last_atime = file.LastAccessTime();
    time_t cur_time = time(NULL);
    return (cur_time - last_atime > _hot_time);
}
```

**设计亮点**：
- 基于访问时间的智能判断，类似 LRU 缓存淘汰策略
- 独立线程运行，不影响 HTTP 服务性能

#### 4.2 HTTP 服务管理 (server.hpp)

**职责**：提供文件上传、下载、列表查看的 HTTP 接口

**核心接口**：

1. **文件上传** (`POST /upload`)
   ```
   - 接收 multipart/form-data 格式的文件
   - 保存到 backdir 目录
   - 创建备份信息并持久化
   ```

2. **文件列表** (`GET /listshow` 或 `GET /`)
   ```
   - 查询所有备份信息
   - 生成 HTML 页面展示
   - 显示文件名、修改时间、大小
   - 提供上传表单
   ```

3. **文件下载** (`GET /download/<filename>`)
   ```
   - 根据 URL 查询备份信息
   - 判断是否已压缩
   - 如果已压缩，先解压
   - 支持断点续传（ETag + Range）
   - 返回文件内容
   ```

**断点续传实现**：
```cpp
// 生成文件唯一标识
ETag = filename-filesize-mtime

// HTTP 请求头
If-Range: <ETag>
Range: bytes=start-end

// 服务器响应
// 正常下载: 200 OK
// 断点续传: 206 Partial Content
Accept-Ranges: bytes
ETag: <ETag>
Content-Range: bytes start-end/total
```

**设计亮点**：
- 自动解压机制：下载时检测压缩状态，透明处理
- ETag 机制：基于文件元数据，确保文件未变化才允许断点续传
- httplib 库内置支持 Range 请求，简化实现

### 5. 应用层 (cloud.cpp)

**职责**：系统启动入口，协调各个模块

**启动流程**：
```cpp
1. 创建全局 DataManager 实例
2. 自动加载历史备份数据 (InitLoad)
3. 启动热点管理线程 (HotManager::RunModule)
4. 启动 HTTP 服务线程 (Service::RunModule)
5. 等待线程结束 (join)
```

**多线程模型**：
- 主线程：初始化和线程管理
- 热点管理线程：独立运行，定时扫描和压缩
- HTTP 服务线程：监听端口，处理 HTTP 请求

**设计亮点**：
- 两个核心业务模块互不阻塞
- 共享 DataManager 实例，通过锁机制保证线程安全

## 关键设计模式

### 1. 单例模式 (Singleton)
- **应用**：Config 类
- **目的**：全局唯一配置实例，避免重复读取文件
- **实现**：静态局部变量（线程安全）

### 2. 工厂模式思想
- **应用**：BackupInfo::NewBackupInfo()
- **目的**：统一创建备份信息对象

### 3. 外观模式 (Facade)
- **应用**：FileUtil、JsonUtil
- **目的**：封装复杂的底层操作，提供简洁接口

### 4. 观察者模式思想
- **应用**：热点管理模块
- **目的**：持续监控文件状态变化

## 数据流转示意

### 上传流程
```
HTTP 请求 → POST /upload → Service::Upload()
    → FileUtil::SetContent() (保存文件)
    → BackupInfo::NewBackupInfo() (创建备份信息)
    → DataManager::Insert() (添加到数据库)
    → 写入 MySQL 数据库
```

### 下载流程
```
HTTP 请求 → GET /download/file.txt → Service::Download()
    → DataManager::GetOneByURL() (查询备份信息)
    → 判断 pack_flag
        如果已压缩:
            → FileUtil::UnCompress() (解压)
            → FileUtil::Remove() (删除压缩包)
            → DataManager::Update() (更新状态)
    → FileUtil::GetContent() (读取文件)
    → 返回文件内容
```

### 热点管理流程
```
HotManager::RunModule() (循环运行)
    → FileUtil::GetDirectory() (扫描 backdir)
    → 遍历每个文件
        → HotJudge() (判断是否热点)
        如果非热点:
            → DataManager::GetOneByRealPath() (查询备份信息)
            → FileUtil::Compress() (压缩文件)
            → FileUtil::Remove() (删除原文件)
            → DataManager::Update() (更新压缩标志)
    → sleep(1000ms)
    → 继续下一轮
```

## 技术难点与解决方案

### 1. 多线程数据竞争
**问题**：热点管理和 HTTP 服务同时访问 DataManager

**解决方案**：
- 使用 `shared_mutex` 读写锁
- 读操作（查询）：多线程并发
- 写操作（更新）：互斥访问

### 2. 文件状态一致性
**问题**：文件压缩过程中，HTTP 请求下载

**解决方案**：
- 下载时检测压缩状态
- 如已压缩，先解压再提供下载
- 更新压缩标志，避免重复处理

### 3. 断点续传可靠性
**问题**：如何保证断点续传的文件未被修改

**解决方案**：
- 使用 ETag (文件名-大小-修改时间) 作为唯一标识
- 请求发送 If-Range: <ETag>
- 服务器比对 ETag，不一致则重新下载

### 4. 数据持久化性能
**问题**：每次更新都写文件，IO开销大

**当前方案**：
- 每次数据变更立即持久化（安全优先）

**优化方向**：
- 批量写入：定时或累积一定数量后写入
- 写时复制：使用临时文件+原子替换

## 环境要求

### 服务器端（Linux）

#### 系统要求
- **操作系统**：Linux（Ubuntu 18.04 或更高版本推荐）
- **编译器**：GCC 7.0+ （支持 C++17 标准）
- **内核**：支持文件系统库的内核版本

#### 依赖库

1. **g++ 编译器**（支持 C++17）
```bash
sudo apt-get update
sudo apt-get install g++ build-essential
```

2. **MySQL 数据库服务器**（数据持久化）
```bash
# 安装 MySQL 服务器
sudo apt-get install mysql-server

# 启动 MySQL 服务
sudo systemctl start mysql
sudo systemctl enable mysql
```

3. **MySQL C++ Connector**（MySQL C++ 连接库）
```bash
sudo apt-get install libmysqlcppconn-dev libmysqlclient-dev
```

4. **pthread 线程库**（通常已自带）
```bash
# 一般系统自带，如需安装：
sudo apt-get install libpthread-stubs0-dev
```

5. **文件系统库**（C++17 标准库的一部分）
```bash
# GCC 8+ 通常已包含，如需升级：
sudo apt-get install gcc-8 g++-8
```

## 部署指南

### 1. 配置 MySQL 数据库

#### 1.1 登录 MySQL 并设置密码

```bash
# 使用 sudo 登录 MySQL（无需密码）
sudo mysql
```

在 MySQL 命令行中执行：

```sql
-- 设置 root 用户密码
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '123456';
FLUSH PRIVILEGES;
EXIT;
```

#### 1.2 创建数据库和表

```bash
# 方法1：使用 SQL 脚本（推荐）
mysql -u root -p123456 < create_tables.sql

# 方法2：手动创建
mysql -u root -p123456
```

如果手动创建，执行以下 SQL：

```sql
-- 创建数据库
CREATE DATABASE IF NOT EXISTS cloud_backup 
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE cloud_backup;

-- 创建备份文件信息表
CREATE TABLE IF NOT EXISTS backup_info (
    id INT AUTO_INCREMENT PRIMARY KEY,
    url_path VARCHAR(512) NOT NULL UNIQUE,
    real_path VARCHAR(512) NOT NULL,
    pack_path VARCHAR(512) DEFAULT NULL,
    pack_flag TINYINT(1) DEFAULT 0,
    fsize BIGINT UNSIGNED DEFAULT 0,
    mtime BIGINT DEFAULT 0,
    atime BIGINT DEFAULT 0,
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_real_path (real_path(255)),
    INDEX idx_mtime (mtime)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

EXIT;
```

#### 1.3 验证数据库

```bash
# 查看数据库
mysql -u root -p123456 -e "SHOW DATABASES;"

# 查看表结构
mysql -u root -p123456 cloud_backup -e "DESC backup_info;"
```

### 2. 编译程序

进入服务器目录并编译：

```bash
cd server/
make
```

编译成功后会生成可执行文件 `cloud`

### 3. 配置数据库连接信息

编辑 `data.hpp` 文件，修改数据库连接参数（第 57-60 行）：

```cpp
std::string db_host = "127.0.0.1";
std::string db_user = "root";           // 修改为您的数据库用户名
std::string db_password = "123456";     // 修改为您的数据库密码
std::string db_name = "cloud_backup";
```

**注意**：修改后需要重新编译：`make clean && make`

### 4. 配置文件

编辑 `cloud.json` 配置文件：

```json
{
    "hot_time": 30,                    // 热点判断时间（秒）
    "server_port": 8080,               // 服务器监听端口
    "server_ip": "0.0.0.0",           // 服务器IP（0.0.0.0表示监听所有接口）
    "download_prefix": "/download/",   // 下载URL前缀
    "packfile_suffix": ".lzip",        // 压缩文件后缀
    "pack_dir": "./packdir/",          // 压缩文件存放目录
    "back_dir": "./backdir/",          // 备份文件存放目录
    "backup_file": "./cloud.dat"       // 备份信息持久化文件
}
```

### 5. 创建必要目录

```bash
mkdir -p backdir   # 备份文件目录
mkdir -p packdir   # 压缩文件目录
```

### 6. 运行服务器

```bash
./cloud
```

服务器启动后会显示：
```
✓ MySQL 连接成功：cloud_backup
加载了 0 条备份记录
```

服务器将：
- 连接到 MySQL 数据库
- 加载历史备份数据到内存
- 在指定端口（默认 8080）启动 HTTP 服务
- 自动监控 `backdir` 目录中的文件
- 对超过热点时间未访问的文件进行压缩

### 7. 验证服务

检查服务是否正常运行：

```bash
# 检查端口监听
netstat -tulnp | grep 8080

# 使用 curl 测试
curl http://localhost:8080/

# 查看数据库数据
mysql -u root -p123456 cloud_backup -e "SELECT COUNT(*) FROM backup_info;"
```

### 8. 后台运行（可选）

使用 nohup 或 systemd 让程序在后台运行：

```bash
# 使用 nohup
nohup ./cloud > cloud.log 2>&1 &

# 查看日志
tail -f cloud.log
```

### 9. 停止服务

```bash
# 查找进程
ps aux | grep cloud

# 停止进程
kill <PID>
```

## API 接口

### 上传文件
```
POST /upload
Content-Type: multipart/form-data
```

### 下载文件
```
GET /download/<filename>
```

## 清理编译文件

```bash
cd server/
make clean
```

## 常见问题

### 1. MySQL 连接失败：Access denied

**错误信息**：
```
Access denied for user 'root'@'localhost'
```

**解决方案**：
```bash
# 重置 root 密码
sudo mysql
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '123456';
FLUSH PRIVILEGES;
EXIT;
```

### 2. MySQL 连接失败：数据库不存在

**错误信息**：
```
Unknown database 'cloud_backup'
```

**解决方案**：
```bash
# 创建数据库
mysql -u root -p123456 < create_tables.sql
```

### 3. 编译错误：找不到 MySQL 头文件

**错误信息**：
```
fatal error: mysql_driver.h: No such file or directory
```

**解决方案**：
```bash
sudo apt-get install libmysqlcppconn-dev
```

### 4. 运行时错误：找不到共享库

**错误信息**：
```
error while loading shared libraries: libmysqlcppconn.so.7
```

**解决方案**：
```bash
sudo ldconfig
```

### 5. 清空数据库数据

```bash
# 清空表数据（保留表结构）
mysql -u root -p123456 -e "USE cloud_backup; TRUNCATE TABLE backup_info;"

# 删除并重建数据库
mysql -u root -p123456 -e "DROP DATABASE cloud_backup;"
mysql -u root -p123456 < create_tables.sql
```

### 6. 编译错误：C++17 特性不支持

确保 GCC 版本 ≥ 7.0，或更新编译器

### 7. 端口被占用

修改 `cloud.json` 中的 `server_port` 配置

### 8. 权限不足

确保对 `backdir` 和 `packdir` 目录有读写权限

## 开发者

如需参与开发或提交问题，请：

1. Fork 本仓库
2. 创建特性分支
3. 提交代码
4. 发起 Pull Request

## 许可证

详见 LICENSE 文件