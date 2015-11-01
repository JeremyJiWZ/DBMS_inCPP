//
//  RecordManager.cpp
//  DB_Designer 
//
//  Created by 赵冠淳 on 15/10/30.
//  Copyright (c) 2015年 zgc All rights reserved.
//


#include <iostream>
#include <string>
#include <algorithm>  
#include "RecordManager.h" 
using namespace std;

#define DATAFILE 0
#define INT 0;
#define CHAR 1;
#define FLOAT 2;


void RecordManager::WriteInt(char* location,int x)
{
	memcpy(location,&x,4);
}

int RecordManager::ReadInt(char* location)
{	
	int x;
	memcpy(&x,location,4);
	return x;
}

string RecordManager::ReadString(char* location,int amount)
{
	string s;
	for(int i=0;i<amount;i++)
		s = s + location[i];
	return s;
}

float RecordManager::ReadFloat(char* location)
{	
	float x;
	memcpy(&x,location,4);
	return x;
}

void RecordManager::DropTable(string DB_name,string table_name)
{
	BufferManager::DeleteFile(string DB_name,string table_name,0);
} 

bool RecordManager::DataExist(string DB_name,string table_name)
{
	return BufferManager::HasFile(string DB_name,string table_name,0);	
}

void RecordManager::CreateTable(string DB_name,string table_name,int rl)
{
	BufferManager::CreateFile(string DB_name,string table_name,0);
	
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1;
	WriteInt(block_zero_info->cBlock,4096/rl);
	WriteInt(block_zero_info->cBlock+4,0);
	WriteInt(block_zero_info->cBlock+8,0);
	block_zero_info->lock=0;
}

string RecordManager::cstr_to_string( char* cstr)
{
    string s = cstr; // 利用重载的赋值完成。
    return s;
}

bool RecordManager::match(char* record,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum)
{
	bool result=1;
	Value ValidValue[CondNum];
	for(int i=0;i<tableHead.attrAmount;i++)
	{
		for(int j=0;j<CondNum;j++)
		{
			if(cstr_to_string(tableAttr[i].attrName)==Value[j].ValueName)
			{
				switch(tableAttr[i].type)
				{
					case INT:
						ValidValue[j].setInt(ReadInt(record));
						break;
					case CHAR:
						ValidValue[j].setString(ReadString(record,tableAttr[i].amount));
						break;
					case FLOAT:
						ValidValue[j].setFloat(ReadFloat(record));
						break;
				}
			}		
		}
		if(tableAttr[i].type == CHAR)
			record = record + tableAttr[i].amount;
		else record = record + 4;
	}
	
	for(int i=0;i<CondNum;i++)
	{
		switch (cond[i]) 
		{
	        case EQUAL:
	            if (!ValidValue[i].isEqualTo(attributeValue[i]))
	                result = 0;
	            break;
	        case NOTEQUAL:
				if (!ValidValue[i].isNotEqualTo(attributeValue[i]))
	                result = 0;
	            break;    
	        case LESS:
	            if (!ValidValue[i].isLessThan(attributeValue[i]))
	                result = 0;
	            break;
	        case LESSEQUAL:
	            if (!ValidValue[i].isLessEqualTo(attributeValue[i]))
	                result = 0;
	            break;
	        case GREAT:
	            if (!ValidValue[i].isGreatThan(attributeValue[i]))
	                result = 0;
	            break;
	        case GREATEQUAL:
	            if (!ValidValue[i].isGreatEqualTo(attributeValue[i]))
	                result = 0;
	            break;       	
	        default:
	            break;
    	}	
	}
//    printf("Match Failed\n");
    return result;
}

void RecordManager::PrintRecord(char* recordinfo,struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	for(int i=0;i<tableHead.attrAmount;i++)
	{
		switch(tableAttr[i].type)
		{
			case INT:
				cout << ReadInt(record)) << '\t';
				record = record + 4;
				break;
			case CHAR:
				cout << ReadString(record,tableAttr[i].amount) << '\t';
				record = record + tableAttr[i].amount;
				break;
			case FLOAT:
				cout << ReadFloat(record) << '\t';
				break;
		}				
	}
}

void RecordManager::PrintAttr(struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	for(int i=0;i<tableHead.attrAmount;i++)
	{
		printf("%s\t",tableAttr[i].attrName);
	}
}

//0号block的前0~3个字节储存”每个块中可以储存的最大条目数“
//第4~7个字节储存”该表的最大偏移量+1”，即最后一条的offset+1
//第8~11个字节储存”该表的空条目数（删除条目时不会重排列各条目的偏移量）“
//之后每四个字节储存一条空条目的偏移量 

void SeletRecord_WithoutIndex(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,Value attributeValue,Condition cond,int CondNum)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	block_zero_info->lock=0;
	
	PrintAttr(tableHead,tableAttr);	
	if(offset_max==0) return;
	
	for(block_no=1;block_no<(offset_max-1)/br+1;block_no++)
	{
		block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
		block_info->lock=1;
		for(int i=0;i<br*x.rl;i=i+x.rl)
		{
			if(match(block_info->cBlock[i],tableHead,tableAttr,attributeValue,cond,CondNum))
			{
				PrintRecord(block_info->cBlock+i,tableHead,tableAttr);
			}
		}
		block_info->lock=0;
	}
	
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);//最后一个块的扫描
	block_info->lock=1;
	for(int i=0;i<=((offset_max-1)%br)*x.rl;i=i+x.rl)
	{
		if(match(block_info->cBlock[i],x,y))
		{
			PrintRecord(block_info->cBlock+i,x);
		}
	}
	block_info->lock=0;
	
}

bool RecordManager::SortByM1( const int& a, const int& b)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{  
    return a < b;//升序排列  
}  

void RecordManager::SelectRecord_WithIndex(string& DB_name,string& table_name,vector<unsigned int>& offset,attr_info& x)
{
	block_no=0;
	
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	block_zero_info->lock=0;
	
	sort(offset.begin(),offset.end(),SortByM1);  //对offset排序 	
	PrintAttr(x);

	for(int i=0;i<offset.size();i++)
	{
		if(offset.at(i)>=block_no*br)
		{
			block_info->lock=0;
			block_no = (offset.at(i)/x.br)+1;
			block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
			block_info->lock=1;
		}
		PrintRecord(block_info->cBlock+(offset.at(i)%x.br)*x.rl,x);
	}
	block_info->lock=0;
}

void RecordManager::InsertRecord(char* record,char* insert_info,int rl)
{
	for(int i=0;i<rl;i++)
	{
		record[i] = insert_info[i];
	}
}

unsigned int RecordManager::InsertRecord(string& DB_name,string& table_name,attr_info& x,char* insert_info)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1; 
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	DeletedRecordNum = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	
	unsigned int offset;
	if(DeletedRecordNum!=0)
	{
		offset = (unsigned int)ReadInt(block_zero_info->cBlock+8+DeletedRecordNum*4);		
	}
	else
	{
		offset = offset_max;
	}
	
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,offset/br+1);
	block_info->lock=1;
	InsertRecord(block_info->cBlock+(offset%br),insert_info,x.rl);
	block_info->lock=0;
	block_info->dirtyBlock=1;
	
	if(DeletedRecordNum!=0)
	{
		WriteInt(block_zero_info->cBlock+8+DeletedRecordNum*4,0);
		WriteInt(block_zero_info->cBlock+8,DeletedRecordNum-1);			
	}
	else
	{
		WriteInt(block_zero_info->cBlock+4,offset_max+1);
	}
	block_zero_info->lock=0;
		
	printf("Insert Operation Succeed\n");
	return offset;
}

void RecordManager::DeleteRecord(char* record,int rl)
{
	for(int i=0;i<rl;i++)
	{
		record[i] = 0;
	}
}

void RecordManager::DeleteRecord_WithIndex(string& DB_name,string& table_name,unsigned int offset,attr_info& x)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1; 
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	DeletedRecordNum = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	
	block_no=offset/br+1;
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
	block_info->lock=1;
	DeleteRecord(block_info->cBlock+(offset%br)*x.rl,x.rl);
	block_info->lock=0;
	block_info->dirtyBlock=1;
	
	WriteInt(block_zero_info->cBlock+12+DeletedRecordNum*4,offset);
	WriteInt(block_zero_info->cBlock+8,DeletedRecordNum+1);	
	block_zero_info->lock=0; 
	
	printf("Delete Operation Succeed\n");
}

void RecordManager::DeleteRecord_WithoutIndex(string& DB_name,string& table_name,attr_info& x,condition& y)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	DeletedRecordNum = (unsigned int)ReadInt(block_zero_info->cBlock+8);
			
	if(offset_max==0) return;
	
	for(block_no=1;block_no<(offset_max-1)/br+1;block_no++)
	{
		block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
		block_info->lock=1;
		for(int i=0;i<br*x.rl;i=i+x.rl)
		{
			if(match(block_info->cBlock[i],x,y))
			{
				DeleteRecord(block_info->cBlock+i,x.rl);
				block_info->dirtyBlock=1;
				
				WriteInt(block_zero_info->cBlock+12+DeletedRecordNum*4,(block_no-1)*br+(i%x.rl));
				DeletedRecordNum++;
			}
		}
		block_info->lock=0;
	}
	
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);//最后一个块的扫描
	block_info->lock=1;
	for(int i=0;i<=((offset_max-1)%br)*x.rl;i=i+x.rl)
	{
		if(match(block_info->cBlock[i],x,y))
		{
			DeleteRecord(block_info->cBlock+i,x.rl);
			block_info->dirtyBlock=1;
			
			WriteInt(block_zero_info->cBlock+12+DeletedRecordNum*4,(block_no-1)*br+(i%x.rl));
			DeletedRecordNum++;
		}
	}
	block_info->lock=0;
	
	WriteInt(block_zero_info->cBlock+8,DeletedRecordNum);
	block_zero_info->lock=1;
	printf("Delete Operation Succeed\n");
}
