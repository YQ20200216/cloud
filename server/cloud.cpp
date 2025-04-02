// #include "util.hpp"
// #include <iostream>

// int main(int argc, char* argv[]) {
//     std::cout << "argv[1] 是原始⽂件路径名称\n";
//     std::cout << "argv[2] 是压缩包名称\n";
//     if (argc < 3)
//         return -1;
//     std::string originalFile = argv[1];
//     std::string compressedFile = argv[2];
//     std::string decompressedFile = "test_decompressed.txt";

//     // 写入测试数据到原始文件
//     myspace::FileUtil originalUtil(originalFile);
//     std::string testData = "This is a test file for compression and decompression.";
//     if (originalUtil.SetContent(testData)) {
//         std::cout << "测试数据写入成功" << std::endl;
//     }

//     // 压缩文件
//     if (originalUtil.Compress(compressedFile)) {
//         std::cout << "文件压缩成功" << std::endl;
//     }

//     // 解压文件
//     myspace::FileUtil compressedUtil(compressedFile);
//     if (compressedUtil.UnCompress(decompressedFile)) {
//         std::cout << "文件解压成功" << std::endl;
//     }

//     return 0;
// }    


// #include <iostream>
// #include <vector>
// #include <string>
// #include "util.hpp"

// int main() {
//     // 测试文件路径
//     std::string testFilePath = "test_file.txt";
//     std::string testDirectoryPath = "test_directory";

//     // 测试 Exists 函数
//     myspace::FileUtil fileUtil(testFilePath);
//     if (fileUtil.Exists()) {
//         std::cout << testFilePath << " 存在" << std::endl;
//     } else {
//         std::cout << testFilePath << " 不存在" << std::endl;
//     }

//     // 测试 CreateDirectory 函数
//     myspace::FileUtil dirUtil(testDirectoryPath);
//     if (dirUtil.CreateDirectory()) {
//         std::cout << "成功创建目录: " << testDirectoryPath << std::endl;
//     } else {
//         std::cout << "创建目录失败: " << testDirectoryPath << std::endl;
//     }

//     // 测试 GetDirectory 函数
//     std::vector<std::string> fileNames;
//     if (dirUtil.GetDirectory(fileNames)) {
//         std::cout << "目录 " << testDirectoryPath << " 下的文件有:" << std::endl;
//         for (const auto& fileName : fileNames) {
//             std::cout << fileName << std::endl;
//         }
//     } else {
//         std::cout << "获取目录下文件列表失败: " << testDirectoryPath << std::endl;
//     }

//     return 0;
// }

// #include "config.hpp"
// #include <iostream>

// int main() {
//     // 获取 Config 类的单例实例
//     myspace::Config& config = myspace::Config::GetInstance();

//     // 测试获取各个配置项的值
//     std::cout << "Hot Time: " << config.GetHotTime() << std::endl;
//     std::cout << "Server Port: " << config.GetServerPort() << std::endl;
//     std::cout << "Server IP: " << config.GetServerIp() << std::endl;
//     std::cout << "Download Prefix: " << config.GetDownloadPrefix() << std::endl;
//     std::cout << "Pack File Suffix: " << config.GetPackFileSuffix() << std::endl;
//     std::cout << "Pack Dir: " << config.GetPackDir() << std::endl;
//     std::cout << "Back Dir: " << config.GetBackDir() << std::endl;
//     std::cout << "Backup File: " << config.GetBackupFile() << std::endl;

//     return 0;
// }
    


// #include "data.hpp"
// #include <iostream>
// #include <string>

// int main() {
//     // 测试 BackupInfo 结构体
//     myspace::BackupInfo info;
//     std::string realPath = "./test_file.txt";
//     if (info.NewBackupInfo(realPath)) {
//         std::cout << "BackupInfo 数据填充成功" << std::endl;
//         std::cout << "Real Path: " << info._real_path << std::endl;
//         std::cout << "Pack Path: " << info._pack_path << std::endl;
//         std::cout << "Pack Flag: " << (info._pack_flag ? "true" : "false") << std::endl;
//         std::cout << "File Size: " << info._fsize << std::endl;
//         std::cout << "Last Modify Time: " << info._mtime << std::endl;
//         std::cout << "Last Access Time: " << info._atime << std::endl;
//         std::cout << "URL Path: " << info._url_path << std::endl;
//     } else {
//         std::cout << "BackupInfo 数据填充失败" << std::endl;
//     }

//     // 测试 DataManager 类
//     myspace::DataManager dataManager;

//     // 测试 Insert 方法
//     if (dataManager.Insert(info)) {
//         std::cout << "数据插入成功" << std::endl;
//     } else {
//         std::cout << "数据插入失败" << std::endl;
//     }

//     // 测试 GetOneByURL 方法
//     myspace::BackupInfo retrievedInfo;
//     if (dataManager.GetOneByURL(info._url_path, retrievedInfo)) {
//         std::cout << "通过 URL 获取数据成功" << std::endl;
//         std::cout << "Real Path: " << retrievedInfo._real_path << std::endl;
//     } else {
//         std::cout << "通过 URL 获取数据失败" << std::endl;
//     }

//     // 测试 Update 方法
//     retrievedInfo._pack_flag = true;
//     if (dataManager.Update(retrievedInfo)) {
//         std::cout << "数据更新成功" << std::endl;
//     } else {
//         std::cout << "数据更新失败" << std::endl;
//     }

//     // 测试 GetOneByRealPath 方法
//     myspace::BackupInfo infoByRealPath;
//     if (dataManager.GetOneByRealPath(realPath, infoByRealPath)) {
//         std::cout << "通过真实路径获取数据成功" << std::endl;
//         std::cout << "URL Path: " << infoByRealPath._url_path << std::endl;
//     } else {
//         std::cout << "通过真实路径获取数据失败" << std::endl;
//     }

//     // 测试 GetAll 方法
//     std::vector<myspace::BackupInfo> allInfos;
//     if (dataManager.GetAll(&allInfos)) {
//         std::cout << "获取所有数据成功，数据数量: " << allInfos.size() << std::endl;
//     } else {
//         std::cout << "获取所有数据失败" << std::endl;
//     }

//     return 0;
// }


// #include "hot.hpp"
// #include "util.hpp"
// #include "config.hpp"
// #include <iostream>
// #include <string>

// // 定义全局变量 _data
// myspace::DataManager* _data;
// int main() {
//     // 创建 DataManager 对象并赋值给全局变量 _data
//     _data = new myspace::DataManager();

//     // 创建 HotManager 对象
//     myspace::HotManager hotManager;

//     // 调用 RunModule 方法进行测试
//     // 由于 RunModule 是一个无限循环，这里我们可以选择在一定时间后终止测试
//     // 例如，使用线程和定时器来控制测试时间
//     // 这里简单地调用一次 RunModule 中的单次处理逻辑
//     std::vector<std::string> arr;
//     myspace::FileUtil file("./backdir/");
//     file.GetDirectory(arr);
//     for (auto& a : arr) {
//         if (!hotManager.HotJudge(a)) {
//             continue;
//         }
//         myspace::BackupInfo bi;
//         if (!_data->GetOneByRealPath(a, bi)) {
//             bi.NewBackupInfo(a);
//         }
//         myspace::FileUtil tmp(a);
//         tmp.Compress(bi._pack_path);
//         tmp.Remove();
//         bi._pack_flag = true;
//         _data->Update(bi);
//     }

//     // 释放 _data 内存
//     delete _data;

//     return 0;
// }
    

// server.hpp 部分代码（之前已有，这里完整展示）

#include <thread>
#include "httplib.h"
#include "data.hpp"
#include "util.hpp"
#include "config.hpp"
#include "server.hpp"
#include "hot.hpp"

myspace::DataManager* data;

void HotTest()
{
	myspace::HotManager hot;
	hot.RunModule();
}

void ServiceTest()
{
	myspace::Service srv;
	srv.RunModule();
}
int main() {
    data = new myspace::DataManager();
	std::thread thread_hot_manager(HotTest);
	std::thread thread_service(ServiceTest);
	thread_hot_manager.join();
	thread_service.join();
    return 0;
}    
