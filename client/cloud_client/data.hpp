// data.hpp
#pragma once
#include <unordered_map>
#include<sstream>
#include "util.hpp"
namespace myspace
{
    // 数据管理类
    class DataManager
    {
	public:
		//字符串分割，对序列化字符串进行分割
		// 按指定的分隔符sep进行分割，将分割后的每一跳数据放到数组中
		//"key val key" -> "key" "val" "key"
		int Split(const std::string& str, const std::string& seq, std::vector<std::string>* arry)
		{
			size_t count = 0;
			size_t pos = 0, idx = 0;
			while (idx < str.size())
			{
				pos = str.find(seq, idx);
				if (pos == std::string::npos) break;
				arry->push_back(str.substr(idx, pos - idx));
				idx = pos + 1;
				count++;
			}

			if (idx < str.size())
			{
				//说明str还有最后一截字符串
				arry->push_back(str.substr(idx));
				count++;
			}
			return count;//分割之后数据的个数
		}
		DataManager(const std::string& backup_file)
			:_backup_file(backup_file)
		{
			InitLoad();
		}
		//程序运行时加载以前的数据
		bool InitLoad()
		{
			//1.从文件中读取所有数据
			FileUtil f(_backup_file);
			std::string body;
			f.GetContent(&body);
			//2.按照自定义格式进行数据解析，"key val\nkey val" ->"kay val" "key val"
			//字符串分割函数得到每一项数据
			std::vector<std::string> arr;
			Split(body, "\n", &arr);
			for (auto& a : arr)
			{
				//再字符串分割函数得到key 和 val
				std::vector<std::string> tmp;
				//"key val" -> "key" "val"
				Split(a, " ", &tmp);
				if (tmp.size() != 2) continue;
				//添加到_table中
				_table[tmp[0]] = tmp[1];

			}

			return true;
		}
		//持久化存储
		bool Storage()
		{
			std::stringstream ss;
			//1.获取所有备份信息
			auto it = _table.begin();
			for (; it != _table.end(); it++)
			{
				//2.自定义持久化存储格式组织  key val\nkey val\n
				ss << it->first << " " << it->second << "\n";
			}

			//3.持久化存储
			FileUtil f(_backup_file);
			f.SetContent(ss.str());
			return true;

		}
		bool Insert(const std::string& key, const std::string& val)
		{
			_table[key] = val;
			Storage();
			return true;
		}
		bool Update(const std::string& key, const std::string& val)
		{
			_table[key] = val;
			Storage();
			return true;
		}
		bool GetOneByKey(const std::string& key, std::string* val)
		{
			auto it = _table.find(key);
			if (it == _table.end()) return false;
			*val = it->second;
			return true;
		}
    private:
        std::string _backup_file;                           // 持久化存储文件
        std::unordered_map<std::string, std::string> _table; // 内存中hash存储的文件信息管理表
    };
}
