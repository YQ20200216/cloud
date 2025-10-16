# MySQL 连接问题快速修复

## ❌ 错误信息
```
Access denied for user 'root'@'localhost'
```

## ✅ 快速修复（3步）

### 方法1：自动修复脚本（推荐）

```bash
cd /home/aaa/cloud/new_cloud/cloud/server
./fix_mysql.sh
```

---

### 方法2：手动修复

#### 步骤1：重置密码

```bash
# 登录 MySQL（用 sudo，无需密码）
sudo mysql
```

在 MySQL 命令行中执行：

```sql
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '123456';
FLUSH PRIVILEGES;
EXIT;
```

#### 步骤2：创建数据库

```bash
# 执行 SQL 脚本
mysql -u root -p123456 < create_tables.sql
```

或者手动创建：

```bash
mysql -u root -p123456
```

```sql
CREATE DATABASE IF NOT EXISTS cloud_backup CHARACTER SET utf8mb4;
USE cloud_backup;

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
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

EXIT;
```

#### 步骤3：验证

```bash
# 测试连接
mysql -u root -p123456 -e "USE cloud_backup; SHOW TABLES;"
```

应该看到：
```
+--------------------------+
| Tables_in_cloud_backup   |
+--------------------------+
| backup_info              |
+--------------------------+
```

#### 步骤4：运行程序

```bash
./cloud
```

---

## 🔧 如果还是失败

### 检查1：MySQL 是否运行

```bash
sudo systemctl status mysql
```

如果没运行：
```bash
sudo systemctl start mysql
```

### 检查2：修改 data.hpp 中的密码

编辑 `data.hpp` 第 58 行：
```cpp
std::string db_password = "123456";  // 改成您的实际密码
```

### 检查3：使用其他用户

如果 root 用户有问题，创建新用户：

```bash
sudo mysql
```

```sql
CREATE USER 'cloud_user'@'localhost' IDENTIFIED BY 'cloud123';
GRANT ALL PRIVILEGES ON cloud_backup.* TO 'cloud_user'@'localhost';
FLUSH PRIVILEGES;
EXIT;
```

然后修改 `data.hpp`：
```cpp
std::string db_user = "cloud_user";
std::string db_password = "cloud123";
```

---

## ✅ 修复完成标志

成功后会看到：
```
✓ MySQL 连接成功：cloud_backup
加载了 0 条备份记录
```

现在就可以正常使用了！

