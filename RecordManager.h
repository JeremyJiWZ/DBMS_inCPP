//
//  RecordManager.h 
//  DB_Designer 
//
//  Created by ÕÔ¹Ú´¾ on 15/10/30.
//  Copyright (c) 2015Äê zgc All rights reserved.
//
#ifndef RECORDERMANGER_H
#define RECORDERMANGER_H

#include <iostream>
#include <vector>
#include "CatelogManager.h"
#include "BufferManager.h" 
#include "value.hpp"
using namespace std; 

typedef enum {
    EQUAL,
    LESS,
    LESSEQUAL,
    GREAT,
    GREATEQUAL,
    NOTEQUAL
}
    


class RecordManager
{
private:
	int block_no;
	blockInfo* block_info;
	blockInfo* block_zero_info;
	unsigned int br;
	unsigned int offset_max;
	unsigned int DeletedRecordNum;
	
	bool match(char* record,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum)
	void PrintRecord(char* recordinfo,struct TableHead& tableHead, struct TableAttr* tableAttr)
	void PrintAttr(struct TableHead& tableHead, struct TableAttr* tableAttr);
	bool SortByM1( const int& a, const int& b);
	void InsertRecord(char* record,char* insert_info,int rl);
	void DeleteRecord(char* record,int rl);
	void WriteInt(char* location,int x);
	int ReadInt(char* location);
	string ReadString(char* location,int amount);
	float ReadFloat(char* location);
	string cstr_to_string( char* ctr)
	 
public:
	RecordManager();
	~RecordManger();
	void DropTable(string DB_name,string table_name);
	bool DataExist(string DB_name,string table_name);
	void CreateTable(string DB_name,string table_name,int rl);
	void SeletRecord_WithoutIndex(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum);
	void SelectRecord_WithIndex(string DB_name,string table_name,vector<unsigned int>& offset,struct TableHead& tableHead, struct TableAttr* tableAttr);
	unsigned int InsertRecord(string& DB_name,string& table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,char* insert_info); 
	void DeleteRecord_WithIndex(string DB_name,string table_name,unsigned int offset);
	void DeleteRecord_WithoutIndex(string DB_name,string table_name,Value attributeValue, Condition cond);
	
};


#endif
