//  Value.hpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#ifndef Value_h
#define Value_h

#include <stdio.h>
#include <string>
using namespace std;

class Value {
public:
    enum AttributeType {
        INT = 1,
        FLOAT,
        STRING
    } type;
    int intValue;
    float floatValue;
    string stringValue;
    string ValueName;

    Value();
    Value(int intValue,string ValueName=NULL);
    Value(float floatValue,string ValueName=NULL);
    Value(string stringValue,string ValueName=NULL);
    void setInt(int intValue);
    void setFloat(float floatValue);
    void setString(string stringValue);
    void setValueName(string ValueName); 
    bool isEqualTo(Value value);
    bool isNotEqualTo(Value value);
    bool isLessThan(Value value);
    bool isLessEqualTo(Value value);
    bool isGreatThan(Value value);
    bool isGreatEqualTo(Value value);
};

#include "Value.cpp" 
#endif /* Value_hpp */
