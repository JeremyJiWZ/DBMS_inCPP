//
//  RecordManger.h 
//  DB_Designer 
//
//  Created by �Թڴ� on 15/10/30.
//  Copyright (c) 2015�� zgc All rights reserved.
//
#ifndef RECORDERMANGER_H
#define RECORDERMANGER_H

#include <iostream>
#include <vector>
#include "BufferManger.h" 
#include "BufferManger.cpp" 
using namespace std; 

class RecordManger
{
private:
	vector<short> DeleteRecord_Offset;
	RecordManger();
	~RecordManger(); 
	bool match(char* record,attr_info& x,condition& y);
	void PrintRecord(char* recordinfo,attr_info& x);
	void PrintAttr(attr_info& x);
	bool SortByM1( const int& a, const int& b);
	void InsertRecord(char* record,char* insert_info,int rl);
	void DeleteRecord(char* record,int rl);
	 
public:
	void DropTable(string DB_name,string table_name);
	bool DataExist(string DB_name,string table_name);
	void CreateTable(string DB_name,string table_name);
	void SeletRecord_WithoutIndex(string DB_name,string table_name,condition x);//condition��Ҫ����where����������Ϣ 
	void SelectRecord_WithIndex(string DB_name,string table_name,vector<int>& offset,attr_info x);//attr_info��Ӧ�ð���������Ϣ��record�ĳ�����Ϣ��ÿ���������ɵ�record������ 
	void DeleteRecord_WithIndex(string DB_name,string table_name,int offset);
	void DeleteRecord_WithoutIndex(string DB_name,string table_name,condition x);
	
}


#endif
