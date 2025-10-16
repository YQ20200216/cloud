-- 云备份系统数据库表结构
-- 创建数据库
CREATE DATABASE IF NOT EXISTS cloud_backup 
CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

USE cloud_backup;

-- 创建备份文件信息表
CREATE TABLE IF NOT EXISTS backup_info (
    id INT AUTO_INCREMENT PRIMARY KEY COMMENT '自增主键',
    url_path VARCHAR(512) NOT NULL UNIQUE COMMENT '请求URL路径（唯一键）',
    real_path VARCHAR(512) NOT NULL COMMENT '文件实际存储路径',
    pack_path VARCHAR(512) DEFAULT NULL COMMENT '压缩包存储路径',
    pack_flag TINYINT(1) DEFAULT 0 COMMENT '是否压缩（0:否 1:是）',
    fsize BIGINT UNSIGNED DEFAULT 0 COMMENT '文件大小（字节）',
    mtime BIGINT DEFAULT 0 COMMENT '最后修改时间（时间戳）',
    atime BIGINT DEFAULT 0 COMMENT '最后访问时间（时间戳）',
    create_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '记录创建时间',
    update_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '记录更新时间',
    INDEX idx_real_path (real_path(255)),
    INDEX idx_mtime (mtime),
    INDEX idx_atime (atime)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='备份文件信息表';

-- 创建用户（可选）
-- CREATE USER 'cloud_user'@'localhost' IDENTIFIED BY 'cloud_password_123';
-- GRANT ALL PRIVILEGES ON cloud_backup.* TO 'cloud_user'@'localhost';
-- FLUSH PRIVILEGES;

-- 查看表结构
DESC backup_info;

