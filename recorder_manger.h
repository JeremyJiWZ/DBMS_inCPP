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
	void SeletRecord_WithoutIndex(string DB_name,string table_name,condition x);//condition��Ҫ����where����������Ϣ 
	void SelectRecord_WithIndex(string DB_name,string table_name,vector<int>& offset,attr_info x);//attr_info��Ӧ�ð���������Ϣ��record�ĳ�����Ϣ��ÿ���������ɵ�record������ 
	void DeleteRecord_WithIndex(string DB_name,string table_name,int offset);
	void DeleteRecord_WithoutIndex(string DB_name,string table_name,condition x);
	
}


#endif
