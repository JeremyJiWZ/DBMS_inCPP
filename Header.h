//
//  Header.h
//  t123
//
//  Created by mr.ji on 15/11/4.
//  Copyright (c) 2015年 mr.ji. All rights reserved.
//

#ifndef t123_Header_h
#define t123_Header_h
#include<string>
#include<iostream>
using namespace std;
//验证create table语句是否有效
string get_attribute(string temp,string sql)
{
    int start=0,end,index;
    string T,C;
    temp+=" ";
    //获得属性名
    end=temp.find(' ',start);
    T=temp.substr(start,end-start);
    start=end+1;
    sql+=T+" ";
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
        sql="99";
        return sql;
    }
    //若为int
    else if(T=="int")
        sql+="+";
    //若为float
    else if(T=="float")
        sql+="-";
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
            sql="99";
            return sql;
        }
        //若为char
        else if(C=="char")
        {
            C=T.substr(index,T.length()-index-1);
            if(C == "")
            {
                cout<<"error: the length of the data type char has not been specified!"<<endl;
                sql="99";
                return sql;
            }
            else
                sql+=C;
        }
        //若为非法信息，打印出错信息
        else
        {
            cout<<"error: "<<C<<"---is not a valid key word!"<<endl;
            sql="99";
            return sql;
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
            sql+=" 1,";
        }
        //若为非法信息，打印出错信息
        else
        {
            cout<<"error: "<<temp<<"---is not a valid key word!"<<endl;
            sql="99";
            return sql;
        }
    }
    //若无附加信息
    else
        sql+=" 0,";
    return sql;
}
string create_table(string SQL,int start)
{
    string temp,sql,T;
    int index,end,length;
    //获取表名
    while(SQL.at(start)==' ')
        start++;
    index=start;
    if((end=SQL.find('(',start))==-1)
    {
        cout<<"error: missing ( in the statement!"<<endl;
        SQL="99";
        return SQL;
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
    }
    //若无，打印出错信息
    if(temp == "")
    {
        cout<<"error: error in create table statement!"<<endl;
        SQL="99";
        return SQL;
    }
    //若为非法信息，打印出错信息
    else if(temp.find(' ')!=-1)
    {
        cout<<"error: "<<temp<<"---is not a valid table name!"<<endl;
        SQL="99";
        return SQL;
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
                SQL="99";
                return SQL;
            }
            //保存属性
            else
            {
                sql=get_attribute(temp,sql);
                if(sql=="99")
                    return sql;
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
            SQL="99";
            return SQL;
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
                    SQL="99";
                    return SQL;
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
                        SQL="99";
                        return SQL;
                    }
                    //若为非法信息，打印出错信息
                    else if(T.find(' ')!=-1)
                    {
                        cout<<"error : "<<T<<"---is not a valid primary key attribute name!"<<endl;
                        SQL="99";
                        return SQL;
                    }
                    //保存主键
                    else
                    {
                        sql+=T+" #,";
                        SQL="01"+sql;
                    }
                }
                //若为非法信息，打印出错信息
                else
                {
                    cout<<"error : "<<T<<"---is not a valid key word!"<<endl;
                    SQL="99";
                    return SQL;
                }
            }
            //若为一般属性
            else
            {
                sql=get_attribute(temp,sql);
                if(sql=="99")
                {
                    SQL="99";
                    return SQL;
                }
                else
                    SQL="01"+sql;
            }
        }
    }
    return SQL;
}

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
    return SQL;
}

#endif
