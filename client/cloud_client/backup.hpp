// backup.hpp
#pragma once
#include"data.hpp"
#include"httplib.h"
#include<Windows.h>//Sleep，注意头文件顺序问题，win要在httplib后面

namespace myspace
{
#define SERVER_IP "119.91.60.49"
#define SERVER_PORT 8080
	class BackUp
	{
	public:
		BackUp(const std::string& back_dir, const std::string& back_file)
			:_back_dir(back_dir)
			, _data(new DataManager(back_file))
		{}
		//获取文件的唯一标识
		std::string GetFileIdentifier(const std::string& filename)
		{
			//a.txt-fsize-mtime
			FileUtil f(filename);
			std::stringstream ss;
			ss << f.FileName() << "-" << f.FileSize() << "-" << f.LastModifyTime();
			return ss.str();
		}
		bool Upload(const std::string& filename)
		{
			//1.获取文件数据
			FileUtil f(filename);
			std::string body;
			f.GetContent(&body);//获取文件数据
			//2.搭建http客户端，上传文件
			//httplib实例化一个client对象
			httplib::Client client(SERVER_IP, SERVER_PORT);
			httplib::MultipartFormData item;
			item.content = body;//正文就是文件数据
			item.filename = f.FileName();
			item.name = "file";//字段表示
			item.content_type = "application/octet-stream";//二进制流数据
			httplib::MultipartFormDataItems items;
			items.push_back(item);
			auto res = client.Post("/upload", items);
			if (!res || res->status != 200)
			{
				return false;
			}
			return true;
		}
		//判断文件是否需要上传
		bool JudgeUpload(const std::string& filename)
		{
			//文件新增||被修改过 需要上传
			//新增判断：有没有备份信息  修改判断：有备份信息 但是文件的唯一标识不一致
			std::string id;
			// 找到历史信息
			if (_data->GetOneByKey(filename, &id))
			{
				//有历史信息判断是否修改过（文件唯一标识）
				std::string new_id = GetFileIdentifier(filename);
				if (new_id == id)//文件标识符一致,不用上传
				{
					return false;
				}
			}
			//注意：这里有一种情况
			//一个大文件正在被拷贝,拷贝需要一个过程，该文件的唯一标识时时刻刻都不一致
			//如果文件唯一标识不一致就上传，该文件会被上传很多次
			//因此我们对于被修改文件的上传应该再加一个条件
			//一段时间没有被修改过  修改时间间隔大于3秒
			FileUtil f(filename);
			//当前时间-最后修改时间
			if (time(NULL) - f.LastModifyTime() < 3)
			{
				//修改时间间隔小于3 认为文件还在修改中，不上传
				return false;
			}
			std::cout << filename << " need upload "<<std::endl;
			return true;

		}
		//客户端整体的逻辑合并 运行模块
		bool RunModule()
		{
			while (1)
			{
				//1. 遍历指定文件夹，获取所有文件信息
				FileUtil f(_back_dir);
				std::vector<std::string> arr;
				f.GetDirectory(arr);
				//2. 逐一判断文件是否需要上传
				for (auto& a : arr)
				{
					if (JudgeUpload(a) == false)
					{
						continue;
					}
					if (Upload(a) == true)//3. 需要备份的文件进行上传备份
					{
						//如果文件上传成功了 新增文件备份信息
						//向数据管理模块插入文件名称和唯一标识
						_data->Insert(a, GetFileIdentifier(a));
						std::cout << a << " upload sucess" << std::endl;
					}
				}
				//等待一秒
				Sleep(1);
			}
		}
	private:
		std::string _back_dir;//监控文件
		DataManager* _data;//数据管理
	};
}


