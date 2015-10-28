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
#include "BufferManager.h"
#include "Value.hpp"
using namespace std;

#define BLOCKBYTES sizeof(int)
typedef unsigned char byte;

typedef struct {
    byte isLeaf;
    int father;
    int left;
    short nodeNumber;
    byte reserve;
} BLOCKHEADER;

//
#define INDEXFILE 3

typedef enum {
    EQUAL,
    LESS,
    LESSEQUAL,
    GREAT,
    GREATEQUAL
} Condition;
//

class IndexManager {
private:
    int blockNumber;
    int rootBlock;
    int attributeLen;
    short maxNode;
    
    void writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode);
    void readIndexFileHeader(char *block);
    BLOCKHEADER readBlockHeader(char *block);
    byte readByte(byte *block, int &fp);
    short readShort(byte *block, int &fp);
    int readInt(byte *block, int &fp);
    float readFloat(byte *block, int &fp);
    string readString(byte *block, int &fp, int length);
    blockInfo *searchInTree(string DBName, string tableName, string indexName, int rootNo, Value attributeValue);
public:
    int create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets);
    int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
    int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
    int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<char> &results);
    int drop(string DBName, string tableName, string indexName);
};

#endif /* IndexManager_hpp */
