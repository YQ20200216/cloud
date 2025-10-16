// //server.hpp
// #pragma once
// #include "data.hpp"
// #include "httplib.h"
// // 因为业务处理的回调函数没有传入参数的地方，因此无法直接访问外部的数据管理模块数据
// // 可以使用lamda表达式解决，但是所有的业务功能都要在一个函数内实现，于功能划分上模块不够清晰
// // 因此将数据管理模块的对象定义为全局数据,在这里声明一下，就可以在任意位置访问了
// extern myspace::DataManager *_data;
// namespace myspace
// {
//     class Service
//     {
//     public:
//         Service()
//         {
//             Config config =Config :: GetInstance();
//             _server_port= config.GetServerPort();
//             _server_ip= config.GetServerIp();
//             _download_prefix= config.GetDownloadPrefix();
//         }
//         // 服务启动（httplib 绑定对应处理函数）
//         bool RunModule() 
//         {
//           _server.Post("/upload",UpLoad); 
//           _server.Get("/listshow",ListShow); 
//         //正则表达式 ：
//         //  .  匹配除换行符以外的任意单个字符。例如，a.c 可以匹配 abc、adc   
//         //  *  匹配前面的元素零次或多次。例如，ab* 可以匹配 a、ab、abb  
//         // .*  匹配任意字符任意次数
//           _server.Get("/download/(.*)",DownLoad);
//         //   _server.listen("0.0.0.0",_server_port);//启动服务器
//           _server.listen(_server_ip,_server_port);//启动服务器
//         }

//         // 注意回调函数都需要设置成static，因为httplib库中函数要求的参数只有两个
//         // 如果不用static修饰，那么会多出来一个this指针

//         // 上传文件
//         static void UpLoad(const httplib::Request &req, httplib::Response &res)
//         {
//             //post/upload  文件数据在正文中（分区存储，正文并不是全是文件数据）
            
//             std::cout<<"uploading ..."<<std::endl;
//             //判断是否有文件上传
//             auto ret=req.has_file("file");
//             if(!ret)
//             {
//               res.status=400;                                                                                                                                                               
//               return;
//             }
//             const auto& file=req.get_file_value("file");
//             std::string back_dir=Config::GetInstance().GetBackDir();
//             std::string realpath=back_dir+file.filename;
//             FileUtil f(realpath);
//             f.SetContent(file.content);//将数据写入文件
//             BackupInfo info;
//             info.NewBackupInfo(realpath);
//             _data->Insert(info);//向数据管理模块添加对应的备份信息
//         }

//         // 文件列表展示
//         // http://119.91.60.49:8080/listshow
//         static void ListShow(const httplib::Request &req, httplib::Response &res)
//         {
//             //1.获取所有的文件备份信息
//             std::vector<BackupInfo> arr;
//             _data->GetAll(&arr);
//             //组织html文件数据
//             std::stringstream ss;
//             ss<<"<html><head><title>Download</title></head>";
//             ss<<"<body><h1>Download</h1><table>";
//             for(auto &a: arr)
//             {
//                 ss<<"<tr>";
//                 std::string filename=FileUtil(a._real_path).FileName();
//                 ss<<"<td><a href='"<<a._url_path<<"'>"<<filename<<"</a></td>";
//                 ss<<"<td align='right'>"<<TimetoStr(a._mtime)<<"</td>";
//                 ss<<"<td align='right'>"<<a._fsize/1024<<"k</td>";
//                 ss<<"</tr>";
//             }
//             ss<<"</table></body></html>";
//             res.body=ss.str();
//             res.set_header("Content-Type","text/html");
//             res.status=200;
//         }
//         static std::string TimetoStr(time_t t)
//         {
//             std::string tmp=std::ctime(&t);
//             return tmp;
//         }

//         // 组装文件
//         static std::string GetETag(const BackupInfo &info)
//         {
//             // etg:filename-fsize-mtime
//             FileUtil fu(info._real_path);
//             std::string etag = fu.FileName();
//             etag += "-";
//             etag += std::to_string(info._fsize);
//             etag += "-";
//             etag += std::to_string(info._mtime);
//             return etag;
//         }
//         // 文件下载
//         // 
//         static void DownLoad(const httplib::Request &req, httplib::Response &res)
//         {
//             // 1. 获取客户端请求的资源路径path  req.path
//             // 2. 根据资源路径，获取文件的备份信息
//             BackupInfo info;
//             _data->GetOneByURL(req.path, info);
//             // 3. a:判断文件是否被压缩，如果被压缩，先进行解压
//             if (info._pack_flag)
//             {
//                 FileUtil fu(info._pack_path);
//                 fu.UnCompress(info._real_path); // 将文件解压到备份目录下
//                 //   b:删除压缩包，修改备份信息（文件已经没有被压缩）
//                 fu.Remove();
//                 info._pack_flag = false;
//                 _data->Update(info);
//             }
//             // 4. 读取文件数据，并将数据写入rsp.body中
//             FileUtil fu(info._real_path);
//             fu.GetContent(&res.body);
//             // 判断是否断点续传
//             bool retrans = false; // 符合断点续传的标志位
//             std::string old_etag;
//             if (req.has_file("If_Range"))
//             {
//                 // 有if-range字段&&etag相同 ：符合断点续传
//                 old_etag = req.get_header_value("If_Range");
//                 if (old_etag == GetETag(info))
//                 {
//                     retrans = true;
//                 }
//             }
//             if (retrans == false)
//             {
//                 // 没有If-Rang字段||etag值不一样  则是正常下载（重新返回全部数据）

//                 // 5. 设置响应头部字段：ETag,Accept-Ranges:bytes
//                 res.set_header("Accept-Ranges", "bytes");
//                 res.set_header("ETag", GetETag(info));
//                 res.set_header("Content-Type", "application/octet-stream"); // application/octet-stream——表示的是我响应的正文是一个二进制数据流（常用于文件下载）
//                 res.status = 200;
//             }
//             else
//             {
//                 // httplib已近支持力断点续传
//                 //只需要我们用户将文件所有数据取到res.body中，它内部会自动根据请求区间，从body中取出指定区间数据进行响应
//                 fu.GetContent(&res.body);
//                 res.set_header("Accept-Ranges", "bytes");
//                 res.set_header("ETag", GetETag(info));
//                 res.status = 206;
//             }
//         }
//     private:
//         int _server_port;             // 端口号
//         std::string _server_ip;       // 服务器ip
//         std::string _download_prefix; // 下载路径前缀
//         httplib::Server _server;      // httplib库搭建服务器
//     };

// }


//server.hpp
#pragma once
#include "data.hpp"
#include "httplib.h"

extern myspace::DataManager *data;
namespace myspace
{
    class Service
    {
    public:
        Service()
        {
            Config cf = Config::GetInstance();
            _server_port = cf.GetServerPort();
            _server_ip = cf.GetServerIp();
            _download_prefix = cf.GetDownloadPrefix();
        }
        // 服务启动（httplib 绑定对应处理函数）
        bool RunModule()
        {
            //注册路由
            _server.Post("/upload", Upload);    // 文件上传请求
            _server.Get("/listshow", ListShow); // 文件查看请求
            _server.Get("/", ListShow);
        //  .  匹配除换行符以外的任意单个字符。例如，a.c 可以匹配 abc、adc   
        //  *  匹配前面的元素零次或多次。例如，ab* 可以匹配 a、ab、abb  
        // .*  匹配任意字符任意次数
            std::string download_url = _download_prefix + "(.*)";
            _server.Get(download_url, Download); // 文件下载请求
            _server.listen(_server_ip.c_str(), _server_port);
            return true;
        }
        // 注意回调函数都需要设置成static，因为httplib库中函数要求的参数只有两个
        // 如果不用static修饰，那么会多出来一个this指针
        // 上传文件
        static void Upload(const httplib::Request &req, httplib::Response &rsp)
        {
            // post /upload  文件数据在正文中（正文并不全是文件数据）
            std::cout<<"uploading ..."<<std::endl;
            auto ret = req.has_file("file"); // 判断有没有上传的文件区域，file来源后面HTML表单中name='file'
            if (ret == false)
            {
                rsp.status = 400;
                return;
            }
            const auto &file = req.get_file_value("file");//获取上传文件对象
            // file.filename//文件名称    file.content//文件数据
            std::string back_dir = Config::GetInstance().GetBackDir();
            std::string realpath = back_dir + FileUtil(file.filename).FileName();
            FileUtil fu(realpath);
            fu.SetContent(file.content); // 将数据写入文件中；
            BackupInfo info;
            info.NewBackupInfo(realpath); // 组织备份的文件信息
            data->Insert(info);           // 向数据管理模块添加备份的文件信息
        }

        static std::string TimetoStr(time_t t)
        {
            std::string tmp = std::ctime(&t);
            return tmp;
        }
        // 文件列表展示
        // http://119.91.60.49:8080/listshow
        static void ListShow(const httplib::Request &req, httplib::Response &rsp)
        {
            // 1. 获取所有的文件备份信息
            std::vector<BackupInfo> arry;
            data->GetAll(&arry);
            // 2. 根据所有备份信息，组织html文件数据
            std::stringstream ss;
            ss << "<html><body>";
            ss<<"<form action='/upload' method='post' enctype='multipart/form-data'>";
            ss<<" <input type='file' name='file'>";
            ss<<"<input type='submit' value='upload'>";
            ss<<" </form></body></html>";
              
            ss<< "<html><head><title>Download</title></head>";
            ss << "<body><h1>Download</h1><table>";
            for (auto &a : arry)
            {
                ss << "<tr>";
                std::string filename = FileUtil(a._real_path).FileName();
                ss << "<td><a href='" << a._url_path << "'>" << filename << "</a></td>";
                ss << "<td align='right'>" << TimetoStr(a._mtime) << "</td>";
                ss << "<td align='right'>" << a._fsize / 1024 << "k</td>";
                ss << "</tr>";
            }
            ss << "</table></body></html>";
            rsp.body = ss.str();
            rsp.set_header("Content-Type", "text/html");
            rsp.status = 200;
            return;
        }
        // 组装文件
        static std::string GetETag(const BackupInfo &info)
        {
            // etg :  filename-fsize-mtime
            FileUtil fu(info._real_path);
            std::string etag = fu.FileName();
            etag += "-";
            etag += std::to_string(info._fsize);
            etag += "-";
            etag += std::to_string(info._mtime);
            return etag;
        }
        // 文件下载
        static void Download(const httplib::Request &req, httplib::Response &rsp)
        {
            // 1. 获取客户端请求的资源路径path   req.path
            // 2. 根据资源路径，获取文件备份信息
            BackupInfo info;
            data->GetOneByURL(req.path, info);
            // 3. 判断文件是否被压缩，如果被压缩，要先解压缩,
            if (info._pack_flag == true)
            {
                FileUtil fu(info._pack_path);
                fu.UnCompress(info._real_path); // 将文件解压到备份目录下
                // 4. 删除压缩包，修改备份信息（已经没有被压缩）
                fu.Remove();
                info._pack_flag = false;
                data->Update(info);
            }

            bool retrans = false; // 是否需要断点续传标志
            std::string old_etag;
            if (req.has_header("If-Range"))//来源HTTP/1.1协议
            {
                old_etag = req.get_header_value("If-Range");
                // 有If-Range字段且，这个字段的值与请求文件的最新etag一致则符合断点续传
                if (old_etag == GetETag(info))
                {
                    retrans = true;
                }
            }

            // 4. 读取文件数据，放入rsp.body中
            FileUtil fu(info._real_path);
            if (retrans == false)
            {
                /*HTTP/1.1 200 OK
                Accept-Ranges: bytes                           ← HTTP 标准字段
                ETag: "bigfile.zip-5000001-1729087200"        ← HTTP 标准字段
                Content-Type: application/octet-stream         ← HTTP 标准字段
                Content-Length: 5000001

                [完整文件数据...]*/
                fu.GetContent(&rsp.body);
                // 5. 设置响应头部字段： ETag， Accept-Ranges: bytes
                rsp.set_header("Accept-Ranges", "bytes");//支持按字节范围请求
                rsp.set_header("ETag", GetETag(info));// 示例值："bigfile.zip-5000001-1729087200"
                rsp.set_header("Content-Type", "application/octet-stream");//指定响应内容为二进制文件
                rsp.status = 200;
            }
            else
            {
                // httplib内部实现了对于区间请求也就是断点续传请求的处理
                // 只需要我们用户将文件所有数据读取到rsp.body中，它内部会自动根据请求
                // 区间，从body中取出指定区间数据进行响应
                //  std::string  range = req.get_header_val("Range"); bytes=start-end
                /*HTTP/1.1 206 Partial Content
                Accept-Ranges: bytes                           ← HTTP 标准字段
                ETag: "bigfile.zip-5000001-1729087200"        ← HTTP 标准字段
                Content-Type: application/octet-stream         ← HTTP 标准字段
                Content-Range: bytes 1000000-5000000/5000001   ← HTTP 标准字段（httplib 自动添加）

                [部分文件数据...]
                */
                fu.GetContent(&rsp.body);
                rsp.set_header("Accept-Ranges", "bytes");
                rsp.set_header("ETag", GetETag(info));
                rsp.set_header("Content-Type", "application/octet-stream");
                rsp.status = 206; // 区间请求响应的是206
            }
        }
    private:
        int _server_port;
        std::string _server_ip;
        std::string _download_prefix;
        httplib::Server _server;
    };
}