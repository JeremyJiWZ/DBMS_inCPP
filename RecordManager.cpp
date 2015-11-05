//
//  RecordManager.cpp
//  DB_Designer 
//
//  Created by 赵冠淳 on 15/10/30.
//  Copyright (c) 2015年 zgc All rights reserved.
//

#include "RecordManager.h"
using namespace std;

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
		if(location[i]!=0)
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
	BufferManager::DeleteFile(DB_name,table_name,0);
} 

bool RecordManager::DataExist(string DB_name,string table_name)
{
	return BufferManager::HasFile(DB_name,table_name,0);	
}

void RecordManager::CreateTable(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	BufferManager::CreateFile(DB_name,table_name,0);
	
	if(!DataExist(DB_name,table_name))
	{
		cout << "Create Table Failed\n" ; 
		return;
	}
	int rl = 0;
	for(int i=0;i<tableHead.attrAmount;i++)
	{
		switch(tableAttr[i].type)
		{
			case INT:
				rl = rl + 4;
				break;
			case CHAR:
				rl = rl + tableAttr[i].amount;
				break;
			case FLOAT:
				rl = rl + 4;
				break;
		}	
	}
	
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1;
	WriteInt(block_zero_info->cBlock,4096*1024/rl);
	WriteInt(block_zero_info->cBlock+4,rl);
	WriteInt(block_zero_info->cBlock+8,0);
	WriteInt(block_zero_info->cBlock+12,0);
	block_zero_info->lock=0;
	block_zero_info->dirtyBlock=1; 
}

string RecordManager::cstr_to_string( char* cstr)
{
    string s = cstr; // 利用重载的赋值完成。
    return s;
}

bool RecordManager::match(char* record,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int& CondNum)
{
	bool result=1;
	Value ValidValue[CondNum];
	
	for(int i=0;i<CondNum;i++)
	{
		if(attributeValue[i].ValueName.empty())
		{
			cout << "Match Failed: some values have no name.\n";
			return 0;
		}
	}
	
	if(CondNum==0) result = 0;
	
	for(int i=0;i<tableHead.attrAmount;i++)
	{
		for(int j=0;j<CondNum;j++)
		{
			if(cstr_to_string(tableAttr[i].attrName)==attributeValue[j].ValueName)
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
					default:
						cout << "Match Failed: unknown type.\n";
						return 0;
						break;
				}
				ValidValue[j].setValueName(attributeValue[j].ValueName);
			}		
		}
		if(tableAttr[i].type == CHAR)
			record = record + tableAttr[i].amount;
		else record = record + 4;
	}
	
	for(int i=0;i<CondNum;i++)
	{
		if(ValidValue[i].ValueName.empty())
		{
			cout << "Match Failed: some values don't match.\n";
			return 0;
		}
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
	        	cout << cout << "Match Failed: wrong condition.\n";
	        	return 0;
	            break;
    	}	
	}

    return result;
}

void RecordManager::PrintRecord(char* record,struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	for(int i=0;i<tableHead.attrAmount;i++)
	{
		switch(tableAttr[i].type)
		{
			case INT:
				cout << ReadInt(record);
				record = record + 4;
				break;
			case CHAR:
				cout << ReadString(record,tableAttr[i].amount);
				record = record + tableAttr[i].amount;
				break;
			case FLOAT:
				cout << ReadFloat(record);
				break;
			default:
				cout << "Print Record Failed: unknown type.\n";
				return;
				break;
		}	
		if(i==tableHead.attrAmount-1) cout << '\n';
		else cout << '\t';	
	}
}

void RecordManager::PrintAttr(struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	int i;
	for(i=0;i<tableHead.attrAmount-1;i++)
	{
		printf("%s\t",tableAttr[i].attrName);
	}
	printf("%s\n",tableAttr[i].attrName);
}

bool RecordManager::Record_IsEmpty(char* record,int rl)
{
	for(int i=0;i<rl;i++)
		if(record[i]!=0)
			return 0;
	return 1;
}

//0号block的前0~3个字节储存”每个块中可以储存的最大条目数“
//第4~7个字节储存”每个条目的字节数“ 
//第8~11个字节储存”该表的最大偏移量+1”，即最后一条的offset+1
//第12~15个字节储存”该表的空条目数（删除条目时不会重排列各条目的偏移量）“
//之后每四个字节储存一条空条目的偏移量 

void RecordManager::SelectRecord_WithoutIndex(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock); 
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4); 
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	block_zero_info->lock=0;
	
	PrintAttr(tableHead,tableAttr);	
	if(offset_max==0)  return;
	
	for(block_no=1;block_no<(offset_max-1)/br+1;block_no++)
	{
		block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
		block_info->lock=1;
		for(int i=0;i<br*rl;i=i+rl)
		{
			if(Record_IsEmpty(block_info->cBlock+i,rl))
			{
				continue;
			}
			else if(match(block_info->cBlock+i,tableHead,tableAttr,attributeValue,cond,CondNum))
			{
				PrintRecord(block_info->cBlock+i,tableHead,tableAttr);
			}
		}
		block_info->lock=0;
	}
	
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);//最后一个块的扫描
	block_info->lock=1;
	for(int i=0;i<=((offset_max-1)%br)*rl;i=i+rl)
	{
		if(Record_IsEmpty(block_info->cBlock+i,rl))
		{
			continue;
		}
		else if(match(block_info->cBlock+i,tableHead,tableAttr,attributeValue,cond,CondNum))
		{
			PrintRecord(block_info->cBlock+i,tableHead,tableAttr);
		}
	}
	block_info->lock=0;
	
}

bool SortByM1( const int& a, const int& b)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
{  
    return a < b;//升序排列  
}  

void RecordManager::SelectRecord_WithIndex(string DB_name,string table_name,vector<unsigned int>& offset,struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	block_no=0;
	
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	block_zero_info->lock=0;
	
	sort(offset.begin(),offset.end(),SortByM1);  //对offset排序 	
	if(offset.back()>=offset_max)
	{
		cout << "Select Record Failed: unexisted offset.\n";
		return;
	}	
	
	PrintAttr(tableHead,tableAttr);	

	for(int i=0;i<offset.size();i++)
	{
		if(offset.at(i)>=block_no*br)
		{
			block_info->lock=0;
			block_no = (offset.at(i)/br)+1;
			block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
			block_info->lock=1;
		}
		if(Record_IsEmpty(block_info->cBlock+(offset.at(i)%br)*rl,rl))
		{
			cout << "Select Record Failed: empty record.\n";
		}
		else PrintRecord(block_info->cBlock+(offset.at(i)%br)*rl,tableHead,tableAttr);
	}
	block_info->lock=0;
}

void RecordManager::SelectRecord_All(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock); 
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4); 
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	block_zero_info->lock=0;
	
	PrintAttr(tableHead,tableAttr);	
	if(offset_max==0)  return;
	
	for(block_no=1;block_no<(offset_max-1)/br+1;block_no++)
	{
		block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
		block_info->lock=1;
		for(int i=0;i<br*rl;i=i+rl)
		{
			if(Record_IsEmpty(block_info->cBlock+i,rl))
			{
				continue;
			}
			else
			{
				PrintRecord(block_info->cBlock+i,tableHead,tableAttr);
			}
		}
		block_info->lock=0;
	}
	
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);//最后一个块的扫描
	block_info->lock=1;
	for(int i=0;i<=((offset_max-1)%br)*rl;i=i+rl)
	{
		if(Record_IsEmpty(block_info->cBlock+i,rl))
		{
			continue;
		}
		else
		{
			PrintRecord(block_info->cBlock+i,tableHead,tableAttr);
		}
	}
	block_info->lock=0;
}

void RecordManager::Insert(char* record,char* insert_info,int rl)
{
	for(int i=0;i<rl;i++)
	{
		record[i] = insert_info[i];
	}
}

unsigned int RecordManager::InsertRecord(string& DB_name,string& table_name,char* insert_info)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1; 
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	DeletedRecordNum = (unsigned int)ReadInt(block_zero_info->cBlock+12);
	
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
	Insert(block_info->cBlock+(offset%br)*rl,insert_info,rl);
	block_info->lock=0;
	block_info->dirtyBlock=1;
	
	if(DeletedRecordNum!=0)
	{
		WriteInt(block_zero_info->cBlock+12+DeletedRecordNum*4,0);
		WriteInt(block_zero_info->cBlock+12,DeletedRecordNum-1);			
	}
	else
	{
		WriteInt(block_zero_info->cBlock+8,offset_max+1);
	}
	block_zero_info->lock=0;
	
	block_zero_info->dirtyBlock=1; 
		
	printf("Insert Operation Succeed.\n");
	return offset;
}

void RecordManager::DeleteRecord(char* record,int rl)
{
	for(int i=0;i<rl;i++)
	{
		record[i] = 0;
	}
}

void RecordManager::DeleteRecord_WithIndex(string DB_name,string table_name,vector<unsigned int>& offset)
{
	block_no=0;		
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1; 
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	DeletedRecordNum = (unsigned int)ReadInt(block_zero_info->cBlock+12);
	
	sort(offset.begin(),offset.end(),SortByM1);  //对offset排序 
	if(offset.back()>=offset_max)
	{
		cout << "Delete Record Failed: unexisted offset.\n";
		return;
	}	

	for(int i=0;i<offset.size();i++)
	{
		if(offset.at(i)>=block_no*br)
		{
			block_info->lock=0;
			block_no = (offset.at(i)/br)+1;
			block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
			block_info->lock=1;
		}
		if(Record_IsEmpty(block_info->cBlock+(offset.at(i)%br)*rl,rl))
		{
			cout << "Delete Record Failed: empty record.\n";
		}
		else
		{
			DeleteRecord(block_info->cBlock+(offset.at(i)%br)*rl,rl);
			block_info->dirtyBlock=1;
			WriteInt(block_zero_info->cBlock+16+DeletedRecordNum*4,offset.at(i));
			DeletedRecordNum++;
		}
	}
	block_info->lock=0;
	
	WriteInt(block_zero_info->cBlock+12,DeletedRecordNum);	
	block_zero_info->lock=0; 
	
	block_zero_info->dirtyBlock=1; 
	
	printf("Delete Operation Succeed.\n");
}

void RecordManager::DeleteRecord_WithoutIndex(string DB_name,string table_name,struct TableHead& tableHead, struct TableAttr* tableAttr,Value* attributeValue,Condition* cond,const int CondNum)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	offset_max = (unsigned int)ReadInt(block_zero_info->cBlock+8);
	DeletedRecordNum = (unsigned int)ReadInt(block_zero_info->cBlock+12);
			
	if(offset_max==0) return;
	
	for(block_no=1;block_no<(offset_max-1)/br+1;block_no++)
	{
		block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);
		block_info->lock=1;
		for(int i=0;i<br*rl;i=i+rl)
		{
			if(Record_IsEmpty(block_info->cBlock+i,rl))
			{
				continue;
			}
			else if(match(block_info->cBlock+i,tableHead,tableAttr,attributeValue,cond,CondNum))
			{
				DeleteRecord(block_info->cBlock+i,rl);
				block_info->dirtyBlock=1;
				
				WriteInt(block_zero_info->cBlock+16+DeletedRecordNum*4,(block_no-1)*br+(i%rl));
				DeletedRecordNum++;
			}
		}
		block_info->lock=0;
	}
	
	block_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,block_no);//最后一个块的扫描
	block_info->lock=1;
	for(int i=0;i<=((offset_max-1)%br)*rl;i=i+rl)
	{
		if(Record_IsEmpty(block_info->cBlock+i,rl))
		{
			continue;
		}
		else if(match(block_info->cBlock+i,tableHead,tableAttr,attributeValue,cond,CondNum))
		{
			DeleteRecord(block_info->cBlock+i,rl);
			block_info->dirtyBlock=1;
			
			WriteInt(block_zero_info->cBlock+16+DeletedRecordNum*4,(block_no-1)*br+(i%rl));
			DeletedRecordNum++;
		}
	}
	block_info->lock=0;
	
	WriteInt(block_zero_info->cBlock+12,DeletedRecordNum);
	block_zero_info->lock=1;
	
	block_zero_info->dirtyBlock=1; 
	
	printf("Delete Operation Succeed.\n");
}

void RecordManager::DeleteRecord_All(string DB_name,string table_name)
{
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0); 
	block_zero_info->lock=1;
	br = (unsigned int)ReadInt(block_zero_info->cBlock);
	rl = (unsigned int)ReadInt(block_zero_info->cBlock+4);
	block_zero_info->lock=0;
	
	BufferManager::DeleteFile(DB_name,table_name,0);
	if(BufferManager::HasFile(DB_name,table_name,0))
	{
		cout << "Delete Table Failed\n" ; 
		return;
	}
	BufferManager::CreateFile(DB_name,table_name,0);	
	
	block_zero_info = BufferManager::get_file_block(DB_name,table_name,DATAFILE,0);
	block_zero_info->lock=1;
	WriteInt(block_zero_info->cBlock,br);
	WriteInt(block_zero_info->cBlock+4,rl);
	WriteInt(block_zero_info->cBlock+8,0);
	WriteInt(block_zero_info->cBlock+12,0);
	block_zero_info->lock=0;
	block_zero_info->dirtyBlock=1; 
	
	printf("Delete Operation Succeed.\n");
}
