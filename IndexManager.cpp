//
//  IndexManager.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//
//***************************************
//  Nodes of B+Tree are called blocks.
//  Nodes in blocks are called nodes. Count starts from 1.
//  Total number of XXX is called XXXNumber.
//  Ordinal of XXX is called XXXNo.
//  Index file header is in No.0 block

#include "IndexManager.hpp"

#define SUCCESS 1
#define FAIL 0

int IndexManager::create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets) {
    tree.init(DBName, tableName, indexName);
    int blockNumber = 1;
    int rootBlock = 1;
    int attributeLen = attributeBytes;
    short maxNode = (BLOCK_LEN-sizeof(BLOCKHEADER)-BLOCKBYTES)/(BLOCKBYTES+attributeBytes);
//    short maxNode = 10;
    if (maxNode%2 == 1)
        maxNode--;
    int attributeType;
    switch (attributeValues.front().type) {
        case Value::INT:
            attributeType = 1;
            break;
        case Value::FLOAT:
            attributeType = 2;
            break;
        case Value::STRING:
            attributeType = 3;
            break;
        default:
            break;
    }
    tree.writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, attributeType);
    tree.writeBlockHeader(1, 1, 0, 0, 0);
    blockInfo *root = BufferManager::get_file_block(DBName, indexName, INDEXFILE, 1);
    int fp = sizeof(BLOCKHEADER);
    DataTransfer::writeInt(0, (byte *)root->cBlock, fp);
    root->dirtyBlock = true;
    
    vector<Value>::iterator valueItor = attributeValues.begin();
    vector<int>::iterator offsetItor = recordOffsets.begin();
    int count = 0;
    while (valueItor!=attributeValues.end() && offsetItor!=recordOffsets.end()) {
        cout<<(*valueItor).intValue<<":"<<(*offsetItor)<<"  "<<count<<endl;
        insertInto(DBName, tableName, indexName, *valueItor, *offsetItor);
        valueItor++;
        offsetItor++;
        count++;
//        tree.printTree(count);
    }
    return SUCCESS;
}

int IndexManager::insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset) {
    tree.insertInto(DBName, tableName, indexName, attributeValue, recordOffset);
    return SUCCESS;
}

int IndexManager::deleteFrom(string DBName, string tableName, string indexName, Value attributeValue) {
    tree.deleteFrom(DBName, tableName, indexName, attributeValue);
    return SUCCESS;
}

int IndexManager::select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results) {
    tree.select(DBName, tableName, indexName, attributeValue, cond, results);
    return SUCCESS;
}

int IndexManager::drop(string DBName, string tableName, string indexName) {
    BufferManager::DeleteFile(DBName, indexName, INDEXFILE);
    return SUCCESS;
}
