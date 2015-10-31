//
//  BplusTree.hpp
//  DBMS
//
//  Created by 张函祎 on 15/10/31.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#ifndef BplusTree_hpp
#define BplusTree_hpp

#include <stdio.h>
#include <string>
#include <vector>
#include "BufferManager.h"
#include "DataTransfer.hpp"
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

class BplusTree {
private:
    string currentDB;
    string currentTable;
    string currentIndex;
    int blockNumber;                        // k blocks + 1 indexFileHeader
    int rootBlock;
    int attributeLen;
    short maxNode;                          // even
    enum Value::AttributeType type;
    
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
    Value readValue(byte *block, int &fp);
    void writeValue(Value value, byte *block, int &fp);
public:
    int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
    int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
    int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results);
    
    void readIndexFileHeader(byte *block);
    void writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attributeType);
    BLOCKHEADER *readBlockHeader(byte *block);
    void writeBlockHeader(int blockNo, byte isLeaf, int father, int left, short nodeNumber);
};

#endif /* BplusTree_hpp */
