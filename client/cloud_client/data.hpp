// data.hpp
#pragma once
#include <unordered_map>
#include<sstream>
#include "util.hpp"
namespace myspace
{
    // ���ݹ�����
    class DataManager
    {
	public:
		//�ַ����ָ�����л��ַ������зָ�
		// ��ָ���ķָ���sep���зָ���ָ���ÿһ�����ݷŵ�������
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
				//˵��str�������һ���ַ���
				arry->push_back(str.substr(idx));
				count++;
			}
			return count;//�ָ�֮�����ݵĸ���
		}
		DataManager(const std::string& backup_file)
			:_backup_file(backup_file)
		{
			InitLoad();
		}
		//��������ʱ������ǰ������
		bool InitLoad()
		{
			//1.���ļ��ж�ȡ��������
			FileUtil f(_backup_file);
			std::string body;
			f.GetContent(&body);
			//2.�����Զ����ʽ�������ݽ�����"key val\nkey val" ->"kay val" "key val"
			//�ַ����ָ���õ�ÿһ������
			std::vector<std::string> arr;
			Split(body, "\n", &arr);
			for (auto& a : arr)
			{
				//���ַ����ָ���õ�key �� val
				std::vector<std::string> tmp;
				//"key val" -> "key" "val"
				Split(a, " ", &tmp);
				if (tmp.size() != 2) continue;
				//��ӵ�_table��
				_table[tmp[0]] = tmp[1];

			}

			return true;
		}
		//�־û��洢
		bool Storage()
		{
			std::stringstream ss;
			//1.��ȡ���б�����Ϣ
			auto it = _table.begin();
			for (; it != _table.end(); it++)
			{
				//2.�Զ���־û��洢��ʽ��֯  key val\nkey val\n
				ss << it->first << " " << it->second << "\n";
			}

			//3.�־û��洢
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
        std::string _backup_file;                           // �־û��洢�ļ�
        std::unordered_map<std::string, std::string> _table; // �ڴ���hash�洢���ļ���Ϣ�����
    };
}
