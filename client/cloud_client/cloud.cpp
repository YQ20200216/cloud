//#include <iostream>
//#include <vector>
//#include <string>
//#include "util.hpp"
//
//int main() {
//    // 测试文件路径
//    std::string testFilePath = "test_file.txt";
//    std::string testDirectoryPath = "test_directory";
//
//    // 创建 FileUtil 对象
//    myspace::FileUtil fileUtil(testFilePath);
//    myspace::FileUtil dirUtil(testDirectoryPath);
//
//    // 测试 SetContent 方法
//    std::string testContent = "This is a test content.";
//    bool setContentResult = fileUtil.SetContent(testContent);
//    std::cout << "SetContent result: " << (setContentResult ? "Success" : "Failed") << std::endl;
//
//    // 测试 FileSize 方法
//    uint64_t fileSize = fileUtil.FileSize();
//    std::cout << "File size: " << fileSize << " bytes" << std::endl;
//
//    // 测试 LastModifyTime 方法
//    time_t lastModifyTime = fileUtil.LastModifyTime();
//    std::cout << "Last modify time: " << lastModifyTime << std::endl;
//
//    // 测试 LastAcccessTime 方法
//    time_t lastAccessTime = fileUtil.LastAcccessTime();
//    std::cout << "Last access time: " << lastAccessTime << std::endl;
//
//    // 测试 FileName 方法
//    std::string fileName = fileUtil.FileName();
//    std::cout << "File name: " << fileName << std::endl;
//
//    // 测试 GetContent 方法
//    std::string readContent;
//    bool getContentResult = fileUtil.GetContent(&readContent);
//    std::cout << "GetContent result: " << (getContentResult ? "Success" : "Failed") << std::endl;
//    if (getContentResult) {
//        std::cout << "Read content: " << readContent << std::endl;
//    }
//
//    // 测试 GetPosLen 方法
//    std::string partialContent;
//    bool getPosLenResult = fileUtil.GetPosLen(&partialContent, 0, 5);
//    std::cout << "GetPosLen result: " << (getPosLenResult ? "Success" : "Failed") << std::endl;
//    if (getPosLenResult) {
//        std::cout << "Partial content: " << partialContent << std::endl;
//    }
//
//    // 测试 Exists 方法
//    bool fileExists = fileUtil.Exists();
//    std::cout << "File exists: " << (fileExists ? "Yes" : "No") << std::endl;
//
//    // 测试 CreateDirectory 方法
//    bool createDirResult = dirUtil.CreateDirectory();
//    std::cout << "CreateDirectory result: " << (createDirResult ? "Success" : "Failed") << std::endl;
//
//    // 测试 GetDirectory 方法
//    std::vector<std::string> fileList;
//    bool getDirResult = dirUtil.GetDirectory(fileList);
//    std::cout << "GetDirectory result: " << (getDirResult ? "Success" : "Failed") << std::endl;
//    if (getDirResult) {
//        std::cout << "Files in directory:" << std::endl;
//        for (const auto& file : fileList) {
//            std::cout << "  " << file << std::endl;
//        }
//    }
//
//    // 测试 Remove 方法
//    bool removeResult = fileUtil.Remove();
//    std::cout << "Remove result: " << (removeResult ? "Success" : "Failed") << std::endl;
//
//    return 0;
//}


//#include <iostream>
//#include <vector>
//#include <string>
//#include "data.hpp"
//
//int main() {
//    // 创建 DataManager 对象，指定备份文件路径
//    myspace::DataManager dataMgr("test_backup.txt");
//
//    // 测试 Split 方法
//    std::string testStr = "key1 val1 key2 val2";
//    std::vector<std::string> splitResult;
//    int splitCount = dataMgr.Split(testStr, " ", &splitResult);
//    std::cout << "Split method test: " << splitCount << " elements split." << std::endl;
//    for (const auto& s : splitResult) {
//        std::cout << s << " ";
//    }
//    std::cout << std::endl;
//
//    // 测试 Insert 方法
//    bool insertResult = dataMgr.Insert("key3", "val3");
//    std::cout << "Insert method test: " << (insertResult ? "Success" : "Failed") << std::endl;
//
//    // 测试 GetOneByKey 方法
//    std::string retrievedVal;
//    bool getResult = dataMgr.GetOneByKey("key3", &retrievedVal);
//    std::cout << "GetOneByKey method test: " << (getResult ? "Success, value: " + retrievedVal : "Failed") << std::endl;
//
//    // 测试 Update 方法
//    bool updateResult = dataMgr.Update("key3", "new_val3");
//    std::cout << "Update method test: " << (updateResult ? "Success" : "Failed") << std::endl;
//
//    // 再次测试 GetOneByKey 方法，验证更新后的值
//    getResult = dataMgr.GetOneByKey("key3", &retrievedVal);
//    std::cout << "GetOneByKey after update test: " << (getResult ? "Success, value: " + retrievedVal : "Failed") << std::endl;
//
//    // 测试 Storage 方法，这里简单认为调用 Storage 方法无异常即为成功
//    bool storageResult = dataMgr.Storage();
//    std::cout << "Storage method test: " << (storageResult ? "Success" : "Failed") << std::endl;
//
//    return 0;
//}


//#include <iostream>
//#include <vector>
//#include <string>
//#include "data.hpp"
//#include "httplib.h"
//#include "Windows.h"
//#include "backup.hpp"
//
//
//namespace myspace {
//    class BackUp;
//    // 模拟文件存在的情况
//    class MockFileUtil {
//    public:
//        MockFileUtil(const std::string& filename) : _filename(filename) {}
//
//        // 模拟获取文件大小
//        uint64_t FileSize() {
//            return 1024;  // 模拟文件大小
//        }
//
//        // 模拟获取文件最后修改时间
//        time_t LastModifyTime() {
//            return time(nullptr) - 5;  // 模拟修改时间在5秒前
//        }
//
//        // 模拟获取文件内容
//        bool GetContent(std::string* body) {
//            *body = "Mock file content";  // 模拟文件内容
//            return true;
//        }
//
//        // 模拟获取目录下所有文件名（简单返回一个文件名列表）
//        bool GetDirectory(std::vector<std::string>& arry) {
//            arry.push_back("test_file.txt");  // 模拟目录下有一个文件
//            return true;
//        }
//
//    private:
//        std::string _filename;
//    };
//}
//
//int main() {
//    // 创建 MockFileUtil 的实例，模拟文件操作
//    myspace::MockFileUtil mockFileUtil("test_file.txt");
//
//    // 创建 DataManager 的实例，指定备份文件路径（这里使用临时路径，可根据实际情况修改）
//    myspace::DataManager dataMgr("test_backup.txt");
//
//    // 创建 BackUp 的实例，指定监控文件夹和备份文件路径
//    myspace::BackUp backUp("./test_dir", "test_backup.txt");
//
//    // 测试 GetFileIdentifier 方法
//    std::string fileIdentifier = backUp.GetFileIdentifier("test_file.txt");
//    std::cout << "GetFileIdentifier method test: " << fileIdentifier << std::endl;
//
//    // 测试 JudgeUpload 方法
//    bool shouldUpload = backUp.JudgeUpload("test_file.txt");
//    std::cout << "JudgeUpload method test: " << (shouldUpload ? "Should upload" : "Should not upload") << std::endl;
//
//    // 测试 Upload 方法（这里只是模拟，实际需要确保服务器端正确处理请求）
//    bool uploadResult = backUp.Upload("test_file.txt");
//    std::cout << "Upload method test: " << (uploadResult ? "Success" : "Failed") << std::endl;
//
//    // 测试 RunModule 方法（这里只是简单运行一段时间，可根据实际情况调整）
//    std::cout << "Running RunModule for a short period..." << std::endl;
//    for (int i = 0; i < 5; ++i) {
//        backUp.RunModule();
//        Sleep(1000);  // 暂停1秒
//    }
//
//    return 0;
//}



#include "util.hpp"
#include "data.hpp"
#include "backup.hpp"

#define BACKUP_FILE "./test_backup.txt"//指定备份文件路径
#define BACKUP_DIR "./test_dir"//指定备份文件路径
int main()
{
	myspace::BackUp backup(BACKUP_DIR, BACKUP_FILE);
	backup.RunModule();
	return 0;
}
