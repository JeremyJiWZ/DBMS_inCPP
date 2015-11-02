//
//  Value.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#include "Value.h"

Value::Value() {
}

Value::Value(int _intValue,string _ValueName) {
    type = INT;
    intValue = _intValue;
    ValueName = _ValueName;
}

Value::Value(float _floatValue,string _ValueName) {
    type = FLOAT;
    floatValue = _floatValue;
    ValueName = _ValueName;
}

Value::Value(string _stringValue,string _ValueName) {
    type = STRING;
    stringValue = _stringValue;
    ValueName = _ValueName;
}

void Value::setInt(int _intValue) {
	type = INT;
    intValue = _intValue;
}

void Value::setFloat(float _floatValue) {
	type = FLOAT;
    floatValue = _floatValue;
}

void Value::setString(string _stringValue){
	type = STRING;
    stringValue = _stringValue;
}

void Value::setValueName(string _ValueName){
	ValueName = _ValueName;
}

bool Value::isEqualTo(Value value) {
    switch (type) {
        case INT:
            return (intValue == value.intValue);
            break;
        case FLOAT:
            return (floatValue == value.floatValue);
            break;
        case STRING:
            return (stringValue == value.stringValue);
            break;
    }
}

bool Value::isNotEqualTo(Value value) {
    return !isEqualTo(value);
}

bool Value::isLessThan(Value value) {
    switch (type) {
        case INT:
            return (intValue<value.intValue);
            break;
        case FLOAT:
            return (floatValue<value.floatValue);
            break;
        case STRING:
            return (stringValue<value.stringValue);
            break;
    }
}

bool Value::isGreatThan(Value value) {
    switch (type) {
        case INT:
            return (intValue>value.intValue);
            break;
        case FLOAT:
            return (floatValue>value.floatValue);
            break;
        case STRING:
            return (stringValue>value.stringValue);
            break;
    }
}

bool Value::isLessEqualTo(Value value) {
    return !isGreatThan(value);
}

bool Value::isGreatEqualTo(Value value) {
    return !isLessThan(value);
}
