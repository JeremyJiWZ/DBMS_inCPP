//
//  RecordManager.h 
//  DB_Designer 
//
//  Created by ÕÔ¹Ú´¾ on 15/10/30.
//  Copyright (c) 2015Äê zgc All rights reserved.
//
#ifndef RECORDERMANGER_H
#define RECORDERMANGER_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>  
#include "BufferManager.h" 
#include "Value.h"
#include "Condition.h" 
using namespace std; 

#define MAX_NAME_LENGTH 64

/* 
struct TableHead{
	char valid;
	char tableName[MAX_NAME_LENGTH];//64
	int attrAmount;//4
	int recordAmount;//4
};

struct TableAttr{
	char attrName[MAX_NAME_LENGTH];//64
	char type;//1
	int amount;//max 255, 4bytes
	char unique;//1
	char primary;//1
	char index;//1
	char indexName[MAX_NAME_LENGTH];//64
};
*/

class RecordManager
{
private:
	int block_no;
	blockInfo* block_info;
	blockInfo* block_zero_info;
	unsigned int br;
	unsigned int rl;
	unsigned int offset_max;
	unsigned int DeletedRecordNum;
	
	bool match(char* record,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int& CondNum);//ok
	void PrintRecord(char* record,struct TableHead& tableHead, struct TableAttr* tableAttr);//ok
	void PrintAttr(struct TableHead& tableHead, struct TableAttr* tableAttr);//ok
	void Insert(char* record,char* insert_info,int rl);//ok
	void DeleteRecord(char* record,int rl);//ok
	void WriteInt(char* location,int x);//ok
	int ReadInt(char* location);//ok
	string ReadString(char* location,int amount);//ok
	float ReadFloat(char* location);//ok
	string cstr_to_string( char* ctr);//ok 
	bool Record_IsEmpty(char* record,int rl);//ok
	 
public:
	RecordManager(){};//ok
	~RecordManager(){};//ok
	void DropTable(string DB_name,string table_name);//ok
	bool DataExist(string DB_name,string table_name);//bug
	void CreateTable(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr);//ok
	void SelectRecord_WithoutIndex(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum);//ok
	void SelectRecord_WithIndex(string DB_name,string table_name,vector<unsigned int>& offset,struct TableHead& tableHead, struct TableAttr* tableAttr);//ok
	unsigned int InsertRecord(string& DB_name,string& table_name,char* insert_info); //ok
	void DeleteRecord_WithIndex(string DB_name,string table_name,vector<unsigned int>& offset);//ok 
	void DeleteRecord_WithoutIndex(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum);//ok
	
};

#include "RecordManager.cpp" 
#endif
