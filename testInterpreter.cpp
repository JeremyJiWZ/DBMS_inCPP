//
//  testInterpreter.cpp
//  DBMSSev
//
//  Created by mr.ji on 15/11/6.
//  Copyright (c) 2015年 mr.ji. All rights reserved.
//

#include "Interpreter.h"
#include "Value.hpp"
#include "CatalogManager.h"
#include <iostream>
using namespace std;
int main ()
{
    string sql;
    SQL_CLAUSE sql_cla;
    
    Interpreter(sql_cla);
    cout<<sql_cla.type<<endl;
    cout<<sql_cla.name<<endl;
    cout<<sql_cla.tableName<<endl;
    cout<<sql_cla.attrName<<endl;
    cout<<sql_cla.attrAmount<<endl;
    
    
    
}