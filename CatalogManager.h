#define linux//定义Linux环境

#ifdef linux
#include <unistd.h>
#else
#include<windows.h>
#include<io.h>
#endif

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#ifndef _Catalob_Manage_H_
#define _Catalob_Manage_H_

#define CTG_INT 0
#define CTG_CHAR 1
#define CTG_FLOAT 2
#define MAX_NAME_LENGTH 64 //实际上只有63个字符，因为还要有'\0'

extern class CatalogManager *catl;
struct TableHead{
    char valid;
    char tableName[64];//64
    int attrAmount;//4
    int recordAmount;//4
};

struct TableAttr{
    char attrName[64];//64
    char type;//1
    int amount;//max 255, 4bytes
    char unique;//1
    char primary;//1
    char index;//1
    char indexName[64];//64
};
class CatalogManager
{
private:
	
	CatalogManager();//私有构造
	~CatalogManager();//私有析构
	int GetTable(int Num);
	int PutTable(int Num);
	int CheckUseDB();//返回0 为 没有use database
	int GetTableInit();
	int GetTableNext();
	int GetTableByName(const string & tableName);
	
	static const string mErrMsg[];//错误信息，要初始化
	string mDBName;//数据库名字
	fstream mFile;
	int mErrNum;//错误代码
	struct TableHead mTableHead;
	struct TableAttr mTableAttr[32];
	
	int mGetAmount;
	int mGetNum;
	
	char mcName[MAX_NAME_LENGTH+1];//char name <-- string name
	char mcBuf[128];//
	string mstrBuf;
	int miBuf;
	
public:
    
	static CatalogManager *GetInstance();//获得CatalogManager对象
	
	int CreateDatabase(const string & DBName);//创建数据库：创建数据库目录和cat文件，返回0代表创建成功
	int DropDatabase(const string & DBName);//删除数据库：删除数据库目录和cat文件，返回0代表删除成功
	int UseDatabase(const string & DBName);	//使用数据库
	
	int CreateTable(const struct TableHead & tableHead, const struct TableAttr tableAttr[], int attrAmount = 0);//创建表：在数据库目录下cat文件中添加数据
	int DropTable(const string & tableName);//删除表：删除数据库目录下cat文件相关数据
	
	int GetAttrAmount(const string & tableName, int & attrAmount);//获得某个表属性数量
	int GetAttrNameWithNum(const string & tableName, int num, string & attrName);//这个函数好像没用
	int IsAttrInTable(const string & tableName, const string & attrName, int & ret);//存在返ret为1，不存在ret为0
	int GetAttrType(const string & tableName, const string & attrName, int & type);//返回的类型保存在type中
	int GetAttrType(const string & tableName, int num, int & type);//返回的类型保存在type中
	//int IsAttrType(const string & tableName, const string & attrName, int type, int & ret);//相同返回0
	//int GetAttrByte(const string & tableName, int num, int & bytes);
	//int GetAllAttrByte(const string & tableName, int & bytes);
	int GetTableStruct(const string & tableName, struct TableHead & tableHead, struct TableAttr tableAttr[]);
	
	int CreateIndex(const string & tableName, const string & attrName, const string & indexName);
	int DropIndex(const string & indexName);
	//int GetIndexAmount(const string & tableName);
	int DoesAttrHaveIndex(const string & tableName, const string & attrName, int & ret);
	int DoesAttrHaveIndex(const string & tableName, int num, int & ret);
	int GetIndexName(const string & tableName, int num, string & indexName);//返回的index名字结果在indexName中
	int GetIndexName(const string & tableName, const string & attrName, string & indexName);//返回的index名字在indexName中
	//int IsIndexInTable(const string & tableName, const string & indexName);
	
	int Insert(const string & tableName, int amount, int type[]);//判断插入的数据类型是否匹配，注意不会导致记录的数量改变，若要增加记录数量请使用AddRecordAmount方法
	//int Delete(const string & tableName);这个函数好像没用
	//int Select(const string & tableName);同好像没用
	int GetRecordAmount(const string & tableName, int & amount);
	int AddRecordAmount(const string & tableName, int amount = 1);
	int SubRecordAmount(const string & tableName, int amount);
	
	int GetErrNum();//获得错误代码
	string & GetErrMsg();//通过最近一条错误操作的错误代码返回错误信息
	string & GetErrMsg(int errNum);//通过错误代码返回错误信息
	
	int Quit();//退出数据库时一定要调用，析构catl实例
	
	static const int INT;   // = 0;
	static const int CHAR;  // = 1;
	static const int FLOAT; // = 2;
	static const int MTLEN; // max table length = MAX_NAME_LENGTH*65+16
	
	string output;//输出的错误信息都在这
};

#endif
