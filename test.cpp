#include <iostream>
#include "RecordManager.h" 
using namespace std; 

int main()
{
	vector<unsigned int> offset;
	offset.push_back(4);
	offset.push_back(1);
	offset.push_back(3);
	offset.push_back(6);

	char tmp[4096];
	string DB_name = "zgc";
	string table_name = "book";
	struct TableHead a = {1,"book",3,0};
	struct TableAttr b[3] = {{"no",0,1,1,1,1,"abc"},{"name",1,10,1,1,1,"bcd"},{"price",2,1,1,1,1,"cde"}};	
	
	blockInfo* x;
/*	x.blockNum = 0;
	x.dirtyBlock = 0;
	x.next = NULL;
	x.charNum = 4090;
	x.cBlock = tmp;*/
	

	RecordManager test;
	char book_name[10]="lu kuan";
	float book_price = 10.5;
	Value v[2]={Value(1,"no"),Value((float)13.5,"price")};
	Condition c[2];
	c[0] = GREAT;
	c[1] = LESSEQUAL;
	
	
	test.CreateTable(DB_name,table_name,a,b);
/*	for(int i=0;i<10;i++)
	{
		memcpy(tmp,&i,4);
		memcpy(tmp+4,&book_name,10);
		memcpy(tmp+14,&book_price,4);
		test.InsertRecord(DB_name,table_name,tmp);	
		book_price = book_price + 0.5;
	}*/

	
	x = BufferManager::get_file_block(DB_name,table_name,0,0);
	x->lock=1;
	cout << test.ReadInt(x->cBlock) << '\n';
	cout << test.ReadInt(x->cBlock+4) << '\n';
	cout << test.ReadInt(x->cBlock+8) << '\n';
	cout << test.ReadInt(x->cBlock+12) << '\n';
	x->lock=0;

/*	test.SelectRecord_WithoutIndex(DB_name,table_name,a,b,v,c,2);
	test.SelectRecord_WithIndex(DB_name,table_name,offset,a,b);
	
	test.DeleteRecord_WithoutIndex(DB_name,table_name,a,b,v,c,2);
	test.SelectRecord_WithoutIndex(DB_name,table_name,a,b,v,c,2);
	test.SelectRecord_WithIndex(DB_name,table_name,offset,a,b);
	
	test.DeleteRecord_WithIndex(DB_name,table_name,offset);
	test.SelectRecord_WithIndex(DB_name,table_name,offset,a,b);
*/	
	BufferManager::quitProg(DB_name);
	

/*	for(int i=3;i<6;i++)	
		test.DeleteRecord_WithIndex(DB_name,table_name,i);
	
	x->lock=1;
	cout << test.ReadInt(x->cBlock) << '\n';
	cout << test.ReadInt(x->cBlock+4) << '\n';
	cout << test.ReadInt(x->cBlock+8) << '\n';
	cout << test.ReadInt(x->cBlock+12) << '\n';
	cout << test.ReadInt(x->cBlock+16) << '\n';
	cout << test.ReadInt(x->cBlock+20) << '\n';
	cout << test.ReadInt(x->cBlock+24) << '\n';
	x->lock=0;
	*/
	
	
/*	x = BufferManager::get_file_block(DB_name,table_name,0,1);
	x->lock=1;
	cout << test.ReadInt(x->cBlock) << '\n';
	cout << test.ReadString(x->cBlock+4,10) << '\n';
	cout << test.ReadFloat(x->cBlock+14) << '\n';
	x->lock=0;
*/
	
} 
