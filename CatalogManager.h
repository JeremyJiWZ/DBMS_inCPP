#include <stdio.h>
#include <string>

using namespace std;

#ifndef _Catalob_Manage_H_
#define _Catalob_Manage_H_

#define INT 0
#define CHAR 1
#define FLOAT 2
#define NAME_MAX_LENGTH 64

extern class CatalogManager *catl;

class CatalogManager
{
private:
	
	CatalogManager();//˽�й���
	~CatalogManager();//˽������
	
	static CatalogManager catl;//CatalogManager����
	static const string mErrMsg[];//������Ϣ��Ҫ��ʼ��
	string dbName;//���ݿ�����
	int errNum;//�������
	
public:
	
	static CatalogManager *GetInstance();//���CatalogManager����
	
	int CreateDatabase(const string & DBName);//�������ݿ⣺�������ݿ�Ŀ¼��cat�ļ�������0�������ɹ�
	int DropDatabase(const string & DBName);//ɾ�����ݿ⣺ɾ�����ݿ�Ŀ¼��cat�ļ�������0����ɾ���ɹ�
	int UseDatabase(const string & DBName);	//ʹ�����ݿ�
	
	int CreateTable(const string & tableName);//�����������ݿ�Ŀ¼��cat�ļ����������
	int DropTable(const string & tableName);//ɾ����ɾ�����ݿ�Ŀ¼��cat�ļ��������
	
	int GetAttrAmount(const string & tableName);//���ĳ������������
	int GetAttrNameWithNum(const string & tableName, int num, const string & attrName);//�����������û��
	int IsAttrInTable(const string & tableName, const string & attrName);//���ڷ���0
	int WhatAttrType(const string & tableName, const string & attrName, int & type);//���ص����ͱ�����type��
	int IsAttrType(const string & tableName, const string & attrName, int type);//��ͬ����0
	
	int CreateIndex(const string & tableName, const string & attrName, const string & indexName);
	int DropIndex(const string & indexName);
	int GetIndexAmount(const string & tableName);
	int IsAttrHasIndex(const string & tableName, const string & attrName);
	int GetIndexName(const string & tableName, int num, string & indexName);//���ص�index���ֽ����indexName��
	int GetIndexName(const string & tableName, const string & attrName, string & indexName);//���ص�index������indexName��
	int IsIndexInTable(const string & tableName, const string & indexName);
	
	int Insert(const string & tableName, int type[]);//�жϲ�������������Ƿ�ƥ�䣬ע��ᵼ�¼�¼��������1
	//int Delete(const string & tableName);�����������û��
	//int Select(const string & tableName);ͬ����û��
	int GetRecordAmount(const string & tableName);
	int AddRecordAmount(const string & tableName, int amount = 1);
	int SubRecordAmount(const string & tableName, int amount);
	
	int GetErrNum();//��ô������
	string & GetErrMsg();//ͨ�����һ����������Ĵ�����뷵�ش�����Ϣ
	string & GetErrMsg(int errNum);//ͨ��������뷵�ش�����Ϣ
	
	int Quit();//�˳����ݿ�ʱһ��Ҫ���ã�����catlʵ��
	
	static const int INT_TYPE;   // = 0;
	static const int CHAR_TYPE;  // = 1;
	static const int FLOAT_TYPE; // = 2;
	
	string output;//����Ĵ�����Ϣ������
};

#endif