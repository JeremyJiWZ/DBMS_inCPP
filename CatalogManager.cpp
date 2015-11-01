#include "CatalogManager.h"

class CatalogManager *catl = CatalogManager::GetInstance();

CatalogManager::CatalogManager()//私有构造
{
	mDBName = "";
	mErrNum = 0;
	output = mErrMsg[mErrNum];
}

CatalogManager::~CatalogManager()//私有析构
{
	if (mFile.is_open())
		mFile.close();
}

CatalogManager * CatalogManager::GetInstance()//获得CatalogManager对象(done)
{
	static CatalogManager *catl = NULL;//CatalogManager对象
	if (catl == NULL)
		catl = new class CatalogManager;
	return catl;
}

int CatalogManager::CreateDatabase(const string & DBName)//创建数据库：创建数据库目录和cat文件，返回0代表创建成功(done)
{
	/*DBName.copy(mcName, DBName.length());
	mcName[DBName.length()] = '\0';
	if (access(mcName, 0) == 0)//目录存在
		return 1;*/
		
	if (CreateDirectory(DBName.c_str(), NULL) == 0) //返回0是出错，正确返回1
		return (mErrNum = 1);

	mstrBuf = DBName + "\\" + DBName + ".cat";
	mFile.open(mstrBuf.c_str(), ios::out);
	miBuf = 0;
	mFile.write((char *)&miBuf, sizeof(int));
	mFile.close();
	return (mErrNum = 0);
}

int CatalogManager::DropDatabase(const string & DBName)//删除数据库：删除数据库目录和cat文件，返回0代表删除成功
{
/*BOOL DelTree(LPCTSTR lpszPath)另一种方法
{
  SHFILEOPSTRUCT FileOp;
  FileOp.fFlags = FOF_NOCONFIRMATION;
  FileOp.hNameMappings = NULL;
  FileOp.hwnd = NULL;
  FileOp.lpszProgressTitle = NULL;
  FileOp.pFrom = lpszPath;
  FileOp.pTo = NULL;
  FileOp.wFunc = FO_DELETE;
  return SHFileOperation(&FileOp) == 0;
}*/
	if (access(DBName.c_str(), 0) != 0)//返回0正常存在，返回-1出错
	{
		return (mErrNum = 2);
	}
	mstrBuf = "rmdir /s /q ";
	mstrBuf += DBName;
	if (system(mstrBuf.c_str()) != 0) //返回0正常
		return (mErrNum = 3);
	return (mErrNum = 0);
}

int CatalogManager::UseDatabase(const string & DBName)//使用数据库
{
	if (access(DBName.c_str(), 0) == 0)//返回0正常存在，返回-1出错
	{
		mDBName = DBName;
		if (mFile.is_open())
			mFile.close();
		mstrBuf = DBName;  mstrBuf += "\\";  mstrBuf += DBName;  mstrBuf += ".cat";
		mFile.open(mstrBuf.c_str());
		return (mErrNum = 0);
	}
	return (mErrNum = 2);
}

int CatalogManager::CheckUseDB()
{
	return mFile.is_open();
}

int CatalogManager::GetTable(int Num)
{
	//MAX_NAME_LENGTH*65+16];
	
	int i;
	
	mFile.seekg(4 + Num*MTLEN, ios::beg);
	
	mFile.read((char *)&mTableHead.valid, sizeof(char));
	mFile.read(mTableHead.tableName, MAX_NAME_LENGTH*sizeof(char));
	mFile.read((char *)&mTableHead.attrAmount, sizeof(int));
	mFile.read((char *)&mTableHead.recordAmount, sizeof(int));
	
	for (i=0; i<Num; i++)
	{
		mFile.read(mTableAttr[i].attrName, MAX_NAME_LENGTH*sizeof(char));
		mFile.read((char *)&mTableAttr[i].type, sizeof(char));
		mFile.read((char *)&mTableAttr[i].amount, sizeof(int));
		mFile.read((char *)&mTableAttr[i].unique, sizeof(char));
		mFile.read((char *)&mTableAttr[i].primary, sizeof(char));
		mFile.read((char *)&mTableAttr[i].index, sizeof(char));
		mFile.read(mTableAttr[i].indexName, MAX_NAME_LENGTH*sizeof(char));
	}
	
	return 0;
}

int CatalogManager::PutTable(int Num)
{
	int i;
	
	mFile.seekp(4 + Num*MTLEN, ios::beg);
	
	mFile.write((char *)&mTableHead.valid, sizeof(char));
	mFile.write(mTableHead.tableName, MAX_NAME_LENGTH*sizeof(char));
	mFile.write((char *)&mTableHead.attrAmount, sizeof(int));
	mFile.write((char *)&mTableHead.recordAmount, sizeof(int));
	
	for (i=0; i<Num; i++)
	{
		mFile.write(mTableAttr[i].attrName, MAX_NAME_LENGTH*sizeof(char));
		mFile.write((char *)&mTableAttr[i].type, sizeof(char));
		mFile.write((char *)&mTableAttr[i].amount, sizeof(int));
		mFile.write((char *)&mTableAttr[i].unique, sizeof(char));
		mFile.write((char *)&mTableAttr[i].primary, sizeof(char));
		mFile.write((char *)&mTableAttr[i].index, sizeof(char));
		mFile.write(mTableAttr[i].indexName, MAX_NAME_LENGTH*sizeof(char));
	}
	
	return 0;
}

int CatalogManager::GetTableInit()
{
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&mGetAmount, sizeof(int));
	mGetNum = 0;

	return 0;
}

int CatalogManager::GetTableNext()
{
	do
	{
		GetTable(mGetNum);
		mGetNum++;
	}while (mTableHead.valid == 0);

	return mGetNum;
}

int CatalogManager::GetTableByName(const string & tableName)
{
	int i, tbAmt, num;
	
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&tbAmt, sizeof(int));
	
	num = 0;
	for (i=0; i<tbAmt; i++)
	{
		do
		{
			GetTable(num);
			num += 1;
		}while (mTableHead.valid == 0);
		
		if (strcmp(mTableHead.tableName, tableName.c_str()) == 0)
		{
			mGetNum = num-1;
			mGetAmount = tbAmt;
			return (mErrNum = 0);
		}
	}
	
	return (mErrNum = 5);
}

int CatalogManager::CreateTable(const struct TableHead & tableHead, const struct TableAttr tableAttr[], int attrAmount)
{
	int i, tbAmt, num, fsEpt=-1;
	
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&tbAmt, sizeof(int));
	
	num = 0;
	for (i=0; i<tbAmt; i++)
	{
		do
		{
			if (fsEpt==-1) fsEpt = num-1;
			GetTable(num);
			num += 1;
		}while (mTableHead.valid == 0);
		
		if (strcmp(mTableHead.tableName, tableHead.tableName) == 0) return (mErrNum = 4);
	}
	
	tbAmt++;
	mFile.seekp(0, ios::beg);
	mFile.write((char *)&tbAmt, sizeof(int));
	
	mTableHead = tableHead;
	mTableHead.valid = 1;
	for (i=0; i<mTableHead.attrAmount; i++)
	mTableAttr[i] = tableAttr[i];
	
	if (fsEpt == -1)
	{
		PutTable(tbAmt);
	}
	else
	{
		PutTable(fsEpt);
	}
	
	return (mErrNum = 0);
}//创建表：在数据库目录下cat文件中添加数据

int CatalogManager::DropTable(const string & tableName)
{
	int i, tbAmt, num;
	
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&tbAmt, sizeof(int));
	
	num = 0;
	for (i=0; i<tbAmt; i++)
	{
		do
		{
			GetTable(num);
			num += 1;
		}while (mTableHead.valid == 0);
		
		if (strcmp(mTableHead.tableName, tableName.c_str()) == 0)
		{
			tbAmt--;
			mFile.seekg(0, ios::beg);
			mFile.read((char *)&tbAmt, sizeof(int));
			
			mTableHead.valid = 0;
			num--;
			PutTable(num-1);
			return (mErrNum = 0);
		}
	}
		
	return (mErrNum = 5);
}//删除表：删除数据库目录下cat文件相关数据

int CatalogManager::GetAttrAmount(const string & tableName, int & attrAmount)
{
	int i, tbAmt, num;
	
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&tbAmt, sizeof(int));
	
	num = 0;
	for (i=0; i<tbAmt; i++)
	{
		do
		{
			GetTable(num);
			num += 1;
		}while (mTableHead.valid == 0);
		
		if (strcmp(mTableHead.tableName, tableName.c_str()) == 0)
		{
			attrAmount = mTableHead.attrAmount;
			return (mErrNum = 0);
		}
	}
	
	return (mErrNum = 5);
}//获得某个表属性数量

int CatalogManager::GetAttrNameWithNum(const string & tableName, int num, string & attrName)
{
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
	
	if (num >= mTableHead.attrAmount)
		return (mErrNum = 6);
				
	attrName = mTableAttr[num].attrName;
	return (mErrNum = 0);
}//这个函数好像没用

int CatalogManager::IsAttrInTable(const string & tableName, const string & attrName, int & ret)
{
	int i;
	ret = 0;
	
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	for (i=0; i<mTableHead.attrAmount; i++)
	{
		if (strcmp(mTableAttr[i].attrName, attrName.c_str()) == 0)
		{
			ret = 1;
			return (mErrNum = 0);
		}
	}
	
	return (mErrNum = 7);
}//存在返回0

int CatalogManager::GetAttrType(const string & tableName, const string & attrName, int & type)
{
	int i;
	
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	for (i=0; i<mTableHead.attrAmount; i++)
	{
		if (strcmp(mTableAttr[i].attrName, attrName.c_str()) == 0)
		{
			type = mTableAttr[i].type;
			return (mErrNum = 0);
		}
	}
	return (mErrNum = 7);
}//返回的类型保存在type中

int CatalogManager::GetAttrType(const string & tableName, int num, int & type)
{
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	if (num >= mTableHead.attrAmount)
		return (mErrNum = 6);
		
	type = mTableAttr[num].type;
	return (mErrNum = 0);
}//返回的类型保存在type中

int CatalogManager::IsAttrType(const string & tableName, const string & attrName, int type, int & ret)
{
	return (mErrNum = 0);
}//相同返回0

int CatalogManager::GetAttrByte(const string & tableName, int num, int & bytes)
{
	return (mErrNum = 0);
}

int CatalogManager::GetAllAttrByte(const string & tableName, int & bytes)
{
	return (mErrNum = 0);
}

int CatalogManager::GetTableStruct(const string & tableName, struct TableHead & tableHead, struct TableAttr tableAttr[])
{
	int i;

	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
	
	tableHead = mTableHead;
	
	for (i=0; i<mTableHead.attrAmount; i++)
		tableAttr[i] = mTableAttr[i];
		
	return (mErrNum = 0);
}

int CatalogManager::CreateIndex(const string & tableName, const string & attrName, const string & indexName)
{
	int i, j, tbAmt, num;
	
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&tbAmt, sizeof(int));
	
	num = 0;
	for (i=0; i<tbAmt; i++)
	{
		do
		{
			GetTable(num);
			num += 1;
		}while (mTableHead.valid == 0);
		
		for (j=0; j<mTableHead.attrAmount; j++)
			if (mTableAttr[j].index && (strcmp(mTableAttr[j].indexName, indexName.c_str())==0) )
			{
				//index name exists
				return (mErrNum = 13);
			}
	}
	
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	for (i=0; i<mTableHead.attrAmount; i++)
		if (strcmp(mTableAttr[i].attrName, attrName.c_str()) == 0)
		{
			if (mTableAttr[i].index)
				return (mErrNum = 11);//index exists
			
			mTableAttr[i].index = 1;
			strcpy(mTableAttr[i].indexName, indexName.c_str());
			PutTable(mGetNum);
			
			return (mErrNum = 0);
		}
		
	return (mErrNum = 7);
}

int CatalogManager::DropIndex(const string & indexName)
{
	
	int i, j, tbAmt, num;
	
	mFile.seekg(0, ios::beg);
	mFile.read((char *)&tbAmt, sizeof(int));
	
	num = 0;
	for (i=0; i<tbAmt; i++)
	{
		do
		{
			GetTable(num);
			num += 1;
		}while (mTableHead.valid == 0);
		
		for (j=0; j<mTableHead.attrAmount; j++)
			if (mTableAttr[j].index && (strcmp(mTableAttr[j].indexName, indexName.c_str())==0) )
			{
				//drop
				mTableAttr[j].index = 0;
				num--;
				PutTable(num);
				return (mErrNum = 0);
			}
	}
		
	return (mErrNum = 12);//index not exists
}

int CatalogManager::GetIndexAmount(const string & tableName)
{
	return (mErrNum = 0);
}

int CatalogManager::DoesAttrHaveIndex(const string & tableName, const string & attrName, int & ret)
{
	int i;
	
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	for (i=0; i<mTableHead.attrAmount; i++)
		if (strcmp(mTableAttr[i].attrName, attrName.c_str()) == 0)
		{
			if (mTableAttr[i].index)
			{
				ret = 1;
				return (mErrNum = 0);//index exists
			}
			ret = 0;
			return (mErrNum = 0);
		}
		
	return (mErrNum = 7);//attr not exists
}

int CatalogManager::DoesAttrHaveIndex(const string & tableName, int num, int & ret)
{
	ret = 0;
	
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	if (num >= mTableHead.attrAmount)
		return (mErrNum = 6);
		
	if (mTableAttr[num].index == 0)
		return (mErrNum = 10);
		
	ret = 1;
	
	return (mErrNum = 0);
}

int CatalogManager::GetIndexName(const string & tableName, int num, string & indexName)
{
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	if (num >= mTableHead.attrAmount)
		return (mErrNum = 6);
		
	if (mTableAttr[num].index == 0)
		return (mErrNum = 10);
		
	indexName = mTableAttr[num].indexName;
	
	return (mErrNum = 0);
}//返回的index名字结果在indexName中

int CatalogManager::GetIndexName(const string & tableName, const string & attrName, string & indexName)
{
	return (mErrNum = 0);
}//返回的index名字在indexName中

int CatalogManager::IsIndexInTable(const string & tableName, const string & indexName)
{
	return (mErrNum = 0);
}

int CatalogManager::Insert(const string & tableName, int amount, int type[])
{
	int i;
	
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	if (amount != mTableHead.attrAmount) return (mErrNum = 8);
		
	for (i=0; i<mTableHead.attrAmount; i++)
	{
		if (mTableAttr[i].type != type[i]) return (mErrNum = 9);
	}
		
	return (mErrNum = 0);
}//判断插入的数据类型是否匹配，注意不会导致记录的数量改变，若要增加记录数量请使用AddRecordAmount方法

//int Delete(const string & tableName);这个函数好像没用
//int Select(const string & tableName);同好像没用

int CatalogManager::GetRecordAmount(const string & tableName, int & amount)
{
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	amount = mTableHead.recordAmount;
		
	return (mErrNum = 0);
}

int CatalogManager::AddRecordAmount(const string & tableName, int amount)
{
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	mTableHead.recordAmount += amount;
	
	PutTable(mGetNum);
		
	return (mErrNum = 0);
}

int CatalogManager::SubRecordAmount(const string & tableName, int amount)
{
	GetTableByName(tableName);
	if (mErrNum != 0) return mErrNum;
		
	mTableHead.recordAmount -= amount;
	
	PutTable(mGetNum);
		
	return (mErrNum = 0);
}

int CatalogManager::GetErrNum()
{
	return mErrNum;
}//获得错误代码

string & CatalogManager::GetErrMsg()
{
	output = mErrMsg[mErrNum];
	return output;
}//通过最近一条错误操作的错误代码返回错误信息

string & CatalogManager::GetErrMsg(int errNum)
{
	output = mErrMsg[errNum];
	return output;
}//通过错误代码返回错误信息

int CatalogManager::Quit()//退出数据库时一定要调用，析构catl实例
{
	//关闭cat文件

	delete(this);
	
	return 0;
}

const int CatalogManager::INT = 0;   // = 0;
const int CatalogManager::CHAR = 1;  // = 1;
const int CatalogManager::FLOAT = 2; // = 2;
const int CatalogManager::MTLEN = MAX_NAME_LENGTH*65+17;  //max table length

const string CatalogManager::mErrMsg[] = {
	"Welcome to error message.",//0
	"Database already exists.", //1
	"Database does not exist.", //2
	"Database delete error.",   //3
	"Table already exists.",    //4
	"Table does not exist.",    //5
	"Attribute number larger than amount.",  //6
	"Attribute does not exist.",//7
	"Amount of insertd types does not match.", //8
	"Insertd type does not match.", //9
	"That attribute does not have an index.", //10
	"Attribute already has an index.", //11
	"Index does not exists.",    //12
	"Index name already exists" //13
};//错误信息，要初始化 