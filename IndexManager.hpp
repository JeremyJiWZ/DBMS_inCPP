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
    enum Value::AttributeType type;
    
    // Read/Write Data
    void readIndexFileHeader(char *block);
    void writeIndexFileHeader(string DBName, string tableName, string indexName, int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attributeType);
    BLOCKHEADER *readBlockHeader(char *block);
    void writeBlockHeader(string DBName, string tableName, string indexName, int blockNo, byte isLeaf, int father, int left, short nodeNumber);
    byte readByte(byte *block, int &fp);
    void writeByte(byte byteData, byte *block, int &fp);
    short readShort(byte *block, int &fp);
    void writeShort(short shortData, byte *block, int &fp);
    int readInt(byte *block, int &fp);
    void writeInt(int intData, byte *block, int &fp);
    float readFloat(byte *block, int &fp);
    void writeFloat(float floatData, byte *block, int &fp);
    string readString(byte *block, int &fp, int length);
    void writeString(string stringData, byte *block, int &fp, int length);
    
    // Insertion
    int insertIntoLeafWithoutSplit(byte *block, Value attributeValue, int recordOffset);
    int insertIntoLeafWithSplit(byte *block, Value attributeValue, int recordOffset);
    int insertIntoInternalWithoutSplit(byte *block, Value indexValue, int indexOffset);
    int insertIntoInternalWithSplit(byte *block, Value indexValue, int indexOffset);
    int insertIntoParent(int left, int right, Value indexValue);
    int insertIntoRoot(int left, int right, Value rootValue);
    
    blockInfo *searchInTree(string DBName, string tableName, string indexName, int rootNo, Value attributeValue);
    void copyNodes(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n);
public:
    int create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets);
    int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
    int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
    int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<char> &results);
    int drop(string DBName, string tableName, string indexName);
};

#endif /* IndexManager_hpp */
