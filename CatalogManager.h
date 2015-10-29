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
	
	CatalogManager();//私有构造
	~CatalogManager();//私有析构
	
	static CatalogManager catl;//CatalogManager对象
	static const string mErrMsg[];//错误信息，要初始化
	string dbName;//数据库名字
	int errNum;//错误代码
	
public:
	
	static CatalogManager *GetInstance();//获得CatalogManager对象
	
	int CreateDatabase(const string & DBName);//创建数据库：创建数据库目录和cat文件，返回0代表创建成功
	int DropDatabase(const string & DBName);//删除数据库：删除数据库目录和cat文件，返回0代表删除成功
	int UseDatabase(const string & DBName);	//使用数据库
	
	int CreateTable(const string & tableName);//创建表：在数据库目录下cat文件中添加数据
	int DropTable(const string & tableName);//删除表：删除数据库目录下cat文件相关数据
	
	int GetAttrAmount(const string & tableName);//获得某个表属性数量
	int GetAttrNameWithNum(const string & tableName, int num, const string & attrName);//这个函数好像没用
	int IsAttrInTable(const string & tableName, const string & attrName);//存在返回0
	int WhatAttrType(const string & tableName, const string & attrName, int & type);//返回的类型保存在type中
	int IsAttrType(const string & tableName, const string & attrName, int type);//相同返回0
	
	int CreateIndex(const string & tableName, const string & attrName, const string & indexName);
	int DropIndex(const string & indexName);
	int GetIndexAmount(const string & tableName);
	int IsAttrHasIndex(const string & tableName, const string & attrName);
	int GetIndexName(const string & tableName, int num, string & indexName);//返回的index名字结果在indexName中
	int GetIndexName(const string & tableName, const string & attrName, string & indexName);//返回的index名字在indexName中
	int IsIndexInTable(const string & tableName, const string & indexName);
	
	int Insert(const string & tableName, int type[]);//判断插入的数据类型是否匹配，注意会导致记录的数量加1
	//int Delete(const string & tableName);这个函数好像没用
	//int Select(const string & tableName);同好像没用
	int GetRecordAmount(const string & tableName);
	int AddRecordAmount(const string & tableName, int amount = 1);
	int SubRecordAmount(const string & tableName, int amount);
	
	int GetErrNum();//获得错误代码
	string & GetErrMsg();//通过最近一条错误操作的错误代码返回错误信息
	string & GetErrMsg(int errNum);//通过错误代码返回错误信息
	
	int Quit();//退出数据库时一定要调用，析构catl实例
	
	static const int INT_TYPE;   // = 0;
	static const int CHAR_TYPE;  // = 1;
	static const int FLOAT_TYPE; // = 2;
	
	string output;//输出的错误信息都在这
};

#endif