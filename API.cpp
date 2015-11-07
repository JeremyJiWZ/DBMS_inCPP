#include "API.h"
#include <string>
#include "Interpreter.h"
#include "RecordManager.h"
#include "CatalogManager.h"
#include <fstream>
using namespace std;

void API()
{
	string s;
	SQL_CLAUSE x;	
    TableAttr x_att[32];
    Value x_v[20];
    Condition x_cond[20];
	
	TableHead table_head;
	TableAttr table_attr[20];
	int flag;
	
	string DB_name;
	string Index_name;
	IndexManager indexmanager; 
	RecordManager recordmanager;
    fstream fp;
	
	vector<Value> attriValue;
	vector<int> offset;
    int executeFile=0;
    string SQL;
    
	while(1)
	{
        
		cout << "Please input statement:" << endl;
        x.att = x_att;
        x.v = x_v;
        x.cond = x_cond;
        if (executeFile) {
            getline(fp,SQL);
            if (fp.eof()) {
                executeFile=0;
                continue;
            }
            Interpreter(x, SQL);
        }
        else{
            SQL="";
            Interpreter(x,SQL);
        }
		switch(x.type)
		{
			case CREATEDATABASE:
				DB_name = x.name;
				if(catl->CreateDatabase(DB_name))
				{
					cout << catl->GetErrMsg() << endl;
					break;
				}
                BufferManager::UseDB(DB_name);
				break;
            case CREATETABLE:
                if(recordmanager.DataExist(DB_name,x.name) )
                {
                    cout << "table 已存在" <<　 endl;
                    break;
                }
                else
                {
                    flag = 0;
                    for(int i=0;i<x.attrAmount;i++)
                    {
                        if(x.att[i].primary==1)
                        {
                            flag = 1;
                        }
                    }
                    if(flag==0)
                    {
                        cout << "没有主键" << endl;
                        break;
                    }
                    
                    struct TableHead tmp;
                    tmp.valid= 1;
                    int j;
                    for (j=0; j<x.name.length(); j++) {
                        tmp.tableName[j]=x.name[j];
                    }
                    tmp.tableName[j]=0;
                    tmp.attrAmount = x.attrAmount;
                    tmp.recordAmount = 0;
                    
                    if(catl->CreateTable(tmp,x.att,x.attrAmount))
                    {
                        cout << catl->GetErrMsg() << endl;
                        break;
                    }
                    
                    recordmanager.CreateTable(DB_name,x.name,tmp, x.att);
                    int attramount;
                    for(int i=0;i<x.attrAmount;i++)
                    {
                        if(x.att[i].primary==1)
                        {
                            catl->GetAttrAmount(x.name,attramount);
                            attriValue.clear();
                            offset.clear();
                            string attrname = x.att[i].attrName;
                            Index_name = DB_name + x.name + attrname;
                            
                            if(x.att[i].type==1)
                                indexmanager.create(DB_name,x.name,Index_name,x.attrAmount,(int)x.att[i].type,attriValue,offset);
                            else
                                indexmanager.create(DB_name,x.name,Index_name,4,(int)x.att[i].type,attriValue,offset);
                            
                            
                            catl->CreateIndex(x.name,attrname,Index_name);
                        }				 		 
                    }
                }
                break;
            case CREATEINDEX:
                if( !recordmanager.DataExist(DB_name,x.tableName) )
                {
                    cout << "table does not exist" << endl;
                    break;
                }
                else
                {
                    catl->GetTableStruct(x.tableName,table_head,table_attr);
                    if(catl->CreateIndex(x.tableName,x.attrName,x.name))
                    {
                        cout << catl->GetErrMsg() << endl;
                        break;
                    }
                    offset.clear();
                    attriValue.clear();
                    vector<unsigned int> uoffset;
                    recordmanager.GetAttrValue(DB_name,x.tableName,table_head,table_attr,x.attrName,uoffset,attriValue);
                    
                    vector<unsigned int>::iterator i;
                    for (i=uoffset.begin(); i!=uoffset.end(); i++) {
                        offset.push_back((int)(*i));
                    }
                    int type;
                    int len;
                    catl->GetAttrType(x.tableName, x.attrName, type);
                    catl->GetAttrLen(x.tableName, x.attrName, len);
                    
                    
                    if(type==1)
                        indexmanager.create(DB_name,x.tableName,x.name,len,type,attriValue,offset);
                    else
                        indexmanager.create(DB_name,x.tableName,x.name,4,type,attriValue,offset);

                }
                break;
			case DROPDATABASE:
				BufferManager::quitProg(x.name);
				if(catl->DropDatabase(x.name))
				{
					cout << catl->GetErrMsg() << endl;
					break;	
				}
				DB_name = "";
				break;
            case DROPTABLE:
                if(!recordmanager.DataExist(DB_name,x.name))
                {
                    cout << "table 不存在" << endl;
                    catl->DropTable(x.name);
                    break;
                }
                else
                {
                    int attramount;
                    if(catl->GetAttrAmount(x.name,attramount))
                    {
                        cout << catl->GetErrMsg() << endl;
                        break;
                    }
                for(int i=0;i<attramount;i++)
                {
                    //检查每个属性是否有index，如果有，就删除
                    catl->DoesAttrHaveIndex(x.name,i,flag);
                    if(flag==1)
                    {
                        catl->GetIndexName(x.name,i,Index_name);
                        indexmanager.drop(DB_name,x.name,Index_name);
                    }
                }
                catl->DropTable(x.name);
                recordmanager.DropTable(DB_name,x.name);//recordmananger
                }
                break;
                
                //
            case SELECT:
                 if(x.correct==0)
                 {
                     cout << "Wrong Statement" << endl;
                     break;
                 }
                 if(!recordmanager.DataExist(DB_name,x.name))
                 {
                     cout << "table 不存在" << endl;
                     break;
                 }
                 catl->GetTableStruct(x.name,table_head,table_attr);
                 if(x.condAmount==0)
                 {
                     recordmanager.SelectRecord_All(DB_name,x.name,table_head,table_attr);
                 }
                 else
                 {
                     flag = 1;
                     string value_name = x.v[0].ValueName;
                     for(int i=0;i<x.condAmount;i++)
                     {
                     //检查每个value.valuename是否在该table的属性中，不存在就报错
                         catl->IsAttrInTable(x.name,x.v[i].ValueName,flag);
                         if(flag==0||x.v[i].ValueName!=value_name) //同时检查属性是否是一个
                         {
                             flag = 0;
                             break;
                         }
                    }
                     if(x.condAmount>1)
                         flag = 0;
                     if(flag == 1)
                     {
                         if(catl->GetIndexName(x.name,value_name,Index_name))
                             flag = 0;
                     } 
                     if(flag == 1)
                     {
                         offset.clear();
                         //调用index 返回offset
                         indexmanager.select(DB_name,x.name,Index_name,x.v[0],x.cond[0],offset);
                         vector<unsigned int> uoffset;
                         vector<int>::iterator i;
                         for (i=offset.begin(); i!=offset.end(); i++) {
                             uoffset.push_back((unsigned int)(*i));
                         }
                         
                         //调用 recordmanager_withindex 
                         recordmanager.SelectRecord_WithIndex(DB_name,x.name,uoffset,table_head,table_attr);
                     }
                     else
                         recordmanager.SelectRecord_WithoutIndex(DB_name,x.name,table_head,table_attr,x.v,x.cond,x.condAmount);
                 }
            break;
                
             
			case DROPINDEX:
                Index_name = x.name;
                if(catl->DropIndex(Index_name))
                {
                    cout << catl->GetErrMsg() << endl;
                    break;
                }
                indexmanager.drop(DB_name,x.tableName,Index_name);
				break;
			case DELETE:
				if(x.correct==0)
				{
					cout << "Wrong Statement" << endl;
					break;
				}
				if(!recordmanager.DataExist(DB_name,x.tableName))
				{
					cout << "table ≤ª¥Ê‘⁄" << endl;
					break;
				}
							
				catl->GetTableStruct(x.tableName,table_head,table_attr);	
				
				if(x.condAmount==0)
				{
					recordmanager.DeleteRecord_All(DB_name,x.name);
				}
				else
				{					
					flag = 1;
					string value_name = x.v[0].ValueName;
					for(int i=0;i<x.condAmount;i++)
					{
						//ºÏ≤È√ø∏ˆvalue.valuename «∑Ò‘⁄∏√tableµƒ Ù–‘÷–£¨≤ª¥Ê‘⁄æÕ±®¥Ì 
						catl->IsAttrInTable(x.name,x.v[i].ValueName,flag);
						if(flag==0||x.v[i].ValueName!=value_name) //Õ¨ ±ºÏ≤È Ù–‘ «∑Ò «“ª∏ˆ
						{
							flag = 0;
							break; 
						}	
					}
					if(flag == 1)
					{
						if(catl->GetIndexName(x.name,value_name,Index_name))
							flag = 0;
					} 
					if(flag == 1)
					{
						//µ˜”√index ∑µªÿoffset
						indexmanager.select(DB_name,x.name,Index_name,x.v[0],x.cond[0],offset);
                        
                        vector<unsigned int> uoffset;
                        vector<int>::iterator i;
                        for (i=offset.begin(); i!=offset.end(); i++) {
                            uoffset.push_back((unsigned int)(*i));
                        }
                        
						recordmanager.DeleteRecord_WithIndex(DB_name,x.name,uoffset);
						
						indexmanager.deleteFrom(DB_name,x.name,Index_name,x.v[0]);

					}
					else recordmanager.DeleteRecord_WithoutIndex(DB_name,x.name,table_head,table_attr,x.v,x.cond,x.condAmount);					
				}
				break;
            case INSERT:
                if(x.correct==0)
                 {
                     cout << "Wrong Statement" << endl;
                     break;
                 }
                 if(!recordmanager.DataExist(DB_name,x.name))
                 {
                     cout << "table 不存在" << endl;
                     break;
                 }
                 catl->GetTableStruct(x.name,table_head,table_attr);
                 char tmp[200];
                 if(!convert(table_head,table_attr,x.value,tmp))
                     break;
                unsigned int insert_offset;
                insert_offset = recordmanager.InsertRecord(DB_name, x.name, tmp);
                 for(int i=0;i<table_head.attrAmount;i++)
                 {
                     catl->DoesAttrHaveIndex(x.name,i,flag);
                     if(flag==1)
                     {
                         catl->GetIndexName(x.name,i,Index_name);
                         //这个函数通过i获得条目中对应属性值
                         stringstream ss;
                         int a=0;
                         string b;
                         float c;
                         Value d;
                         switch(table_attr[i].type)
                         {
                             case 0:
                                 ss << x.value.at(i);
                                 ss >> a;
                                 d.setInt(a);
                                 break;
                             case 1:
                                 d.setString(x.value.at(i).substr(1,x.value.at(i).size()-2));
                                 break;
                             case 2:
                                 ss << x.value.at(i);
                                 ss >> c;
                                 d.setFloat(c);
                                 break;
                             default:
                                 cout << "Convert Failed: unknown type.\n";
                                 return ;
                                break;
                         }
                         if(!catl->GetIndexName(x.name,i,Index_name))
                             indexmanager.insertInto(DB_name,x.name,Index_name,d,insert_offset);
                     }
                 }
                break;
                
			case USE:
				if(catl->UseDatabase(x.name))
				{
					cout <<catl->GetErrMsg()<< endl;
					break;
				} 
				else
                    DB_name = x.name;
				break;
			case EXECFILE:
                fp.open(x.name,ios::beg|ios::in);
                if (!fp.is_open()) {
                    cout<<"file does not exist!!"<<endl;
                    break;
                }
                executeFile=1;
				break;
			case QUIT:
				BufferManager::quitProg(DB_name);//πÿ±’ ˝æ›ø‚
				catl->Quit();
                DB_name = "";
                return;
				break;
			case HELP:
                cout<<"========================================"<<endl;
                cout<<"welcome to MINISQL v1.0!!"<<endl;
                cout<<"========================================"<<endl;
				break;
			default:
				cout << "Wrong Explain" << endl;
				break;			
		}


	}


}
