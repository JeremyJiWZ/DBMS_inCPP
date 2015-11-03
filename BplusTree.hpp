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
    int left;                               // valid only for leaves
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
    void deleteNode(blockInfo *block, Value value);
    void deleteNode(blockInfo *block, int ptr);
    void handleTooFewNodes(BLOCKHEADER *blockHeader, blockInfo *block);
    void coalesce(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock);
    void redistributeFromLeftToRight(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock);
    void redistributeFromRightToLeft(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock);
    void adjustRoot(BLOCKHEADER *blockHeader, blockInfo *block);
    
    // Utility
    blockInfo *searchInTree(int rootNo, Value attributeValue);
    short findPosition(byte *block, Value benchmark);
    short findPosition(byte *block, int ptr);
    int findRightNeighbor(byte *block, int ptr);
    void copyNodesWithPointerFirst(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n);
    void copyNodesWithValueFirst(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n);
    void addNodeWithPointerFirst(byte *block, int pos, short n, Value value, int ptr);
    void addNodeWithValueFirst(byte *block, int pos, short n, Value value, int ptr);
    void updateFatherPointer(short n, byte *data, int fatherNo);
    Value readValue(byte *block, int &fp);
    void writeValue(Value value, byte *block, int &fp);
    void printValue(byte *block, int &fp);
public:
    int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
    int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
    int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results);
    
    void readIndexFileHeader(byte *block);
    void writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attributeType);
    BLOCKHEADER *readBlockHeader(byte *block);
    void writeBlockHeader(int blockNo, byte isLeaf, int father, int left, short nodeNumber);
    
    void printTree();
};

#endif /* BplusTree_hpp */
