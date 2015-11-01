//
//  BufferManager.h
//  DBMSSev
//
//  Created by mr.ji on 15/10/22.
//  Copyright (c) 2015年 mr.ji. All rights reserved.
//

#ifndef __DBMSSev__BufferManager__
#define __DBMSSev__BufferManager__
#include<string>
#include<fstream>
#include<iostream>
#include<stdlib.h>
using namespace std;

#define BLOCK_LEN 4096 //4K 
#define BLOCK_FILE_ACTIVE 5
#define BLOCK_MAX 40
typedef char Block[BLOCK_LEN];
struct blockInfo
{
	int blockNum; 			//块号,从0开始计算
	bool dirtyBlock;		//脏位
	blockInfo *next;		//下一块
	struct fileInfo *file;	//文件头
	int charNum;			//块内已用字节数,写块时需要重新给其赋值
	char *cBlock;			//块内信息
	int iTime;				//LRU时间
	int lock;				//锁存位
};
struct fileInfo
{
    int type;				//文件类型,1表示index，0表示数据
    string fileName;		//文件名
    int FileBlockNum;		//文件有的块数
    fstream fp;             //文件指针，进行文件操作
    // int recordAmount;
    // int freeNum;
    // int recordLength;
    bool busy; 				//表示改文件中是否有块正在使用(locked)
    fileInfo *Next; 		//下一个文件
    blockInfo *firstBlock;	//第一个块
}; 
class BufferManager
{
private:
	static int blockCount;			//块数目
	static int fileCount;			//文件数
	static blockInfo *BlockHandle; 	//垃圾链表
	static fileInfo *FileHandle; 	//文件链表
    static void addTime(string DB_Name);
	static blockInfo *findBlock(string DB_Name);
	static void replce(fileInfo *m_fileInfo,blockInfo *m_blockInfo);
	static fileInfo *get_file_info(string DB_Name,string File_Name,int m_fileType);
	static void closeFile(string DB_Name,string File_Name,int type);
	static void writeBlock(string DB_Name,blockInfo* block);
	static blockInfo *readBlock(string DB_Name,fileInfo* file, int blockNum);
	//void Get_Table_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);
	//void Get_Index_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);
public:
	BufferManager(){blockCount=0;fileCount=0;BlockHandle=NULL;FileHandle=NULL;};
	~BufferManager(){if(fileCount!=0) cout<<"DB not closed yet! Bad manner!"<<endl;};
    //接口函数：
    //根据数据库名、文件名、文件类型、块号找到该块在内存中的位置。1表示index，0表示data
	static blockInfo * get_file_block(string DB_Name,string Table_Name,int fileType,int blockNum);
    //根据数据库名，关闭打开的所有文件，并且释放所有块
	static void quitProg(string DB_Name);
    //根据数据库名，文件名删除文件,type=1表示index，0表示data
	static void DeleteFile(string DB_Name, string FileName,int type);
    //根据数据库名，文件名创建文件,type=1表示index，0表示data
    static void CreateFile(string DB_Name,string File_Name,int type);
    //根据数据库名，文件名判断该文件是否存在,type=1表示index，0表示data
    static bool HasFile(string DB_Name, string File_Name,int type);
    //获得该文件的块数，,type=1表示index，0表示data，计数规则从0开始
    static int getBlockNum(string DB_Name,string File_Name,int type);
    //运行use DataBase 时使用，创建相应文件夹以及目录
    static void UseDB(string DB_Name);
    //static Write_Table_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);
    //static Write_Index_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);

};

#endif /* defined(__DBMSSev__BufferManager__) */
