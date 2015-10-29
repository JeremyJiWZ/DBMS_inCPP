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

#include "IndexManager.hpp"

#define SUCCESS 1
#define FAIL 0

int IndexManager::create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets) {
    blockNumber = 1;
    rootBlock = 1;
    attributeLen = attributeBytes;
    maxNode = (BLOCK_LEN-sizeof(BLOCKHEADER)-BLOCKBYTES)/(BLOCKBYTES+attributeBytes);
    if (maxNode%2 == 1)
        maxNode--;
    byte attributeType;
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
    writeIndexFileHeader(DBName, tableName, indexName, blockNumber, rootBlock, attributeLen, maxNode, attributeType);
    
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
    BLOCKHEADER *blockHeader = readBlockHeader(leaf->cBlock);
    if (blockHeader->nodeNumber == maxNode) {
        //split;
    } else {
        insertIntoLeafWithoutSplit((byte *)leaf->cBlock, attributeValue, recordOffset);
        blockHeader->nodeNumber++;
    }
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

#pragma mark -
#pragma mark Insertion

int IndexManager::insertIntoLeafWithoutSplit(byte *block, Value attributeValue, int recordOffset) {
    return SUCCESS;
}

#pragma mark -
#pragma mark Utilization

blockInfo *IndexManager::searchInTree(string DBName, string tableName, string indexName, int rootNo, Value attributeValue) {
    blockInfo *block;
    int blockNo = rootNo;
    while (true) {
        block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, blockNo);
        BLOCKHEADER *blockHeader = readBlockHeader(block->cBlock);
        if (blockHeader->isLeaf)
            break;
        int fp = sizeof(BLOCKHEADER);
        short nodeNo = 1;
        while (nodeNo<=blockHeader->nodeNumber) {
            int leftSon = readInt((byte *)block->cBlock, fp);
            Value value;
            switch (type) {
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
        if (nodeNo>blockHeader->nodeNumber)                      //last son
            blockNo = readInt((byte *)block->cBlock, fp);
    }
    return block;
}

void IndexManager::copyNodes(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n) {
    for (int i=0; i<n; i++) {
        switch (type) {
            case Value::INT:
                break;
            case Value::FLOAT:
                break;
            case Value::STRING:
                break;
            default:
                break;
        }
    }
}

#pragma mark -
#pragma mark Read/Write Data

void IndexManager::writeIndexFileHeader(string DBName, string tableName, string indexName, int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attribyteType) {
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    int fp = 0;
    writeInt(blockNumber, (byte *)block->cBlock, fp);
    writeInt(rootNumber, (byte *)block->cBlock, fp);
    writeInt(attributeLen, (byte *)block->cBlock, fp);
    writeShort(maxNode, (byte *)block->cBlock, fp);
}

void IndexManager::writeBlockHeader(string DBName, string tableName, string indexName, int blockNo, byte isLeaf, int father, int left, short nodeNumber) {
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, blockNo);
    int fp = 0;
    writeByte(isLeaf, (byte *)block->cBlock, fp);
    writeInt(father, (byte *)block->cBlock, fp);
    writeInt(left, (byte *)block->cBlock, fp);
    writeShort(nodeNumber, (byte *)block->cBlock, fp);
}

byte IndexManager::readByte(byte *block, int &fp) {
    byte *data = block+fp;
    fp += sizeof(byte);
    return *data;
}

void IndexManager::writeByte(byte byteData, byte *block, int &fp) {
    byte *data = (byte *)(block+fp);
    *data = byteData;
    fp += sizeof(byteData);
}

short IndexManager::readShort(byte *block, int &fp) {
    short *data = (short *)(block+fp);
    fp += sizeof(short);
    return *data;
}

void IndexManager::writeShort(short shortData, byte *block, int &fp) {
    short *data = (short *)(block+fp);
    *data = shortData;
    fp += sizeof(shortData);
}

int IndexManager::readInt(byte *block, int &fp) {
    int *data = (int *)(block+fp);
    fp += sizeof(int);
    return *data;
}

void IndexManager::writeInt(int intData, byte *block, int &fp) {
    int *data = (int *)(block+fp);
    *data = intData;
    fp += sizeof(intData);
}

float IndexManager::readFloat(byte *block, int &fp) {
    float *data = (float *)(block+fp);
    fp += sizeof(float);
    return *data;
}

void IndexManager::writeFloat(float floatData, byte *block, int &fp) {
    float *data = (float *)(block+fp);
    *data = floatData;
    fp += sizeof(floatData);
}

string IndexManager::readString(byte *block, int &fp, int length) {
    char data[256];
    strncpy(data, (char *)(block+fp), length);
    fp += length;
    return string(data);
}

void IndexManager::writeString(string stringData, byte *block, int &fp, int length) {
    strncpy((char*)(block+fp), stringData.c_str(), length);
    fp += length;
}
