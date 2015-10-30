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
    int blockNumber;                        // k blocks + 1 indexFileHeader
    int rootBlock;
    int attributeLen;
    short maxNode;                          // even
    enum Value::AttributeType type;
    string currentDB;
    string currentTable;
    string currentIndex;
    
    // Read/Write Data
    void readIndexFileHeader(byte *block);
    void writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attributeType);
    BLOCKHEADER *readBlockHeader(byte *block);
    void writeBlockHeader(int blockNo, byte isLeaf, int father, int left, short nodeNumber);
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
    Value readValue(byte *block, int &fp);
    void writeValue(Value value, byte *block, int &fp);
    
    // Insertion
    void insertIntoLeafWithoutSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset);
    void insertIntoLeafWithSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset);
    void insertIntoInternalWithoutSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset);
    void insertIntoInternalWithSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset);
    void insertIntoParent(blockInfo *block, Value indexValue);
    void insertIntoRoot(blockInfo *block, Value rootValue);
    
    // Deleteion
    int coalesce(int neighbor);
    int redistribute(int neighbor);
    
    blockInfo *searchInTree(int rootNo, Value attributeValue);
    short findPisition(byte *block, Value benchmark);
    void copyNodes(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n);
    void addNodeWithPointerFirst(byte *block, int pos, short n, Value value, int ptr);
    void addNodeWithValueFirst(byte *block, int pos, short n, Value value, int ptr);
public:
    int create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets);
    int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
    int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
    int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results);
    int drop(string DBName, string tableName, string indexName);
};

#endif /* IndexManager_hpp */
