#include "API.h"
#include <string>
#include "Interpreter.h"
#include "RecordManager.h"
#include "CatalogManager.h"
using namespace std;

void API()
{
	string s;
	SQL_CLAUSE x;	
	TableAttr x_attr[20];
    Value x_v[20]; 
    Condition x_cond[20];
    x.att = x_attr;
    x.v = x_v;
    x.cond = x_cond;
	
	TableHead table_head;
	TableAttr table_attr[20];
	int flag;
	
	string DB_name;
	string Index_name;
	IndexManager indexmanager; 
	RecordManager recordmanager;
	
	vector<Value> attriValue;
	vector<int> offset;

	while(1)
	{
		
		cout << "Please input statement:" << endl;
		
		Interpreter(x);
		
/*		
				
		if(DB_name.empty())
		{
			cout << "Please Use DB" << endl;

			continue;
		}
*/	

	
		

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
                    for (int j=0; j<x.name.length(); j++) {
                        tmp.tableName[j]=x.name[j];
                    }
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
                            indexmanager.create(DB_name,x.name,Index_name,attramount,attriValue,offset);
                            catl->CreateIndex(x.name,attrname,Index_name);
                        }				 		 
                    }
                }
                break;
		/*	case CREATEINDEX:
				if( recordmanager.DataExist(DB_name,x.name) )
				{
					cout << "table ≤ª¥Ê‘⁄" <<°° endl;
					break;
				}
				else
				{
					
				 	if(catl->CreateIndex(x.tableName,x.attrName,x.name))
					{
						cout << catl->GetErrMsg() << endl;
						break;
					}
					
					for()
					{
						ªÒµ√±Ì¿Ô√ÊÀ˘”–Ãıƒøµƒoffset∫Õ∂‘”¶ Ù–‘µƒ÷µ		
					}
				 	indexmanager.create(DB_name,x.name,index_name,catalog.getattributbytes(),attriValue,offset);
				}
				break;*/
			case DROPDATABASE:
				BufferManager::quitProg(x.name);
				if(catl->DropDatabase(x.name))
				{
					cout << catl->GetErrMsg() << endl;
					break;	
				}
				DB_name = "";
				break;
			/*case DROPTABLE:
				if(recordmanager.DataExist(DB_name,x.name))
				{
					cout << "table ≤ª¥Ê‘⁄" << endl;
					break;
				}
				else
				{
					int attramount;
					if(catl->GetAttrAmount(x.tableName,attramount))
					{
						cout << catl->GetErrMsg() << endl;
						break;	
					}
					
				 	for(int i=0;i<attramount;i++)
					{
						//ºÏ≤È√ø∏ˆ Ù–‘ «∑Ò”–index£¨»Áπ˚”–£¨æÕ…æ≥˝ 
						catl->DoesAttrHaveIndex(x.tableName,i,flag);
						if(flag==1)
						{
							catl->GetIndexName(x.tableName,i,Index_name);
							indexmanager.drop(DB_name,x.name,Index_name);
						}						
					} 
				 	catl->DropTable(x.tableName);
				 	recordmanager.DropTable(DB_name,x.tableName);//recordmananger 
				}
				break;
			case DROPINDEX:
				if(!recordmanager.DataExist((DB_name,x.tableName)))
				{
					cout << "table ≤ª¥Ê‘⁄" << endl;
					break;
				}
				 else
				{
					catl->IsAttrInTable(x.tableName,x.attrName,flag);
				 	if(flag==0)
					{
						cout << " Ù–‘ ≤ª¥Ê‘⁄" << endl;
						break;
					} 
					if(catl->GetIndexName(x.tableName,x.attrName,Index_name))
					{
						cout << catl->GetErrMsg() << endl;
						break;
					} 
				 	indexmanager.drop(DB_name,x.name,Index_name);
				 	catl->DropIndex(Index_name);
				 }
				break;			
			case SELECT:	
				if(x.correct==0)
				{
					cout << "Wrong Statement" << endl;
					break;
				}
				if(!recordmanager.DataExist((DB_name,x.tableName)))
				{
					cout << "table ≤ª¥Ê‘⁄" << endl;
					break;
				}
				
				catl->GetTableStruct(x.tableName,table_head,table_attr);

				if(x.condAmount==0)
				{
					recordmanager.SelectRecord_All(DB_name,x.name,table_head,table_attr);
				}
				else
				{					
					bool flag = 1;
					string value_name = x.v.at(0).ValueName;
					for(int i=0;i<x.condAmount;i++)
					{
						//ºÏ≤È√ø∏ˆvalue.valuename «∑Ò‘⁄∏√tableµƒ Ù–‘÷–£¨≤ª¥Ê‘⁄æÕ±®¥Ì 
						catl->IsAttrInTable(x.name,x.v.at(i).ValueName,flag);
						
						if(flag==0||x.v.at(i).ValueName!=vaule_name) //Õ¨ ±ºÏ≤È Ù–‘ «∑Ò «“ª∏ˆ 
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
						indexmanager.select(DB_name,x.name,Index_name,x.v[0],cond[0],offset);
						//µ˜”√ recordmanager_withindex 
						recordmanager.SelectRecord_WithIndex(DB_name,x.name,offset,table_head,table_attr);
					}
					else recordmanager.SelectRecord_WithoutIndex(DB_name,x.name,table_head,table_attr,x.v,x.cond,x.condAmount);					
				}
				break;
			case DELETE:
				if(x.correct==0)
				{
					cout << "Wrong Statement" << endl;
					break;
				}
				if(!recordmanager.DataExist((DB_name,x.tableName)))
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
					bool flag = 1;
					string value_name = x.v.at(0).ValueName;
					for(int i=0;i<x.condAmount;i++)
					{
						//ºÏ≤È√ø∏ˆvalue.valuename «∑Ò‘⁄∏√tableµƒ Ù–‘÷–£¨≤ª¥Ê‘⁄æÕ±®¥Ì 
						catl->IsAttrInTable(x.name,x.v.at(i).ValueName,flag);						
						if(flag==0||x.v.at(i).ValueName!=vaule_name) //Õ¨ ±ºÏ≤È Ù–‘ «∑Ò «“ª∏ˆ 
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
						indexmanager.select(DB_name,x.name,Index_name,x.v[0],cond[0],offset);
						recordmanager.DeleteRecord_WithIndex(DB_name,x.name,offset);
						
						indexmanager.deleteFrom(DB_name,x.name,Index_name,v[0]);

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
				if(!recordmanager.DataExist((DB_name,x.tableName)))
				{
					cout << "table ≤ª¥Ê‘⁄" << endl;
					break;
				}
					
				catl->GetTableStruct(x.tableName,table_head,table_attr);
				char tmp[200];				
				Interpreter::convert(table_head,table_attr,x.value,tmp);					
				 
				if() 
				{
					cout << "insert≤ª∑˚∫œ∏Ò Ω" << endl;	//’‚¿ÔÕ®π˝–ﬁ∏ƒconvert÷±Ω”Ω‚æˆ≈–∂œ 
				} 		

				unsigned int insert_offset = recordmanager.InsertRecord(DB_name,x.name,tmp);	
				
				for(int i=0;i<table_head.attrAmount;i++)
				{
					catl->DoesAttrHaveIndex(x.name,i,flag);
					if(flag==1)
					{
						catl->GetIndexName(x.name,i,Index_name);
						–¥∏ˆ∫Ø ˝Õ®π˝iªÒµ√Ãıƒø÷–∂‘”¶ Ù–‘÷µ 
						indexmanager.insertInto(DB_name,x.name,getindexname(),,insert_offset);						
					} 
				}
				break;
     */
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
				break;
			case QUIT:
				BufferManager::quitProg(DB_name);//πÿ±’ ˝æ›ø‚
				catl->Quit();
                DB_name = "";
                return;
				break;
			case HELP:
//				¥Ú”°∞Ô÷˙–≈œ¢; 
				break;
			default:
				cout << "Wrong Explain" << endl;
				break;			
		}


	}


}
