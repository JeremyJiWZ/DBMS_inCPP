//
//  Value.hpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#ifndef Value_hpp
#define Value_hpp

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

    Value();
    Value(int intValue);
    Value(float floatValue);
    Value(string stringValue);
    void setInt(int intValue);
    void setFloat(float floatValue);
    void setString(string stringValue);
    bool isLessThan(Value value);
};

#endif /* Value_hpp */