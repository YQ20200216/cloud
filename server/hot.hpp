// hot.hpp
#pragma once
#include "data.hpp"
#include <unistd.h>


extern myspace::DataManager *data; // 先声明一个数据管理类的全局变量，外部变量
namespace myspace
{
    class HotManager
    {
    public:
        HotManager()
        {
            Config config = Config::GetInstance();
            _back_dir = config.GetBackDir();
            _pack_dir = config.GetPackDir();
            _pack_suffix = config.GetPackFileSuffix();
            _hot_time = config.GetHotTime();
            //如果目录不存在，创建目录
            FileUtil tmp1(_back_dir);
            FileUtil tmp2(_pack_dir);
            tmp1.CreateDirectory();
            tmp2.CreateDirectory();
        }
        // 热点文件压缩功能实现
        bool RunModule()
        {
            while (1)
            {
                // 1. 遍历备份目录，获取所有文件路径名称
                FileUtil file(_back_dir);
                std::vector<std::string> arr;
                file.GetDirectory(arr);
                // 2. 判断是否位热点文件
                for (auto &a : arr)
                {
                    if (!HotJudge(a))
                    {
                        continue; // 热点文件不处理
                    }
                    // 3. 对非热点文件进行压缩处理
                    // 获取文件的备份信息
                    BackupInfo bi;
                    if (!data->GetOneByRealPath(a, bi))
                    {
                        // 如果文件存在，但是没有备份信息--设置一个新的备份信息
                        bi.NewBackupInfo(a);
                    }
                    FileUtil tmp(a);
                    tmp.Compress(bi._pack_path);
                    // 4. 删除源文件,修改数据管理模块对应的文件信息（压缩标志(_pack_flag)–>true)
                    tmp.Remove();
                    bi._pack_flag = true;
                    data->Update(bi);
                }
                usleep(1000);//每一千毫秒检测一次
            }
        }
        // 判断是否是热点文件
        bool HotJudge(const std::string &filename)
        {
            // 逐个文件获取最后一次访问时间与当前系统时间进行比较判断
            FileUtil file(filename);
            time_t last_atime = file.LastAcccessTime();
            time_t cur_time = time(NULL);
            if (cur_time - last_atime > _hot_time)
            {
                return true;
            }
            return false;
        }
    private:
        std::string _back_dir;    // 备份文件的路径
        std::string _pack_dir;    // 压缩文件的路径
        std::string _pack_suffix; // 压缩文件后缀
        int _hot_time;            // 热点判断时间
    };

}

