//
//  RecordManger.cpp
//  DB_Designer 
//
//  Created by 赵冠淳 on 15/10/30.
//  Copyright (c) 2015年 zgc All rights reserved.
//


#include <iostream>
#include <string>
#include <algorithm>  
#include "RecordManger.h" 
using namespace std;


void RecordManger::DropTable(string& DB_name,string& table_name)
{
	BufferManger::DeleteFile(string& DB_name,string& table_name);
} 

bool RecordManger::DataExist(string& DB_name,string& table_name)
{
	return BufferManger::HasFile(string& DB_name,string& table_name);	
}

void RecordManger::CreateTable(string& DB_name,string& table_name)
{
	BufferManger::CreateFile(string DB_name,string table_name);
}

bool RecordManger::match(char* record,attr_info& x,condition& y)
{
	
}

void RecordManger::PrintRecord(char* recordinfo,attr_info& x)
{
	
}

void RecordManger::PrintAttr(attr_info& x)
{
	
}

void RecordManger::SeletRecord_WithoutIndex(string& DB_name,string& table_name,attr_info& x,condition& y)
{
	blockInfo* block_info;
	PrintAttr(x);		
	block_info = BufferManger::get_file_block(DB_name,table_name,1,0);
	while(block_info!=NULL)
	{
		block_info->lock=1;
		for(int i=0;i<block_info->charNum-1;i=i+x.rl)
		{
			if(match(block_info->cBlock[i],x,y))
			{
				PrintRecord(block_info->cBlock+i,x);
			}		
		}
		block_info->lock=0;
		block_info=block_info->next;
	}
}

bool RecordManger::SortByM1( const int& a, const int& b)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{  
    return a < b;//升序排列  
}  

void RecordManger::SelectRecord_WithIndex(string& DB_name,string& table_name,vector<int>& offset,attr_info& x)
{
	int block_no=0;
	blockInfo* block_info;
	sort(offset.begin(),offset.end(),SortByM1);  //对offset排序 	
	PrintAttr(x);
	block_info = BufferManger::get_file_block(DB_name,table_name,1,block_no);
	block_info->lock=1;
	for(int i=0;i<offset.size();i++)
	{
		if(offset.at(i)>=(block_no+1)*x.br)
		{
			block_info->lock=0;
			block_no = offset[i]/x.br;
			block_info = BufferManger::get_file_block(DB_name,table_name,1,block_no);
			block_info->lock=1;
		}
		PrintRecord(block_info->cBlock+(offset.at(i)%x.br)*x.rl,x);
	}
	block_info->lock=0;
}

void RecordManger::InsertRecord(char* record,char* insert_info,int rl)
{
	for(int i=0;i<rl;i++)
	{
		record[i] = insert_info[i];
	}
}

int RecordManger::InsertRecord(string& DB_name,string& table_name,attr_info& x,char* insert_info)
{
	blockInfo* block_info;
	if(x.nr%x.br!=0)	block_info = BufferManger::get_file_block(DB_name,table_name,1,bn-1);
	else	block_info = BufferManger::get_file_block(DB_name,table_name,1,bn);
	block_info->lock=1;
	InsertRecord(block_info->cBlock+block_info->charNum,insert_info,x.rl);
	block_info->lock=0;
	block_info->dirtyBlock=1;
	block_info->charNum = block_info->charNum + x.rl;
	return x.nr + 1;
}

void RecordManger::DeleteRecord(char* record,int rl)
{
	for(int i=0;i<rl;i++)
	{
		record[i] = 0;
	}
}

void RecordManger::DeleteRecord_WithIndex(string& DB_name,string& table_name,int offset,attr_info& x)
{
	int block_no=offset/x.br;
	blockInfo* block_info;
	block_info = BufferManger::get_file_block(DB_name,table_name,1,block_no);
	block_info->lock=1;
	DeleteRecord(block_info->cBlock+(offset%x.br)*x.rl,x.rl);
	block_info->lock=0;
	block_info->dirtyBlock=1;
}

void RecordManger::DeleteRecord_WithoutIndex(string& DB_name,string& table_name,attr_info& x,condition& y)
{
	blockInfo* block_info;
	block_info = BufferManger::get_file_block(DB_name,table_name,1,0);
	while(block_info!=NULL)
	{
		block_info->lock=1;
		for(int i=0;i<block_info->charNum-1;i=i+x.rl)
		{
			if(match(block_info->cBlock[i],x,y))
			{
				DeleteRecord(block_info->cBlock+i,x.rl);
				block_info->dirtyBlock=1;
			}	
		}
		block_info->lock=0;
		block_info=block_info->next;
	}
}
