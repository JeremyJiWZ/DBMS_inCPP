#include <iostream>
#include <vector>
#include "buffer.h" 
using namespace std; 


#ifndef RECORDER_MANGER_H
#define RECORDER_MANGER_H

class record_manger
{
private:
	vector<short> DeleteRecord_Offset;
public:
	void DropTable(string DB_name,string table_name);
	bool DataExist(string DB_name,string table_name);
	void CreateTable(string DB_name,string table_name);
	void SeletRecord_WithoutIndex(string DB_name,string table_name,condition x);//condition需要包含where语句的所有信息 
	void SelectRecord_WithIndex(string DB_name,string table_name,vector<int>& offset,attr_info x);//attr_info里应该包含属性信息，record的长度信息，每个块能容纳的record条数等 
	void DeleteRecord_WithIndex(string DB_name,string table_name,int offset);
	void DeleteRecord_WithoutIndex(string DB_name,string table_name,condition x);
	
}


#endif
