#include<interpreter.h>
#include<cstdlib>
#include<iterator>
#include<algorithm>
string read_input()
{
    string SQL;
    string temp;
    char str[100];
    bool finish=false;
    while(!finish)
    {
        cin>>str;
        temp=str;
        SQL=SQL+" "+temp;
        if(SQL.at(SQL.length()-1)==';')
        {
            SQL[SQL.length()-1] = ' ';
            SQL+=";";
            finish=true;
        }
    }
    //将输入大写转化为小写
    //SQL.MakeLower();
    //返回用户输入
    transform(SQL.begin(), SQL.end(), SQL.begin(), ::tolower);
    return SQL;
}
void get_attribute(string temp,SQL_CLAUSE &sql_cla)
{
    int start=0,end,index;
    string T,C;
    TableAttr attr;
    temp+=" ";
    //获得属性名
    end=temp.find(' ',start);
    T=temp.substr(start,end-start);
    start=end+1;
    //    sql+=T+" ";
    int i;
    for (i=0; i<T.length(); i++) {
        attr.attrName[i]=T[i];
    }
    attr.attrName[i]=0;
    
    //获得数据类型
    while(temp.at(start)==' ')
        start++;
    end=temp.find(' ',start);
    T=temp.substr(start,end-start);
    start=end+1;
    //若无，打印出错信息
    if(T == "")
    {
        cout<<"error : error in create table statement!"<<endl;
        sql_cla.type=ERROR;
        return;
    }
    //若为int
    else if(T=="int")
        attr.type=CTG_INT;
    //若为float
    else if(T=="float")
        attr.type=CTG_FLOAT;
    //其他
    else
    {
        index=T.find('(');
        C=T.substr(0, index);
        index++;
        //若有误，打印出错信息
        if(C == "")
        {
            cout<<"error: "<<T<<"---is not a valid data type definition!"<<endl;
            sql_cla.type=ERROR;
            return;
        }
        //若为char
        else if(C=="char")
        {
            C=T.substr(index,T.length()-index-1);
            if(C == "")
            {
                cout<<"error: the length of the data type char has not been specified!"<<endl;
                sql_cla.type=ERROR;
                return;
            }
            else{
                attr.type=CTG_CHAR;
                attr.amount=atoi(C.c_str());
            }
        }
        //若为非法信息，打印出错信息
        else
        {
            cout<<"error: "<<C<<"---is not a valid key word!"<<endl;
            sql_cla.type=ERROR;
            return ;
        }
    }
    //是否有附加信息
    while(start<temp.length()&&temp.at(start)==' ')
        start++;
    if(start<temp.length())
    {
        //若为unique定义，保存信息
        end=temp.find(' ',start);
        T=temp.substr(start,end-start);
        if(T=="unique")
        {
            attr.unique=1;
        }
        //若为非法信息，打印出错信息
        else
        {
            cout<<"error: "<<temp<<"---is not a valid key word!"<<endl;
            sql_cla.type=ERROR;
            return ;
        }
    }
    //若无附加信息
    else
        attr.unique=0;
    sql_cla.attrAmount++;
    sql_cla.attr.push_back(attr);
    return ;
}

void create_table(string SQL,int start,SQL_CLAUSE &sql_cla)
{
    string temp,sql,T;
    sql_cla.attrAmount=0;
    sql_cla.attr.clear();
    int index,end,length;
    //获取表名
    while(SQL.at(start)==' ')
        start++;
    index=start;
    if((end=SQL.find('(',start))==-1)
    {
        cout<<"error: missing ( in the statement!"<<endl;
        sql_cla.type=ERROR;
        return ;
    }
    temp=SQL.substr(start,end-start);
    start=end+1;
    if(!(temp == ""))
    {
        while(SQL.at(start)==' ')
            start++;
        length=temp.length()-1;
        while(temp.at(length)==' ')
            length--;
        temp=temp.substr(0, length+1);
        sql_cla.name=temp;
    }
    //若无，打印出错信息
    if(temp == "")
    {
        cout<<"error: error in create table statement!"<<endl;
        sql_cla.type=ERROR;
        return ;
    }
    //若为非法信息，打印出错信息
    else if(temp.find(' ')!=-1)
    {
        cout<<"error: "<<temp<<"---is not a valid table name!"<<endl;
        sql_cla.type=ERROR;
        return ;
    }
    else
    {
        sql=temp+",";
        //获取每个属性
        while((end=SQL.find(',',start))!=-1)
        {
            temp=SQL.substr(start,end-start);
            start=end+1;
            //若有空属性，打印出错信息
            if(temp == "")
            {
                cout<<"error: error in create table statement!"<<endl;
                sql_cla.type=ERROR;
                return ;
            }
            //保存属性
            else
            {
                get_attribute(temp, sql_cla);
                if(sql_cla.type==ERROR)
                    return ;
            }
            while(SQL.at(start)==' ')
                start++;
        }
        //获得最后属性
        temp=SQL.substr(start,SQL.length()-start-1);
        length=temp.length()-1;
        while(temp.at(length)!=')'&&length>=0)
            length--;
        //若无，打印出错信息
        if(length<1)
        {
            cout<<"error: error in create table statement!"<<endl;
            sql_cla.type=ERROR;
            return ;
        }
        //存储属性
        else
        {
            temp=temp.substr(0, length);
            end=SQL.find(' ',start);
            T=SQL.substr(start,end-start);
            start=end+1;
            //若为主键定义
            if(T=="primary")
            {
                //判断是否有关键字key
                temp+=")";
                while(SQL.at(start)==' ')
                    start++;
                end=SQL.find('(',start);
                T=SQL.substr(start,end-start);
                start=end+1;
                length=T.length()-1;
                while(T.at(length)==' ')
                    length--;
                T=T.substr(0, length+1);
                //若为空，打印出错信息
                if(T == "")
                {
                    cout<<"syntax error: syntax error in create table statement!"<<endl;
                    cout<<"\t missing key word key!"<<endl;
                    sql_cla.type=ERROR;
                    return ;
                }
                //若有，继续验证
                else if(T=="key")
                {
                    //获取主键属性名
                    while(SQL.at(start)==' ')
                        start++;
                    end=SQL.find(')',start);
                    T=SQL.substr(start,end-start);
                    length=T.length()-1;
                    while(T.at(length)==' ')
                        length--;
                    T=T.substr(0, length+1);
                    //若无，打印出错信息
                    if(T == "")
                    {
                        cout<<"error : missing primary key attribute name!"<<endl;
                        sql_cla.type=ERROR;
                        return ;
                    }
                    //若为非法信息，打印出错信息
                    else if(T.find(' ')!=-1)
                    {
                        cout<<"error : "<<T<<"---is not a valid primary key attribute name!"<<endl;
                        sql_cla.type=ERROR;
                        return ;
                    }
                    //保存主键
                    else
                    {
                        for (auto i:sql_cla.attr) {
                            if(T==i.attrName){
                                i.primary=1;
                                i.index=1;
                                break;
                            }
                        }
                        sql_cla.type=CREATETABLE;
                    }
                }
                //若为非法信息，打印出错信息
                else
                {
                    cout<<"error : "<<T<<"---is not a valid key word!"<<endl;
                    sql_cla.type=ERROR;
                    return ;
                }
            }
            //若为一般属性
            else
            {
                get_attribute(temp,sql_cla);
            }
        }
    }
    sql_cla.type=CREATETABLE;
    ConvertToArray(sql_cla.att, sql_cla.attr);
    return ;
}
SQL_CLAUSE create_database(string SQL,int start)
{
    SQL_CLAUSE sql_cla;
    int index,end;
    string temp;
    //获取第三个单词
    while(SQL.at(start)==' ')
        start++;
    index=start;
    end=SQL.find(' ',start);
    temp=SQL.substr(start,end-start);
    start=end+1;
    //若无，打印出错信息
    if(temp=="")
    {
        cout<<"error: database name has not been specified!"<<endl;
        sql_cla.type=ERROR;
    }
    else
    {
        while(SQL.at(start)==' ')
            start++;
        //若为非法信息，打印出错信息
        if(SQL.at(start)!=';'||start!=SQL.length()-1)
        {
            cout<<"error12:"<<SQL.substr(index,SQL.length()-index-2)<<"---is not a valid database name!"<<endl;
            sql_cla.type=ERROR;
        }
        //返回drop database语句的内部形式
        else{
            sql_cla.name=temp;
            sql_cla.type=CREATEDATABASE;
        }
    }
    sql_cla.type=CREATEDATABASE;
    return sql_cla;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//验证create index on语句是否有效
void create_index_on(string SQL,int start,SQL_CLAUSE &sql_cla)
{
    string temp;
    int end,length;
    //获取表名
    while(SQL.at(start)==' ')
        start++;
    end=SQL.find('(',start);
    temp=SQL.substr(start,end-start);
    start=end+1;
    //若无，打印出错信息
    if(temp=="")
    {
        cout<<"syntax error: syntax error for create index statement!"<<endl;
        cout<<"\t missing ( !"<<endl;
        sql_cla.type=ERROR;
        return ;
    }
    else
    {
        //检验是否为有效文件名
        length=temp.length()-1;
        while(temp.at(length)==' ')
            length--;
        temp=temp.substr(0,length+1);
        //有效
        if(temp.find(' ')==-1)
        {
            sql_cla.tableName=temp;//置表名
            //获取属性名
            while(SQL.at(start)==' ')
                start++;
            end=SQL.find(')',start);
            temp=SQL.substr(start,end-start);
            start=end+1;
            //若无，打印出错信息
            if(temp=="")
            {
                cout<<"syntax error: syntax error for create index statement!"<<endl;
                cout<<"\t missing ) !"<<endl;
                sql_cla.type=ERROR;
                return ;
            }
            else
            {
                //检验是否为有效属性名
                length=temp.length()-1;
                while(temp.at(length)==' ')
                    length--;
                temp=temp.substr(0,length+1);
                //有效
                if(temp.find(' ')==-1)
                {
                    sql_cla.attrName=temp;
                    while(SQL.at(start)==' ')
                        start++;
                    if(SQL.at(start)!=';'||start!=SQL.length()-1)
                    {
                        cout<<"syntax error: syntax error for quit!"<<endl;
                        sql_cla.type=ERROR;
                        return ;
                    }
                    //返回create index语句的内部形式
                    else
                        sql_cla.type=CREATEINDEX;
                }
                //无效,打印出错信息
                else
                {
                    cout<<"error:"<<" "<<temp<<"---is not a valid attribute name!"<<endl;
                    sql_cla.type=ERROR;
                    return ;
                }
            }
        }
        //无效,打印出错信息
        else
        {
            cout<<"error:"<<" "<<temp<<"---is not a valid table name!"<<endl;
            sql_cla.type=ERROR;
            return ;
        }
    }
    return ;
}
//验证create index语句是否有效
SQL_CLAUSE create_index(string SQL,int start)
{
    string temp;
    SQL_CLAUSE sql_cla;
    int end;
    //获取第三个单词
    while(SQL.at(start)==' ')
        start++;
    end=SQL.find(' ',start);
    temp=SQL.substr(start,end-start);
    start=end+1;
    //若无，打印出错信息
    if(temp=="")
    {
        cout<<"syntax error: syntax error for create index statement!"<<endl;
        sql_cla.type=ERROR;
    }
    else
    {
        sql_cla.name=temp;
        //获取第四个单词
        while(SQL.at(start)==' ')
            start++;
        end=SQL.find(' ',start);
        temp=SQL.substr(start,end-start);
        start=end+1;
        //若无，打印出错信息
        if(temp=="")
        {
            cout<<"syntax error: syntax error for create index statement!"<<endl;
            sql_cla.type=ERROR;
        }
        //若为on,继续验证
        else if(temp=="on")
            create_index_on(SQL,start,sql_cla);
        //若为非法信息，打印非法信息
        else
        {
            cout<<"syntax error:"<<" "<<temp<<"---is not a valid key word!"<<endl;
            sql_cla.type=ERROR;
        }
    }
    return sql_cla;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//验证create语句是否有效
void create_clause(string SQL,int start,SQL_CLAUSE &sql_cla)
{
    string temp;
    int end;
    //获取第二个单词
    while(SQL.at(start)==' ')
        start++;
    end=SQL.find(' ',start);
    temp=SQL.substr(start,end-start);
    start=end+1;
    //若无，打印出错信息
    if(temp=="")
    {
        cout<<"syntax error: syntax error for create statement!"<<endl;
        sql_cla.type=ERROR;
    }
    //若为database,继续验证
    else if(temp=="database")
        sql_cla=create_database(SQL,start);
    //若为table,继续验证
    else if(temp=="table")
        create_table(SQL,start,sql_cla);
    //若为index,继续验证
    else if(temp=="index")
        sql_cla=create_index(SQL,start);
    //若为错误信息，打印出错信息
    else
    {
        cout<<"syntax error:"<<" "<<temp<<"---is not a valid key word!"<<endl;
        sql_cla.type=ERROR;
    }
    //返回create语句的内部形式
}
//获取用户输入，并对输入作有效性检查，若正确，返回语句的内部表示形式
void Interpreter(SQL_CLAUSE &sql_cla)
{
    string SQL;
    string temp;
    string sql;
    int start=0,end;
    //获取用户输入
    SQL=read_input();
    //获取输入的第一个单词
    while(SQL.at(start)==' ')
        start++;
    end=SQL.find(' ',start);
    temp=SQL.substr(start,end-start);
    start=end+1;
    //若无输入，打印出错信息
    if(temp=="")
    {
        cout<<"syntax error: empty statement!"<<endl;
        sql_cla.type=ERROR;
    }
    //若为create语句
    else if(temp=="create")
        create_clause(SQL,start,sql_cla);
    //若为drop语句
    else if(temp=="drop")
        sql_cla=drop_clause(SQL,start);
    //若为select语句
    else if(temp=="select")
        ExplainStatement(SQL.substr(0,SQL.length()-2),sql_cla);
    //若为insert语句
    else if(temp=="insert")
        ExplainStatement(SQL.substr(0,SQL.length()-2),sql_cla);
    //若为delete语句
    else if(temp=="delete")
        ExplainStatement(SQL.substr(0,SQL.length()-2),sql_cla);
    //若为use语句
    else if(temp=="use")
        sql_cla=use_clause(SQL,start);
    //若为execfile语句
    else if(temp=="execfile")
        sql_cla=execfile_clause(SQL,start);
    //若为quit语句
    else if(temp=="quit")
        sql_cla=quit_clause(SQL,start);
    //获取帮助
    else if(temp=="help")
        sql_cla.type=HELP;
    //若为非法语句
    else
    {
        cout<<"syntax error:"<<" "<<temp<<"---is not a valid key word!"<<endl;
        sql_cla.type=ERROR;
    }
    //返回输入语句的内部形式
}

vector<string> split( string str, string pattern)
{
    vector<string> ret;
    if(pattern.empty()) return ret;
    size_t start=0,index=str.find_first_of(pattern,0);
    while(index!=str.npos)
    {
        if(start!=index)
            ret.push_back(str.substr(start,index-start));
        start=index+1;
        index=str.find_first_of(pattern,start);
    }
    if(!str.substr(start).empty())
        ret.push_back(str.substr(start));
    return ret;
}

bool getCond(Condition* c,string s)
{
    if(s=="=") *c=EQUAL;
    else if(s=="<") *c=LESS;
    else if(s=="<=") *c=LESSEQUAL;
    else if(s==">") *c=GREAT;
    else if(s==">=") *c=GREATEQUAL;
    else if(s=="!=") *c=NOTEQUAL;
    else return 0;
    
    return 1;
}



void convert(struct TableHead& tableHead, struct TableAttr* tableAttr,vector<string>& info,char* insert_info)
{
    if(tableHead.attrAmount!=info.size())
    {
        cout << "Convert Failed: wrong size" << endl;	
        return;
    }
    
    stringstream ss;
    int a=0;
    string b;
    float c;
    
    
    for(int i=0;i<tableHead.attrAmount;i++)
    {
        switch(tableAttr[i].type)
        {
            case CTG_INT:
                ss << info.at(i);
                ss >> a;
                memcpy(insert_info,&a,4);
                insert_info = insert_info + 4;
                break;
            case CTG_CHAR:
                for(int k=0;k<tableAttr[i].amount;k++)
                {
                    if(k<info.at(i).size()-2)
                        insert_info[k] = info.at(i).at(k+1);
                    else insert_info[k] = 0;
                }
                insert_info = insert_info + tableAttr[i].amount;
                break;
            case CTG_FLOAT:
                ss << info.at(i);
                ss >> c;
                memcpy(insert_info,&c,4);
                insert_info = insert_info + 4;
                break;
            default:
                cout << "Convert Failed: unknown type.\n";
                return;
                break;
        }
        ss.clear();
    }
    
}
void ExplainStatement(string statement,SQL_CLAUSE &x)
{
    x.correct = 1;
    
    vector<string> result=split(statement," ");
    
    if(result.at(0)=="select")
    {
        x.type = SELECT;
        if(result.at(1)=="*")
        {
            if(result.at(2)=="from")
            {
                x.name = result.at(3);
                if(result.size()>5 && result.size()%4==0)
                {
                    const int CondNum= result.size()/4 - 1;
                    x.condAmount = CondNum;
                    for(int i=4;i<result.size();i=i+4)
                    {
                        if(result.at(i)=="where"||result.at(i)=="and")
                        {
                            x.v[i/4-1].setValueName(result.at(i+1));
                            x.correct=getCond(x.cond+i/4-1,result.at(i+2));
                            
                            int a;
                            string b;
                            float c;
                            string s = result.at(i+3);
                            stringstream ss;
                            
                            if(s.at(0)=='\''&&s.at(s.size()-1)=='\'')
                            {
                                b = s.substr(1,s.size()-2);
                                x.v[i/4-1].setString(b);
                            }
                            else
                            {
                                ss << s;
                                if(s.find('.')!=string::npos)
                                {
                                    ss >> c;
                                    x.v[i/4-1].setFloat(c);
                                }
                                else
                                {
                                    ss >> a;
                                    x.v[i/4-1].setInt(a);
                                }
                            }
                            
                        }
                        else x.correct = 0;
                    }
                }
                else if(result.size()==4) x.condAmount = 0;
                else x.correct = 0;
            }
            else x.correct = 0;
        }
        else x.correct = 0;
    }
    else if(result.at(0)=="delete")
    {
        x.type = DELETE;
        if(result.at(1)=="from")
        {
            x.name = result.at(2);
            if(result.size()>4 && (result.size()+1)%4==0)
            {
                const int CondNum= result.size()/4;
                x.condAmount = CondNum;
                for(int i=3;i<result.size();i=i+4)
                {
                    if(result.at(i)=="where"||result.at(i)=="and")
                    {
                        x.v[i/4].setValueName(result.at(i+1));
                        x.correct=getCond(x.cond+i/4,result.at(i+2));
                        int a;
                        string b;
                        float c;
                        string s = result.at(i+3);
                        stringstream ss;
                        if(s.at(0)=='\''&&s.at(s.size()-1)=='\'')
                        {
                            b = s.substr(1,s.size()-2);
                            x.v[i/4].setString(b);
                        }
                        else
                        {
                            ss << s;
                            if(s.find('.')!=string::npos)
                            {
                                ss >> c;
                                x.v[i/4].setFloat(c);
                            }
                            else
                            {
                                ss >> a;
                                x.v[i/4].setInt(a);
                            }	
                        }	
                        
                    }
                    else x.correct = 0;
                }
            }
            else if(result.size()==3) x.condAmount = 0;
            else x.correct = 0;					
        }
        else x.correct = 0;
    }
    else if(result.at(0)=="insert")
    {
        x.type = INSERT;
        if(result.at(1)=="into")
        {
            x.name = result.at(2);					
            if(result.size()==4)
            {
                if(result.at(3).find("values(")==0&&result.at(3).rfind(")")==result.at(3).size()-1)
                {
                    string s = result.at(3).substr(7,result.at(3).size()-8);
                    x.value = split(s,",");
                }
                else x.correct = 0;		
            }
            else x.correct = 0;				
        }
        else x.correct = 0;
    }
    else
    {
        x.correct = 0;
    }
    return ;
}
//验证drop database语句是否有效
void drop_database(string SQL,int start, SQL_CLAUSE & sql_cla)
{
    int pos1, pos2, pos3;
    pos1 = SQL.find_first_not_of(' ', start);
    pos2 = SQL.find_first_of(' ', pos1);
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
    {
        sql_cla.type = ERROR;
        return ;
    }
    sql_cla.type = DROPDATABASE;
    sql_cla.name = SQL.substr(pos1, pos2-pos1);
    return ;
}

//验证drop table语句是否有效
void drop_table(string SQL,int start, SQL_CLAUSE & sql_cla)
{
    int pos1, pos2, pos3;
    pos1 = SQL.find_first_not_of(' ', start);
    pos2 = SQL.find_first_of(' ', pos1);
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
    {
        sql_cla.type = ERROR;
        return ;
    }
    sql_cla.type = DROPTABLE;
    sql_cla.name = SQL.substr(pos1, pos2-pos1);
    return ;
}

//验证drop index语句是否有效
void drop_index(string SQL,int start, SQL_CLAUSE & sql_cla)
{
    int pos1, pos2, pos3;
    pos1 = SQL.find_first_not_of(' ', start);
    pos2 = SQL.find_first_of(' ', pos1);
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
    {
        sql_cla.type = ERROR;
        return ;
    }
    sql_cla.type = DROPINDEX;
    sql_cla.name = SQL.substr(pos1, pos2-pos1);
    return ;
}

//验证drop语句是否有效
SQL_CLAUSE drop_clause(string SQL,int start)
{
    SQL_CLAUSE ret;
    string st;
    int pos1, pos2;
    while ((pos1 = SQL.find_first_of('\t')) != string::npos)
        SQL.replace(pos1, 1, " ");
    while (SQL.at(0) == ' ')
        SQL.erase(0, 1);
    
    pos1 = SQL.find_first_not_of(' ', 0);
    pos2 = SQL.find_first_of(' ', pos1);
    if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "drop")
    {
        ret.type = ERROR;
        return ret;
    }
    pos1 = SQL.find_first_not_of(' ', pos2);
    pos2 = SQL.find_first_of(' ', pos1);
    if (pos1 == string::npos || pos2 == string::npos)
    {
        ret.type = ERROR;
        return ret;
    }
    if (SQL.substr(pos1, pos2-pos1) == "database")
    {
        drop_database(SQL, pos2, ret);
        return ret;
    }
    if (SQL.substr(pos1, pos2-pos1) == "table")
    {
        drop_table(SQL, pos2, ret);
        return ret;
    }
    if (SQL.substr(pos1, pos2-pos1) == "index")
    {
        drop_index(SQL, pos2, ret);
        return ret;
    }
    ret.type = ERROR;
    return ret;
}

//验证use语句是否有效
SQL_CLAUSE use_clause(string SQL,int start)
{
    SQL_CLAUSE ret;
    string st;
    int pos1, pos2, pos3;
    while ((pos1 = SQL.find_first_of('\t')) != string::npos)
        SQL.replace(pos1, 1, " ");
    while (SQL.at(0) == ' ')
        SQL.erase(0, 1);
    
    pos1 = SQL.find_first_not_of(' ', 0);
    pos2 = SQL.find_first_of(' ', pos1);
    if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "use")
    {
        ret.type = ERROR;
        return ret;
    }
    pos1 = SQL.find_first_not_of(' ', pos2);
    pos2 = SQL.find_first_of(' ', pos1);
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
    {
        ret.type = ERROR;
        return ret;
    }
    ret.type = USE;
    ret.name = SQL.substr(pos1, pos2-pos1);
    return ret;
}

//验证execfile语句是否有效
SQL_CLAUSE execfile_clause(string SQL,int start)
{
    SQL_CLAUSE ret;
    string st;
    int pos1, pos2, pos3;
    while ((pos1 = SQL.find_first_of('\t')) != string::npos)
        SQL.replace(pos1, 1, " ");
    while (SQL.at(0) == ' ')
        SQL.erase(0, 1);
    
    pos1 = SQL.find_first_not_of(' ', 0);
    pos2 = SQL.find_first_of(' ', pos1);
    if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "execfile")
    {
        ret.type = ERROR;
        return ret;
    }
    pos1 = SQL.find_first_not_of(' ', pos2);
    pos2 = SQL.find_first_of(' ', pos1);
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
    {
        ret.type = ERROR;
        return ret;
    }
    ret.type = EXECFILE;
    ret.name = SQL.substr(pos1, pos2-pos1);
    return ret;
}

//验证quit语句是否有效
SQL_CLAUSE quit_clause(string SQL,int start)
{
    SQL_CLAUSE ret;
    string st;
    int pos1, pos2, pos3;
    while ((pos1 = SQL.find_first_of('\t')) != string::npos)
        SQL.replace(pos1, 1, " ");
    while (SQL.at(0) == ' ')
        SQL.erase(0, 1);
    
    pos1 = SQL.find_first_not_of(' ', 0);
    pos2 = SQL.find_first_of(' ', pos1);
    if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "use")
    {
        ret.type = ERROR;
        return ret;
    }
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos3 == string::npos || SQL.at(pos3) != ';')
    {
        ret.type = ERROR;
        return ret;
    }
    ret.type = QUIT;
    return ret;
}

//验证help语句是否有效
SQL_CLAUSE help_clause(string SQL,int start)
{
    SQL_CLAUSE ret;
    string st;
    int pos1, pos2, pos3;
    while ((pos1 = SQL.find_first_of('\t')) != string::npos)
        SQL.replace(pos1, 1, " ");
    while (SQL.at(0) == ' ')
        SQL.erase(0, 1);
    
    pos1 = SQL.find_first_not_of(' ', 0);
    pos2 = SQL.find_first_of(' ', pos1);
    if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "use")
    {
        ret.type = ERROR;
        return ret;
    }
    pos3 = SQL.find_first_not_of(' ', pos2);
    if (pos3 == string::npos || SQL.at(pos3) != ';')
    {
        ret.type = ERROR;
        return ret;
    }
    ret.type = QUIT;
    return ret;
}
void ConvertToArray(TableAttr* &att, vector<TableAttr> &attr)
{
    for (int i=0; i<attr.size(); i++) {
        att[i]=attr[i];
    }
    return;
}
