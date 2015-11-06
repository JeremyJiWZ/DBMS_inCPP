//
//  Interpreter.h
//  DBMSSev
//
//  Created by mr.ji on 15/11/5.
//  Copyright (c) 2015年 mr.ji. All rights reserved.
//

#ifndef DBMSSev_Interpreter_h
#define DBMSSev_Interpreter_h

#include<iostream>
#include<string>
#include<vector>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include"CatalogManager.h"
#include"Value.hpp"
#include"Condition.h"

using namespace std;
//定义SQL语句类型的宏
#define CREATEDATABASE 0
#define CREATETABLE 1
#define CREATEINDEX 2
#define DROPDATABASE 3
#define DROPTABLE 4
#define DROPINDEX 5
#define SELECT 6 //无where
#define SELECT_WHERE 7 //有where
#define INSERT 8
#define DELETE 9
#define DELETE_WHERE 10
#define USE 11
#define EXECFILE 12
#define QUIT 13
#define HELP 14
#define ERROR 99
struct SQL_CLAUSE
{
    bool correct;//语句是否正确
    int type;//语句类型
    string name;//名字
    string tableName;//create index时的表名
    string attrName;//create index时的属性名
    int attrAmount;//属性数量
    vector<TableAttr> attr;//属性信息
    TableAttr * att;//属性信息，转化为数组
    Value *v;   //INDEX?
    vector<string> value;//insert的值
    Condition *cond;//where后面的信息
    int condAmount;//条件数
};
//获取用户输入，并对输入作有效性检查，若正确，返回语句的内部表示形式
SQL_CLAUSE Interpreter();
//读取用户输入
string read_input();//OK
//验证create语句是否有效
SQL_CLAUSE create_clause(string SQL,int start);
//验证create database语句是否有效
SQL_CLAUSE create_database(string SQL,int start);
//验证create table语句是否有效
SQL_CLAUSE create_table(string SQL,int start);
//验证create index语句是否有效
SQL_CLAUSE create_index(string SQL,int start);
void ConvertToArray(TableAttr* &att,vector<TableAttr> &attr);
//验证drop语句是否有效
SQL_CLAUSE drop_clause(string SQL,int start);
//验证drop database语句是否有效
SQL_CLAUSE drop_database(string SQL,int start);
//验证drop table语句是否有效
SQL_CLAUSE drop_table(string SQl,int start);
//验证drop index语句是否有效
SQL_CLAUSE drop_index(string SQL,int start);

////
SQL_CLAUSE ExplainStatement(string SQL);
//
//
////将表达式转化为内部形式
//SQL_CLAUSE get_expression(string temp,string sql);
////获取表达式组的每个表达式
//SQL_CLAUSE get_each(string T,string sql,string condition);
////验证use语句是否有效
SQL_CLAUSE use_clause(string SQL,int start);
//验证execfile语句是否有效
SQL_CLAUSE execfile_clause(string SQL,int start);
//验证quit语句是否有效
SQL_CLAUSE quit_clause(string SQL,int start);

#endif
