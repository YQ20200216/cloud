// backup.hpp
#pragma once
#include"data.hpp"
#include"httplib.h"
#include<Windows.h>//Sleep��ע��ͷ�ļ�˳�����⣬winҪ��httplib����

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
		//��ȡ�ļ���Ψһ��ʶ
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
			//1.��ȡ�ļ�����
			FileUtil f(filename);
			std::string body;
			f.GetContent(&body);//��ȡ�ļ�����
			//2.�http�ͻ��ˣ��ϴ��ļ�
			//httplibʵ����һ��client����
			httplib::Client client(SERVER_IP, SERVER_PORT);
			httplib::MultipartFormData item;
			item.content = body;//���ľ����ļ�����
			item.filename = f.FileName();
			item.name = "file";//�ֶα�ʾ
			item.content_type = "application/octet-stream";//������������
			httplib::MultipartFormDataItems items;
			items.push_back(item);
			auto res = client.Post("/upload", items);
			if (!res || res->status != 200)
			{
				return false;
			}
			return true;
		}
		//�ж��ļ��Ƿ���Ҫ�ϴ�
		bool JudgeUpload(const std::string& filename)
		{
			//�ļ�����||���޸Ĺ� ��Ҫ�ϴ�
			//�����жϣ���û�б�����Ϣ  �޸��жϣ��б�����Ϣ �����ļ���Ψһ��ʶ��һ��
			std::string id;
			// �ҵ���ʷ��Ϣ
			if (_data->GetOneByKey(filename, &id))
			{
				//����ʷ��Ϣ�ж��Ƿ��޸Ĺ����ļ�Ψһ��ʶ��
				std::string new_id = GetFileIdentifier(filename);
				if (new_id == id)//�ļ���ʶ��һ��,�����ϴ�
				{
					return false;
				}
			}
			//ע�⣺������һ�����
			//һ�����ļ����ڱ�����,������Ҫһ�����̣����ļ���Ψһ��ʶʱʱ�̶̿���һ��
			//����ļ�Ψһ��ʶ��һ�¾��ϴ������ļ��ᱻ�ϴ��ܶ��
			//������Ƕ��ڱ��޸��ļ����ϴ�Ӧ���ټ�һ������
			//һ��ʱ��û�б��޸Ĺ�  �޸�ʱ��������3��
			FileUtil f(filename);
			//��ǰʱ��-����޸�ʱ��
			if (time(NULL) - f.LastModifyTime() < 3)
			{
				//�޸�ʱ����С��3 ��Ϊ�ļ������޸��У����ϴ�
				return false;
			}
			std::cout << filename << " need upload "<<std::endl;
			return true;

		}
		//�ͻ���������߼��ϲ� ����ģ��
		bool RunModule()
		{
			while (1)
			{
				//1. ����ָ���ļ��У���ȡ�����ļ���Ϣ
				FileUtil f(_back_dir);
				std::vector<std::string> arr;
				f.GetDirectory(arr);
				//2. ��һ�ж��ļ��Ƿ���Ҫ�ϴ�
				for (auto& a : arr)
				{
					if (JudgeUpload(a) == false)
					{
						continue;
					}
					if (Upload(a) == true)//3. ��Ҫ���ݵ��ļ������ϴ�����
					{
						//����ļ��ϴ��ɹ��� �����ļ�������Ϣ
						//�����ݹ���ģ������ļ����ƺ�Ψһ��ʶ
						_data->Insert(a, GetFileIdentifier(a));
						std::cout << a << " upload sucess" << std::endl;
					}
				}
				//�ȴ�һ��
				Sleep(1);
			}
		}
	private:
		std::string _back_dir;//����ļ�
		DataManager* _data;//���ݹ���
	};
}


