//
//  IndexManager.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#include "IndexManager.hpp"

#define SUCCESS 1
#define FAIL 0

int IndexManager::create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets) {
    blockNumber = 1;
    rootBlock = 1;
    attributeLen = attributeBytes;
    maxNode = (BLOCK_LEN-sizeof(BLOCKHEADER)-BLOCKBYTES)/(BLOCKBYTES+attributeBytes);
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode);
    
    vector<Value>::iterator valueItor = attributeValues.begin();
    vector<int>::iterator offsetItor = recordOffsets.begin();
    while (valueItor!=attributeValues.end() && offsetItor!=recordOffsets.end()) {
        insertInto(DBName, tableName, indexName, *valueItor, *offsetItor);
        valueItor++;
        offsetItor++;
    }
    return SUCCESS;
}

int IndexManager::insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset) {
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader(block->cBlock);
    blockInfo *leaf = searchInTree(DBName, tableName, indexName, rootBlock, attributeValue);
    return SUCCESS;
}

int IndexManager::deleteFrom(string DBName, string tableName, string indexName, Value attributeValue) {
    blockInfo *block;
    block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader(block->cBlock);
    blockInfo *leaf = searchInTree(DBName, tableName, indexName, rootBlock, attributeValue);
    return SUCCESS;
}

int IndexManager::select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<char> &results) {
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader(block->cBlock);
    blockInfo *leaf = searchInTree(DBName, tableName, indexName, rootBlock, attributeValue);
    return SUCCESS;
}

int IndexManager::drop(string DBName, string tableName, string indexName) {
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader(block->cBlock);
    return SUCCESS;
}

blockInfo *IndexManager::searchInTree(string DBName, string tableName, string indexName, int rootNo, Value attributeValue) {
    blockInfo *block;
    int blockNo = rootNo;
    while (true) {
        block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, blockNo);
        BLOCKHEADER blockHeader = readBlockHeader(block->cBlock);
        if (blockHeader.isLeaf)
            break;
        int fp = sizeof(BLOCKHEADER);
        short nodeNo = 1;
        while (nodeNo<=blockHeader.nodeNumber) {
            int leftSon = readInt((byte *)block->cBlock, fp);
            Value value;
            switch (attributeValue.type) {
                case Value::INT:
                    value.setInt(readInt((byte *)block->cBlock, fp));
                    break;
                case Value::FLOAT:
                    value.setFloat(readFloat((byte *)block->cBlock, fp));
                    break;
                case Value::STRING:
                    value.setString(readString((byte *)block->cBlock, fp, attributeLen));
                    break;
                default:
                    break;
            }
            if (attributeValue.isLessThan(value)) {
                blockNo = leftSon;
                break;
            }
            nodeNo++;
        }
        if (nodeNo>blockHeader.nodeNumber)                      //last son
            blockNo = readInt((byte *)block->cBlock, fp);
    }
    return block;
}

void IndexManager::writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode) {
    
}

byte IndexManager::readByte(byte *block, int &fp) {
    byte *data = block+fp;
    fp += sizeof(byte);
    return *data;
}

short IndexManager::readShort(byte *block, int &fp) {
    short *data = (short *)(block+fp);
    fp += sizeof(short);
    return *data;
}

int IndexManager::readInt(byte *block, int &fp) {
    int *data = (int *)(block+fp);
    fp += sizeof(int);
    return *data;
}

float IndexManager::readFloat(byte *block, int &fp) {
    float *data = (float *)(block+fp);
    fp += sizeof(float);
    return *data;
}

string IndexManager::readString(byte *block, int &fp, int length) {
    char data[256];
    strncpy(data, (char *)(block+fp), length);
    fp += length;
    return string(data);
}
