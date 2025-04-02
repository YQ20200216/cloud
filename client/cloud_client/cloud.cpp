//#include <iostream>
//#include <vector>
//#include <string>
//#include "util.hpp"
//
//int main() {
//    // �����ļ�·��
//    std::string testFilePath = "test_file.txt";
//    std::string testDirectoryPath = "test_directory";
//
//    // ���� FileUtil ����
//    myspace::FileUtil fileUtil(testFilePath);
//    myspace::FileUtil dirUtil(testDirectoryPath);
//
//    // ���� SetContent ����
//    std::string testContent = "This is a test content.";
//    bool setContentResult = fileUtil.SetContent(testContent);
//    std::cout << "SetContent result: " << (setContentResult ? "Success" : "Failed") << std::endl;
//
//    // ���� FileSize ����
//    uint64_t fileSize = fileUtil.FileSize();
//    std::cout << "File size: " << fileSize << " bytes" << std::endl;
//
//    // ���� LastModifyTime ����
//    time_t lastModifyTime = fileUtil.LastModifyTime();
//    std::cout << "Last modify time: " << lastModifyTime << std::endl;
//
//    // ���� LastAcccessTime ����
//    time_t lastAccessTime = fileUtil.LastAcccessTime();
//    std::cout << "Last access time: " << lastAccessTime << std::endl;
//
//    // ���� FileName ����
//    std::string fileName = fileUtil.FileName();
//    std::cout << "File name: " << fileName << std::endl;
//
//    // ���� GetContent ����
//    std::string readContent;
//    bool getContentResult = fileUtil.GetContent(&readContent);
//    std::cout << "GetContent result: " << (getContentResult ? "Success" : "Failed") << std::endl;
//    if (getContentResult) {
//        std::cout << "Read content: " << readContent << std::endl;
//    }
//
//    // ���� GetPosLen ����
//    std::string partialContent;
//    bool getPosLenResult = fileUtil.GetPosLen(&partialContent, 0, 5);
//    std::cout << "GetPosLen result: " << (getPosLenResult ? "Success" : "Failed") << std::endl;
//    if (getPosLenResult) {
//        std::cout << "Partial content: " << partialContent << std::endl;
//    }
//
//    // ���� Exists ����
//    bool fileExists = fileUtil.Exists();
//    std::cout << "File exists: " << (fileExists ? "Yes" : "No") << std::endl;
//
//    // ���� CreateDirectory ����
//    bool createDirResult = dirUtil.CreateDirectory();
//    std::cout << "CreateDirectory result: " << (createDirResult ? "Success" : "Failed") << std::endl;
//
//    // ���� GetDirectory ����
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
//    // ���� Remove ����
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
//    // ���� DataManager ����ָ�������ļ�·��
//    myspace::DataManager dataMgr("test_backup.txt");
//
//    // ���� Split ����
//    std::string testStr = "key1 val1 key2 val2";
//    std::vector<std::string> splitResult;
//    int splitCount = dataMgr.Split(testStr, " ", &splitResult);
//    std::cout << "Split method test: " << splitCount << " elements split." << std::endl;
//    for (const auto& s : splitResult) {
//        std::cout << s << " ";
//    }
//    std::cout << std::endl;
//
//    // ���� Insert ����
//    bool insertResult = dataMgr.Insert("key3", "val3");
//    std::cout << "Insert method test: " << (insertResult ? "Success" : "Failed") << std::endl;
//
//    // ���� GetOneByKey ����
//    std::string retrievedVal;
//    bool getResult = dataMgr.GetOneByKey("key3", &retrievedVal);
//    std::cout << "GetOneByKey method test: " << (getResult ? "Success, value: " + retrievedVal : "Failed") << std::endl;
//
//    // ���� Update ����
//    bool updateResult = dataMgr.Update("key3", "new_val3");
//    std::cout << "Update method test: " << (updateResult ? "Success" : "Failed") << std::endl;
//
//    // �ٴβ��� GetOneByKey ��������֤���º��ֵ
//    getResult = dataMgr.GetOneByKey("key3", &retrievedVal);
//    std::cout << "GetOneByKey after update test: " << (getResult ? "Success, value: " + retrievedVal : "Failed") << std::endl;
//
//    // ���� Storage �������������Ϊ���� Storage �������쳣��Ϊ�ɹ�
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
//    // ģ���ļ����ڵ����
//    class MockFileUtil {
//    public:
//        MockFileUtil(const std::string& filename) : _filename(filename) {}
//
//        // ģ���ȡ�ļ���С
//        uint64_t FileSize() {
//            return 1024;  // ģ���ļ���С
//        }
//
//        // ģ���ȡ�ļ�����޸�ʱ��
//        time_t LastModifyTime() {
//            return time(nullptr) - 5;  // ģ���޸�ʱ����5��ǰ
//        }
//
//        // ģ���ȡ�ļ�����
//        bool GetContent(std::string* body) {
//            *body = "Mock file content";  // ģ���ļ�����
//            return true;
//        }
//
//        // ģ���ȡĿ¼�������ļ������򵥷���һ���ļ����б�
//        bool GetDirectory(std::vector<std::string>& arry) {
//            arry.push_back("test_file.txt");  // ģ��Ŀ¼����һ���ļ�
//            return true;
//        }
//
//    private:
//        std::string _filename;
//    };
//}
//
//int main() {
//    // ���� MockFileUtil ��ʵ����ģ���ļ�����
//    myspace::MockFileUtil mockFileUtil("test_file.txt");
//
//    // ���� DataManager ��ʵ����ָ�������ļ�·��������ʹ����ʱ·�����ɸ���ʵ������޸ģ�
//    myspace::DataManager dataMgr("test_backup.txt");
//
//    // ���� BackUp ��ʵ����ָ������ļ��кͱ����ļ�·��
//    myspace::BackUp backUp("./test_dir", "test_backup.txt");
//
//    // ���� GetFileIdentifier ����
//    std::string fileIdentifier = backUp.GetFileIdentifier("test_file.txt");
//    std::cout << "GetFileIdentifier method test: " << fileIdentifier << std::endl;
//
//    // ���� JudgeUpload ����
//    bool shouldUpload = backUp.JudgeUpload("test_file.txt");
//    std::cout << "JudgeUpload method test: " << (shouldUpload ? "Should upload" : "Should not upload") << std::endl;
//
//    // ���� Upload ����������ֻ��ģ�⣬ʵ����Ҫȷ������������ȷ��������
//    bool uploadResult = backUp.Upload("test_file.txt");
//    std::cout << "Upload method test: " << (uploadResult ? "Success" : "Failed") << std::endl;
//
//    // ���� RunModule ����������ֻ�Ǽ�����һ��ʱ�䣬�ɸ���ʵ�����������
//    std::cout << "Running RunModule for a short period..." << std::endl;
//    for (int i = 0; i < 5; ++i) {
//        backUp.RunModule();
//        Sleep(1000);  // ��ͣ1��
//    }
//
//    return 0;
//}



#include "util.hpp"
#include "data.hpp"
#include "backup.hpp"

#define BACKUP_FILE "./test_backup.txt"//ָ�������ļ�·��
#define BACKUP_DIR "./test_dir"//ָ�������ļ�·��
int main()
{
	myspace::BackUp backup(BACKUP_DIR, BACKUP_FILE);
	backup.RunModule();
	return 0;
}
