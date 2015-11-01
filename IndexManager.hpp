//
//  IndexManager.hpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#ifndef IndexManager_hpp
#define IndexManager_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "BplusTree.hpp"
using namespace std;

class IndexManager {
private:
    BplusTree tree;
public:
    int create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets);
    int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
    int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
    int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results);
    int drop(string DBName, string tableName, string indexName);
};

#endif /* IndexManager_hpp */
