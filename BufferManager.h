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
using namespace std;

#define BLOCK_LEN 4096 //4K 
#define BLOCK_FILE_ACTIVE 5
#define BLOCK_MAX 40
typedef char Block[BLOCK_LEN];
struct blockInfo
{
	int blockNum; 			//块号
	bool dirtyBlock;		//脏位
	blockInfo *next;		//下一块
	struct fileInfo *file;	//文件头
	int charNum;			//块内已用字节数
	char *cBlock;			//块内信息
	int iTime;				//LRU时间
	int lock;				//锁存位
};
struct fileInfo
{
    int type;				//文件类型
    string fileName;		//文件名
    int FileBlockNum;		//文件有的块数
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
	blockInfo *findBlock(string DB_Name);
	void replce(fileInfo *m_fileInfo,blockInfo *m_blockInfo);
	fileInfo *get_file_info(string DB_Name,string File_Name,int m_fileType);
	void closeFile(string DB_Name,string File_Name,int m_fileType);
	void writeBlock(string DB_Name,blockInfo* block);
	blockInfo *readBlock(FILE *fp, int blockNum);
	//void Get_Table_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);
	//void Get_Index_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);
public:
	BufferManager(){blockCount=0;fileCount=0;BlockHandle=NULL;FileHandle=NULL;};
	~BufferManager(){if(fileCount!=0) cout<<"DB not closed yet! Bad manner!"<<endl;};

	static blockInfo * get_file_block(string DB_Name,string Table_Name,int fileType,int blockNum);//根据数据库名、文件名、文件类型、块号找到该块在内存中的位置。
	static void quitProg(string DB_Name);//根据数据库名，关闭打开的所有文件，并且释放所有块
	static void DeleteFile(string DB_Name, string FileName);//根据数据库名，文件名删除文件
	static void CreateFile(string DB_Name,string File_Name);//根据数据库名，文件名创建文件
	static bool HasFile(string DB_Name, string File_Name);//根据数据库名，文件名判断该文件是否存在
    static int getBlockNum(string DB_Name,string File_Name);//获得改文件的块数
    //static Write_Table_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);
    //static Write_Index_Info(string DB_Name,string File_Name,int &recordAmount,int &recordLength);

};



#endif /* defined(__DBMSSev__BufferManager__) */
