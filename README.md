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

## 完整场景流程详解

服务器端共有 **6 个核心场景**，以下是每个场景从开始到结束的完整代码执行流程。

### 场景 1：服务器启动场景

**入口**：`cloud.cpp` 的 `main()` 函数（234-241行）

**完整执行流程**：

```
main() 函数启动
  │
  ├─→ [步骤 1] 创建全局 DataManager 对象
  │      data = new myspace::DataManager();
  │      
  │      触发 DataManager 构造函数 (data.hpp: 50-68行)：
  │        ├─→ 初始化数据库连接信息
  │        │     db_host = "127.0.0.1"
  │        │     db_user = "root"
  │        │     db_password = "123456"
  │        │     db_name = "cloud_backup"
  │        │
  │        ├─→ 创建 MySQLManager 对象
  │        │     _mysql = new MySQLManager();
  │        │
  │        ├─→ 连接 MySQL 数据库
  │        │     _mysql->Connect(db_host, db_user, db_password, db_name)
  │        │     
  │        │     Connect 流程 (mysql_manager.hpp: 19-39行)：
  │        │       ├─→ 获取 MySQL 驱动实例
  │        │       │     driver = sql::mysql::get_mysql_driver_instance();
  │        │       │
  │        │       ├─→ 构建连接 URL
  │        │       │     url = "tcp://127.0.0.1:3306"
  │        │       │
  │        │       ├─→ 建立数据库连接
  │        │       │     _conn.reset(driver->connect(url, user, password));
  │        │       │
  │        │       └─→ 选择数据库
  │        │             _conn->setSchema("cloud_backup");
  │        │
  │        └─→ 加载历史数据到内存
  │              InitLoad() (data.hpp: 76-113行)：
  │                ├─→ 加写锁（独占访问）
  │                │     std::unique_lock<std::shared_mutex> lock(_rwlock);
  │                │
  │                ├─→ 获取数据库连接
  │                │     auto& conn = _mysql->GetConn();
  │                │
  │                ├─→ 创建 SQL 语句执行器
  │                │     std::unique_ptr<sql::Statement> stmt(conn->createStatement());
  │                │
  │                ├─→ 执行查询语句
  │                │     std::unique_ptr<sql::ResultSet> res(
  │                │         stmt->executeQuery("SELECT * FROM backup_info")
  │                │     );
  │                │
  │                ├─→ 遍历结果集
  │                │     while (res->next()) {
  │                │         BackupInfo info;
  │                │         info._id = res->getInt("id");
  │                │         info._url_path = res->getString("url_path");
  │                │         info._real_path = res->getString("real_path");
  │                │         info._pack_path = res->getString("pack_path");
  │                │         info._pack_flag = res->getBoolean("pack_flag");
  │                │         info._fsize = res->getUInt64("fsize");
  │                │         info._mtime = res->getInt64("mtime");
  │                │         info._atime = res->getInt64("atime");
  │                │         
  │                │         // 存入内存哈希表
  │                │         _table[info._url_path] = info;
  │                │         count++;
  │                │     }
  │                │
  │                ├─→ 输出加载信息
  │                │     std::cout << "加载了 " << count << " 条备份记录" << std::endl;
  │                │
  │                └─→ 释放锁，返回成功
  │
  ├─→ [步骤 2] 创建热点管理线程
  │      std::thread thread_hot_manager(HotTest);
  │      
  │      HotTest() 函数 (cloud.cpp: 223-227行)：
  │        └─→ 创建 HotManager 对象
  │              myspace::HotManager hot;
  │              
  │              HotManager 构造函数 (hot.hpp: 13-25行)：
  │                ├─→ 从 Config 读取配置
  │                │     _back_dir = "./backdir/"
  │                │     _pack_dir = "./packdir/"
  │                │     _pack_suffix = ".lz"
  │                │     _hot_time = 30
  │                │
  │                └─→ 创建必要目录
  │                      FileUtil tmp1(_back_dir);
  │                      FileUtil tmp2(_pack_dir);
  │                      tmp1.CreateDirectory();
  │                      tmp2.CreateDirectory();
  │              
  │              hot.RunModule();  // 进入热点管理循环（详见场景6）
  │
  ├─→ [步骤 3] 创建 HTTP 服务线程
  │      std::thread thread_service(ServiceTest);
  │      
  │      ServiceTest() 函数 (cloud.cpp: 229-233行)：
  │        └─→ 创建 Service 对象
  │              myspace::Service srv;
  │              
  │              Service 构造函数 (server.hpp: 178-184行)：
  │                └─→ 从 Config 读取配置
  │                      _server_port = 9000
  │                      _server_ip = "0.0.0.0"
  │                      _download_prefix = "/download/"
  │              
  │              srv.RunModule() (server.hpp: 186-199行)：
  │                ├─→ 注册路由映射
  │                │     _server.Post("/upload", Upload);
  │                │     _server.Get("/listshow", ListShow);
  │                │     _server.Get("/", ListShow);
  │                │     _server.Get("/download/(.*)", Download);
  │                │
  │                └─→ 启动 HTTP 服务监听
  │                      _server.listen("0.0.0.0", 9000);
  │                      // 服务器开始等待客户端连接
  │
  └─→ [步骤 4] 主线程等待
        thread_hot_manager.join();  // 等待热点管理线程
        thread_service.join();      // 等待 HTTP 服务线程
        // 程序持续运行，直到手动终止
```

**启动完成后的状态**：
- MySQL 数据库连接已建立
- 历史数据已加载到内存（unordered_map）
- 热点管理线程正在后台运行（每1ms扫描一次）
- HTTP 服务在 9000 端口监听请求

---

### 场景 2：文件上传场景

**触发条件**：HTTP POST 请求到 `/upload`

**请求示例**：
```http
POST /upload HTTP/1.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary
Content-Length: 1234

------WebKitFormBoundary
Content-Disposition: form-data; name="file"; filename="test.txt"
Content-Type: text/plain

[文件内容...]
------WebKitFormBoundary--
```

**完整执行流程**：

```
HTTP POST /upload 请求到达
  │
  └─→ httplib 路由匹配，调用 Service::Upload() (server.hpp: 203-222行)
        │
        ├─→ [步骤 1] 检查是否有上传文件
        │      std::cout << "uploading ..." << std::endl;
        │      auto ret = req.has_file("file");
        │      
        │      if (ret == false) {
        │          rsp.status = 400;  // Bad Request
        │          return;  // 终止处理
        │      }
        │      
        │      说明：has_file("file") 检查 multipart/form-data 中
        │           是否有 name="file" 的表单字段
        │
        ├─→ [步骤 2] 获取上传的文件对象
        │      const auto &file = req.get_file_value("file");
        │      
        │      file 对象包含：
        │        - file.filename: "test.txt"（文件名）
        │        - file.content:  "Hello World..."（文件内容）
        │
        ├─→ [步骤 3] 构造文件存储路径
        │      std::string back_dir = Config::GetInstance().GetBackDir();
        │      // back_dir = "./backdir/"
        │      
        │      std::string realpath = back_dir + FileUtil(file.filename).FileName();
        │      // realpath = "./backdir/test.txt"
        │      
        │      说明：FileUtil(file.filename).FileName() 提取纯文件名
        │           （去除可能的路径部分）
        │
        ├─→ [步骤 4] 将文件内容写入磁盘
        │      FileUtil fu(realpath);
        │      fu.SetContent(file.content);
        │      
        │      SetContent 流程 (util.hpp)：
        │        ├─→ 打开文件输出流
        │        │     std::ofstream ofs(realpath, std::ios::binary);
        │        │
        │        ├─→ 写入内容
        │        │     ofs.write(file.content.c_str(), file.content.size());
        │        │
        │        └─→ 关闭文件
        │              ofs.close();
        │      
        │      结果：./backdir/test.txt 文件创建成功
        │
        ├─→ [步骤 5] 创建备份信息
        │      BackupInfo info;
        │      info.NewBackupInfo(realpath);
        │      
        │      NewBackupInfo 流程 (data.hpp: 26-43行)：
        │        ├─→ 检查文件是否存在
        │        │     FileUtil file(realpath);
        │        │     if (!file.Exists()) return false;
        │        │
        │        ├─→ 获取配置信息
        │        │     Config config = Config::GetInstance();
        │        │
        │        ├─→ 填充文件基本信息
        │        │     _real_path = realpath;  // "./backdir/test.txt"
        │        │     _fsize = file.FileSize();  // 1024 字节
        │        │     _mtime = file.LastModifyTime();  // 1729087200
        │        │     _atime = file.LastAcccessTime();  // 1729087200
        │        │
        │        ├─→ 设置压缩相关信息
        │        │     _pack_flag = false;  // 初始未压缩
        │        │     _pack_path = "./packdir/test.txt.lz"
        │        │
        │        └─→ 构造下载 URL
        │              _url_path = "/download/test.txt"
        │
        └─→ [步骤 6] 插入到数据管理模块
              data->Insert(info);
              
              Insert 流程 (data.hpp: 116-158行)：
                ├─→ 加写锁（独占访问）
                │     std::unique_lock<std::shared_mutex> lock(_rwlock);
                │
                ├─→ 获取数据库连接
                │     auto& conn = _mysql->GetConn();
                │     if (!conn) return false;
                │
                ├─→ 构造 SQL 插入语句
                │     std::stringstream sql;
                │     sql << "INSERT INTO backup_info "
                │         << "(url_path, real_path, pack_path, pack_flag, fsize, mtime, atime) "
                │         << "VALUES ('"
                │         << "/download/test.txt" << "', '"
                │         << "./backdir/test.txt" << "', '"
                │         << "./packdir/test.txt.lz" << "', "
                │         << "0, 1024, 1729087200, 1729087200) "
                │         << "ON DUPLICATE KEY UPDATE "
                │         << "real_path='./backdir/test.txt', "
                │         << "pack_path='./packdir/test.txt.lz', "
                │         << "pack_flag=0, fsize=1024, mtime=1729087200, atime=1729087200";
                │     
                │     说明：ON DUPLICATE KEY UPDATE 实现"插入或更新"语义
                │          如果 url_path 已存在，则更新；否则插入新记录
                │
                ├─→ 执行 SQL 语句
                │     std::unique_ptr<sql::Statement> stmt(conn->createStatement());
                │     stmt->executeUpdate(sql.str());
                │     
                │     数据库变化：backup_info 表新增一条记录
                │
                ├─→ 更新内存缓存
                │     _table[info._url_path] = info;
                │     // _table["/download/test.txt"] = info;
                │
                ├─→ 输出成功信息
                │     std::cout << "插入成功: " << info._url_path << std::endl;
                │
                └─→ 释放锁，返回成功
                      return true;
```

**执行结果**：
- 文件系统：`./backdir/test.txt` 文件创建
- MySQL 数据库：`backup_info` 表插入一条记录
- 内存缓存：`_table` 哈希表新增一个键值对
- HTTP 响应：200 OK（httplib 自动设置）

---

### 场景 3：文件列表展示场景

**触发条件**：HTTP GET 请求到 `/listshow` 或 `/`

**完整执行流程**：

```
HTTP GET /listshow 请求到达
  │
  └─→ httplib 路由匹配，调用 Service::ListShow() (server.hpp: 231-260行)
        │
        ├─→ [步骤 1] 获取所有备份文件信息
        │      std::vector<BackupInfo> arry;
        │      data->GetAll(&arry);
        │      
        │      GetAll 流程 (data.hpp: 226-233行)：
        │        ├─→ 加读锁（共享访问）
        │        │     std::shared_lock<std::shared_mutex> lock(_rwlock);
        │        │
        │        ├─→ 遍历内存哈希表
        │        │     for (const auto& pair : _table) {
        │        │         arry->push_back(pair.second);
        │        │     }
        │        │     
        │        │     例如：arry 中包含
        │        │       - BackupInfo { _url_path="/download/test.txt", ... }
        │        │       - BackupInfo { _url_path="/download/image.jpg", ... }
        │        │       - BackupInfo { _url_path="/download/doc.pdf", ... }
        │        │
        │        └─→ 释放锁，返回成功
        │
        ├─→ [步骤 2] 开始构造 HTML 页面
        │      std::stringstream ss;
        │      
        │      ├─→ 添加文件上传表单
        │      │     ss << "<html><body>";
        │      │     ss << "<form action='/upload' method='post' "
        │      │        << "enctype='multipart/form-data'>";
        │      │     ss << " <input type='file' name='file'>";
        │      │     ss << "<input type='submit' value='upload'>";
        │      │     ss << " </form></body></html>";
        │      │     
        │      │     生成的表单：
        │      │       [选择文件] [upload按钮]
        │      │
        │      └─→ 添加下载列表页头
        │            ss << "<html><head><title>Download</title></head>";
        │            ss << "<body><h1>Download</h1><table>";
        │
        ├─→ [步骤 3] 遍历所有备份信息，生成表格行
        │      for (auto &a : arry) {
        │          // 开始一行
        │          ss << "<tr>";
        │          
        │          // 提取文件名
        │          std::string filename = FileUtil(a._real_path).FileName();
        │          // filename = "test.txt"
        │          
        │          // 生成下载链接（第一列）
        │          ss << "<td><a href='" << a._url_path << "'>" 
        │             << filename << "</a></td>";
        │          // 生成：<td><a href='/download/test.txt'>test.txt</a></td>
        │          
        │          // 显示修改时间（第二列）
        │          ss << "<td align='right'>" << TimetoStr(a._mtime) << "</td>";
        │          
        │          TimetoStr 流程 (server.hpp: 224-228行)：
        │            std::string tmp = std::ctime(&t);
        │            // tmp = "Wed Oct 17 12:00:00 2024\n"
        │            return tmp;
        │          
        │          // 显示文件大小（第三列，转换为KB）
        │          ss << "<td align='right'>" << a._fsize / 1024 << "k</td>";
        │          
        │          // 结束一行
        │          ss << "</tr>";
        │      }
        │      
        │      生成的表格行示例：
        │        <tr>
        │          <td><a href='/download/test.txt'>test.txt</a></td>
        │          <td align='right'>Wed Oct 17 12:00:00 2024</td>
        │          <td align='right'>1k</td>
        │        </tr>
        │
        ├─→ [步骤 4] 结束 HTML 页面
        │      ss << "</table></body></html>";
        │
        └─→ [步骤 5] 设置响应
              rsp.body = ss.str();  // 设置响应正文为生成的HTML
              rsp.set_header("Content-Type", "text/html");  // 内容类型
              rsp.status = 200;  // HTTP 状态码
              return;
```

**HTTP 响应示例**：
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: 512

<html><body>
<form action='/upload' method='post' enctype='multipart/form-data'>
  <input type='file' name='file'>
  <input type='submit' value='upload'>
</form>
<h1>Download</h1>
<table>
  <tr>
    <td><a href='/download/test.txt'>test.txt</a></td>
    <td align='right'>Wed Oct 17 12:00:00 2024</td>
    <td align='right'>1k</td>
  </tr>
  <tr>
    <td><a href='/download/image.jpg'>image.jpg</a></td>
    <td align='right'>Wed Oct 17 13:30:00 2024</td>
    <td align='right'>500k</td>
  </tr>
</table>
</body></html>
```

**浏览器显示效果**：
```
┌────────────────────────────────────────┐
│  [选择文件] [upload]                    │
├────────────────────────────────────────┤
│  Download                               │
├────────────────────────────────────────┤
│  test.txt         Wed Oct 17 12:00  1k │
│  image.jpg        Wed Oct 17 13:30  500k│
└────────────────────────────────────────┘
```

---

### 场景 4：文件下载场景（正常下载）

**触发条件**：HTTP GET 请求到 `/download/test.txt`

**完整执行流程**：

```
HTTP GET /download/test.txt 请求到达
  │
  └─→ httplib 路由匹配，调用 Service::Download() (server.hpp: 274-341行)
        │
        ├─→ [步骤 1] 获取请求的资源路径
        │      std::string url = req.path;
        │      // url = "/download/test.txt"
        │
        ├─→ [步骤 2] 根据 URL 查询备份信息
        │      BackupInfo info;
        │      data->GetOneByURL(req.path, info);
        │      
        │      GetOneByURL 流程 (data.hpp: 201-210行)：
        │        ├─→ 加读锁（共享访问）
        │        │     std::shared_lock<std::shared_mutex> lock(_rwlock);
        │        │
        │        ├─→ 在哈希表中查找
        │        │     auto it = _table.find("/download/test.txt");
        │        │     if (it == _table.end()) {
        │        │         return false;  // 文件不存在
        │        │     }
        │        │
        │        ├─→ 返回备份信息
        │        │     info = it->second;
        │        │     // info 现在包含：
        │        │     //   _real_path = "./backdir/test.txt"
        │        │     //   _pack_flag = false (未压缩)
        │        │     //   _fsize = 1024
        │        │     //   ...
        │        │
        │        └─→ 释放锁，返回成功
        │
        ├─→ [步骤 3] 检查文件是否被压缩
        │      if (info._pack_flag == true) {
        │          // 文件已被压缩，需要先解压
        │          
        │          FileUtil fu(info._pack_path);
        │          // fu 指向压缩包：./packdir/test.txt.lz
        │          
        │          fu.UnCompress(info._real_path);
        │          // 解压到：./backdir/test.txt
        │          
        │          UnCompress 流程 (util.hpp)：
        │            ├─→ 打开压缩包文件
        │            ├─→ 使用 zlib 解压数据
        │            ├─→ 将解压数据写入目标文件
        │            └─→ 关闭文件
        │          
        │          fu.Remove();
        │          // 删除压缩包：./packdir/test.txt.lz
        │          
        │          info._pack_flag = false;
        │          data->Update(info);
        │          // 更新数据库和内存缓存中的压缩标志
        │      }
        │      
        │      此场景假设：_pack_flag = false（未压缩），跳过此步
        │
        ├─→ [步骤 4] 判断是否断点续传
        │      bool retrans = false;
        │      std::string old_etag;
        │      
        │      if (req.has_header("If-Range")) {
        │          old_etag = req.get_header_value("If-Range");
        │          if (old_etag == GetETag(info)) {
        │              retrans = true;
        │          }
        │      }
        │      
        │      GetETag 流程 (server.hpp: 262-272行)：
        │        FileUtil fu(info._real_path);
        │        std::string etag = fu.FileName();  // "test.txt"
        │        etag += "-";
        │        etag += std::to_string(info._fsize);  // "1024"
        │        etag += "-";
        │        etag += std::to_string(info._mtime);  // "1729087200"
        │        return etag;  // "test.txt-1024-1729087200"
        │      
        │      此场景假设：没有 If-Range 头，retrans = false（正常下载）
        │
        ├─→ [步骤 5] 读取文件内容
        │      FileUtil fu(info._real_path);
        │      
        │      if (retrans == false) {
        │          // 正常下载分支
        │          
        │          fu.GetContent(&rsp.body);
        │          
        │          GetContent 流程 (util.hpp)：
        │            ├─→ 打开文件
        │            │     std::ifstream ifs(path, std::ios::binary);
        │            │
        │            ├─→ 获取文件大小
        │            │     ifs.seekg(0, std::ios::end);
        │            │     size_t fsize = ifs.tellg();
        │            │     ifs.seekg(0, std::ios::beg);
        │            │
        │            ├─→ 读取全部内容
        │            │     content->resize(fsize);
        │            │     ifs.read(&(*content)[0], fsize);
        │            │
        │            └─→ 关闭文件
        │                  ifs.close();
        │          
        │          结果：rsp.body 现在包含文件的全部内容
        │      }
        │
        └─→ [步骤 6] 设置响应头和状态码
              if (retrans == false) {
                  // 正常下载响应
                  
                  rsp.set_header("Accept-Ranges", "bytes");
                  // 告诉客户端支持按字节范围请求
                  
                  rsp.set_header("ETag", GetETag(info));
                  // ETag: "test.txt-1024-1729087200"
                  // 用于断点续传验证
                  
                  rsp.set_header("Content-Type", "application/octet-stream");
                  // 指定响应内容为二进制文件流
                  
                  rsp.status = 200;
                  // HTTP 200 OK
              }
```

**HTTP 响应示例**：
```http
HTTP/1.1 200 OK
Accept-Ranges: bytes
ETag: "test.txt-1024-1729087200"
Content-Type: application/octet-stream
Content-Length: 1024

[文件的全部二进制内容...]
```

**执行结果**：
- 客户端收到完整文件内容
- 如果文件被压缩，自动解压后返回
- 响应头包含 ETag，支持后续断点续传

---

### 场景 5：文件断点续传下载场景

**触发条件**：HTTP GET 请求 + `If-Range` 和 `Range` 请求头

**请求示例**：
```http
GET /download/bigfile.zip HTTP/1.1
If-Range: "bigfile.zip-50000001-1729087200"
Range: bytes=1000000-5000000
```

**完整执行流程**：

```
HTTP GET /download/bigfile.zip 请求到达（带断点续传头）
  │
  └─→ httplib 路由匹配，调用 Service::Download() (server.hpp: 274-341行)
        │
        ├─→ [步骤 1-3] 同场景 4
        │      获取 URL → 查询备份信息 → 检查是否压缩
        │
        ├─→ [步骤 4] 判断是否断点续传 ⭐ 关键步骤
        │      bool retrans = false;
        │      std::string old_etag;
        │      
        │      // 检查是否有 If-Range 请求头
        │      if (req.has_header("If-Range")) {
        │          // 有 If-Range 头，表示客户端请求断点续传
        │          
        │          old_etag = req.get_header_value("If-Range");
        │          // old_etag = "bigfile.zip-50000001-1729087200"
        │          
        │          // 计算当前文件的 ETag
        │          std::string new_etag = GetETag(info);
        │          // new_etag = "bigfile.zip-50000001-1729087200"
        │          
        │          // 比对 ETag
        │          if (old_etag == new_etag) {
        │              // ETag 相同 → 文件未被修改 → 允许断点续传
        │              retrans = true;  ✅
        │          } else {
        │              // ETag 不同 → 文件已被修改 → 拒绝断点续传
        │              retrans = false;  ❌
        │              // 将按正常下载处理（返回全部数据）
        │          }
        │      }
        │      
        │      断点续传条件：
        │        ✅ 必须有 If-Range 请求头
        │        ✅ If-Range 的值必须与当前文件的 ETag 相同
        │      
        │      此场景：retrans = true
        │
        ├─→ [步骤 5] 读取文件内容
        │      FileUtil fu(info._real_path);
        │      
        │      if (retrans == true) {
        │          // 断点续传分支
        │          
        │          fu.GetContent(&rsp.body);
        │          
        │          注意：虽然读取了全部内容到 rsp.body，
        │               但 httplib 库会根据 Range 请求头
        │               自动截取 body 中的指定区间返回
        │          
        │          Range 头解析（httplib 内部自动处理）：
        │            Range: bytes=1000000-5000000
        │            ↓
        │            从 rsp.body 中提取字节 [1000000, 5000000]
        │            ↓
        │            实际返回 4000001 字节
        │      }
        │
        └─→ [步骤 6] 设置断点续传响应头
              if (retrans == true) {
                  // 断点续传响应
                  
                  rsp.set_header("Accept-Ranges", "bytes");
                  // 支持按字节范围请求
                  
                  rsp.set_header("ETag", GetETag(info));
                  // ETag: "bigfile.zip-50000001-1729087200"
                  
                  rsp.set_header("Content-Type", "application/octet-stream");
                  // 二进制文件流
                  
                  rsp.status = 206;  ⭐ 206 Partial Content
                  // 状态码 206 表示"部分内容"，用于断点续传
                  
                  // httplib 自动添加 Content-Range 头：
                  // Content-Range: bytes 1000000-5000000/50000001
                  //                     （起始-结束/总大小）
              }
```

**HTTP 响应示例**：
```http
HTTP/1.1 206 Partial Content
Accept-Ranges: bytes
ETag: "bigfile.zip-50000001-1729087200"
Content-Type: application/octet-stream
Content-Range: bytes 1000000-5000000/50000001
Content-Length: 4000001

[部分文件数据：从字节 1000000 到 5000000]
```

**断点续传工作原理**：

1. **首次下载**（下载到一半中断）：
   ```
   客户端发送：GET /download/bigfile.zip
   服务器返回：200 OK + ETag: "bigfile.zip-50000001-1729087200"
   客户端下载到 1000000 字节时网络中断
   客户端记录：已下载 1000000 字节，ETag="bigfile.zip-50000001-1729087200"
   ```

2. **续传请求**：
   ```
   客户端发送：GET /download/bigfile.zip
              If-Range: "bigfile.zip-50000001-1729087200"
              Range: bytes=1000000-
   服务器验证：ETag 相同 → 文件未修改 → 允许续传
   服务器返回：206 Partial Content + 剩余数据
   ```

3. **ETag 不匹配情况**：
   ```
   如果文件在下载期间被修改：
     - 文件大小或修改时间改变
     - ETag 值随之改变
     - 服务器检测到 ETag 不匹配
     - 返回 200 OK + 全部数据（重新下载）
   ```

---

### 场景 6：热点文件管理场景（后台压缩）

**触发条件**：热点管理线程持续运行（每1ms扫描一次）

**完整执行流程**：

```
HotManager::RunModule() 进入无限循环 (hot.hpp: 27-59行)
  │
  └─→ while (1) {  // 无限循环，持续监控
        │
        ├─→ [步骤 1] 扫描备份目录
        │      FileUtil file(_back_dir);
        │      // file 指向 "./backdir/"
        │      
        │      std::vector<std::string> arr;
        │      file.GetDirectory(arr);
        │      
        │      GetDirectory 流程 (util.hpp)：
        │        ├─→ 使用 C++17 filesystem 库遍历目录
        │        │     namespace fs = std::filesystem;
        │        │     for (auto& entry : fs::directory_iterator(path)) {
        │        │         if (entry.is_regular_file()) {
        │        │             arr->push_back(entry.path().string());
        │        │         }
        │        │     }
        │        │
        │        └─→ 返回文件列表
        │      
        │      结果：arr 包含所有文件路径
        │        arr = [
        │          "./backdir/test.txt",
        │          "./backdir/image.jpg",
        │          "./backdir/document.pdf"
        │        ]
        │
        ├─→ [步骤 2] 遍历所有文件
        │      for (auto &a : arr) {
        │        │
        │        ├─→ [步骤 2.1] 热点判断
        │        │      if (!HotJudge(a)) {
        │        │          continue;  // 热点文件跳过
        │        │      }
        │        │      
        │        │      HotJudge 流程 (hot.hpp: 61-72行)：
        │        │        ├─→ 获取文件最后访问时间
        │        │        │     FileUtil file(filename);
        │        │        │     time_t last_atime = file.LastAcccessTime();
        │        │        │     // last_atime = 1729087200 (文件最后被访问的时间戳)
        │        │        │
        │        │        ├─→ 获取当前系统时间
        │        │        │     time_t cur_time = time(NULL);
        │        │        │     // cur_time = 1729087250 (当前时间戳)
        │        │        │
        │        │        └─→ 计算时间差并判断
        │        │              int diff = cur_time - last_atime;
        │        │              // diff = 50 秒
        │        │              
        │        │              if (diff > _hot_time) {
        │        │                  // 50 > 30（配置的热点时间阈值）
        │        │                  return true;   // 非热点，需要压缩
        │        │              }
        │        │              return false;    // 热点，不压缩
        │        │      
        │        │      热点判断逻辑：
        │        │        - 如果文件最近 30 秒内被访问过 → 热点文件 → 不压缩
        │        │        - 如果文件超过 30 秒未被访问 → 非热点 → 压缩
        │        │
        │        ├─→ [步骤 2.2] 获取文件备份信息
        │        │      BackupInfo bi;
        │        │      if (!data->GetOneByRealPath(a, bi)) {
        │        │          // 文件存在但没有备份信息
        │        │          // （可能是手动放入 backdir 的文件）
        │        │          bi.NewBackupInfo(a);
        │        │      }
        │        │      
        │        │      GetOneByRealPath 流程 (data.hpp: 213-223行)：
        │        │        ├─→ 加读锁
        │        │        │     std::shared_lock<std::shared_mutex> lock(_rwlock);
        │        │        │
        │        │        ├─→ 遍历哈希表查找
        │        │        │     for (const auto& pair : _table) {
        │        │        │         if (pair.second._real_path == realpath) {
        │        │        │             info = pair.second;
        │        │        │             return true;
        │        │        │         }
        │        │        │     }
        │        │        │
        │        │        └─→ 释放锁
        │        │      
        │        │      结果：bi 包含文件的备份信息
        │        │        bi._real_path = "./backdir/test.txt"
        │        │        bi._pack_path = "./packdir/test.txt.lz"
        │        │        bi._pack_flag = false
        │        │
        │        ├─→ [步骤 2.3] 压缩文件
        │        │      FileUtil tmp(a);
        │        │      // tmp 指向原始文件：./backdir/test.txt
        │        │      
        │        │      tmp.Compress(bi._pack_path);
        │        │      // 压缩到：./packdir/test.txt.lz
        │        │      
        │        │      Compress 流程 (util.hpp)：
        │        │        ├─→ 读取原始文件内容
        │        │        │     std::ifstream ifs(realpath, std::ios::binary);
        │        │        │     std::string body;
        │        │        │     // 读取全部内容到 body
        │        │        │
        │        │        ├─→ 使用 zlib 压缩
        │        │        │     std::string compressed = bundle::lz::compress(body);
        │        │        │     // LZIP 压缩算法
        │        │        │
        │        │        └─→ 写入压缩包
        │        │              std::ofstream ofs(packpath, std::ios::binary);
        │        │              ofs.write(compressed.c_str(), compressed.size());
        │        │              ofs.close();
        │        │      
        │        │      压缩效果示例：
        │        │        原始文件：./backdir/test.txt (1024 字节)
        │        │          ↓ 压缩
        │        │        压缩包：./packdir/test.txt.lz (约 300 字节)
        │        │        节省：约 70% 空间
        │        │
        │        ├─→ [步骤 2.4] 删除原始文件
        │        │      tmp.Remove();
        │        │      
        │        │      Remove 流程 (util.hpp)：
        │        │        std::filesystem::remove(path);
        │        │      
        │        │      文件系统变化：
        │        │        删除：./backdir/test.txt  ❌
        │        │        保留：./packdir/test.txt.lz  ✅
        │        │
        │        └─→ [步骤 2.5] 更新备份信息
        │              bi._pack_flag = true;  // 标记为已压缩
        │              data->Update(bi);
        │              
        │              Update 流程 (data.hpp: 161-198行)：
        │                ├─→ 加写锁
        │                │     std::unique_lock<std::shared_mutex> lock(_rwlock);
        │                │
        │                ├─→ 构造 UPDATE SQL 语句
        │                │     UPDATE backup_info SET
        │                │       pack_flag=1,
        │                │       ...
        │                │     WHERE url_path='/download/test.txt';
        │                │
        │                ├─→ 执行 SQL
        │                │     stmt->executeUpdate(sql.str());
        │                │
        │                ├─→ 更新内存缓存
        │                │     _table[info._url_path] = info;
        │                │
        │                └─→ 释放锁
        │      }
        │
        └─→ [步骤 3] 休眠后继续下一轮
              usleep(1000);  // 休眠 1000 微秒 = 1 毫秒
              
              然后继续 while 循环，进入下一轮扫描...
      }
```

**压缩前后对比**：

```
压缩前：
  文件系统：
    ./backdir/test.txt (1024 字节)
    ./packdir/ (空)
  
  数据库：
    pack_flag = 0  (未压缩)

压缩后：
  文件系统：
    ./backdir/ (test.txt 已删除)
    ./packdir/test.txt.lz (约 300 字节)
  
  数据库：
    pack_flag = 1  (已压缩)
```

**热点判断实例**：

```
场景 A：用户频繁访问文件
  10:00:00  文件被访问（下载）→ atime 更新
  10:00:15  热点线程扫描：当前时间 - atime = 15秒 < 30秒 → 热点 → 不压缩
  10:00:30  热点线程扫描：当前时间 - atime = 30秒 = 30秒 → 不压缩
  10:00:45  热点线程扫描：当前时间 - atime = 45秒 > 30秒 → 非热点 → 压缩

场景 B：用户长时间不访问
  10:00:00  文件上传完成
  10:00:35  热点线程扫描：超过 30 秒未访问 → 立即压缩
```

**压缩与下载的交互**：

```
时间线：用户下载压缩文件

T1: 热点线程压缩了文件
    - ./backdir/test.txt 被删除
    - ./packdir/test.txt.lz 创建
    - pack_flag = 1

T2: 用户请求下载 /download/test.txt
    ↓
    Download() 检测到 pack_flag = 1
    ↓
    自动解压：./packdir/test.txt.lz → ./backdir/test.txt
    ↓
    删除压缩包：./packdir/test.txt.lz
    ↓
    更新数据库：pack_flag = 0
    ↓
    返回文件内容给用户
    
T3: 文件恢复到未压缩状态
    - ./backdir/test.txt 存在
    - ./packdir/test.txt.lz 不存在
    - pack_flag = 0
```

---

## 多线程并发场景示例

**场景**：热点线程正在压缩文件，同时用户请求下载该文件

```
时间轴：

T1: 热点线程开始处理 test.txt
    ├─→ HotJudge(test.txt) 返回 true（非热点）
    ├─→ 开始压缩文件
    └─→ 此时 pack_flag 仍为 false

T2: 用户发送下载请求
    ├─→ Download() 查询备份信息（需要加读锁）
    ├─→ 此时热点线程可能正在写入（需要加写锁）
    └─→ 读写锁机制保护：
          - 如果热点线程持有写锁 → Download 等待
          - 如果 Download 先获得读锁 → 热点线程等待

T3: 热点线程完成压缩
    ├─→ 删除原文件
    ├─→ Update() 加写锁，设置 pack_flag = 1
    └─→ 释放写锁

T4: Download() 获得读锁
    ├─→ 查询到 pack_flag = 1（已压缩）
    ├─→ 自动解压文件
    ├─→ Update() 设置 pack_flag = 0
    └─→ 返回文件内容
```

**线程安全保护机制**：

```cpp
// data.hpp 中的读写锁

class DataManager {
private:
    std::shared_mutex _rwlock;  // 读写锁
    
    // 读操作：多个线程可同时读
    bool GetOneByURL(...) {
        std::shared_lock<std::shared_mutex> lock(_rwlock);
        // 查询内存哈希表...
    }
    
    // 写操作：独占访问
    bool Update(...) {
        std::unique_lock<std::shared_mutex> lock(_rwlock);
        // 修改数据库和内存...
    }
};
```

**锁机制特性**：
- **读-读并发**：多个 Download 请求可同时执行
- **读-写互斥**：Download 和 Update 不能同时执行
- **写-写互斥**：多个 Update 操作串行执行
- **自动解锁**：锁对象析构时自动释放锁

---

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