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
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockNumber = 1;
    rootBlock = 1;
#warning empty tree
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
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, attributeType);
    writeBlockHeader(1, 1, 0, 0, 0);
    
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
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    BLOCKHEADER *blockHeader = readBlockHeader((byte *)leaf->cBlock);
    if (blockHeader->nodeNumber == maxNode) {
        insertIntoLeafWithSplit(blockHeader, leaf, attributeValue, recordOffset);
    } else {
        insertIntoLeafWithoutSplit(blockHeader, leaf, attributeValue, recordOffset);
    }
    return SUCCESS;
}

int IndexManager::deleteFrom(string DBName, string tableName, string indexName, Value attributeValue) {
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockInfo *block;
    block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    return SUCCESS;
}

int IndexManager::select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results) {
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockInfo *block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    byte *data = (byte *)leaf->cBlock;
    short pos = findPisition(data, attributeValue);
    int fpPtr = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen);
    int fpValue = fpPtr+BLOCKBYTES;
    int offset = readInt(data, fpPtr);
    Value benchmark = readValue(data, fpValue);
    int start = 1, end = maxNode;
    switch (cond) {
        case EQUAL:
            if (attributeValue.isEqualTo(benchmark)) {
                results.push_back(offset);
                return SUCCESS;
            } else
                return FAIL;
            break;
        case LESS:
            end = pos-1;
            break;
        case LESSEQUAL:
            if (attributeValue.isEqualTo(benchmark))
                results.push_back(offset);
            end = pos-1;
            break;
        case GREAT:
            if (attributeValue.isNotEqualTo(benchmark))
                results.push_back(offset);
            start = pos+1;
            break;
        case GREATEQUAL:
            start = pos;
            break;
        default:
            break;
    }
#warning the most left and most right leaf
    return SUCCESS;
}

int IndexManager::drop(string DBName, string tableName, string indexName) {
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockInfo *block = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    return SUCCESS;
}

#pragma mark -
#pragma mark Insertion

void IndexManager::insertIntoLeafWithoutSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset) {
    byte *leaf = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(leaf);
    short pos = findPisition(leaf, attributeValue);
    addNodeWithPointerFirst(leaf, pos, blockHeader->nodeNumber-pos+1, attributeValue, recordOffset);
    leafHeader->nodeNumber++;
}

void IndexManager::insertIntoLeafWithSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset) {
    blockNumber++;
    blockInfo *newBlock = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, blockNumber);
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);
    writeBlockHeader(blockNumber, 1, leafHeader->father, block->blockNum, leafHeader->nodeNumber/2);
    
    // move the half
    byte *newLeaf = (byte *)newBlock->cBlock;
    byte *leaf = (byte *)block->cBlock;
    copyNodes(newLeaf, 1, leaf, leafHeader->nodeNumber/2+1, maxNode/2);
    leafHeader->nodeNumber /= 2;

    // change right pointer
    int fp = sizeof(BLOCKHEADER)+(maxNode/2)*(BLOCKBYTES+attributeLen);
    writeInt(blockNumber, leaf, fp);
    
    // insert
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    Value value = readValue(newLeaf, fp);
    if (attributeValue.isLessThan(value))
        insertIntoLeafWithoutSplit(leafHeader, block, attributeValue, recordOffset);
    else
        insertIntoLeafWithoutSplit(readBlockHeader(newLeaf), newBlock, attributeValue, recordOffset);
    
    insertIntoParent(newBlock, value);
}

void IndexManager::insertIntoInternalWithoutSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset) {
    byte *internal = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(internal);
    short pos = findPisition(internal, indexValue);
    addNodeWithValueFirst(internal, pos, blockHeader->nodeNumber-pos+1, indexValue, indexOffset);
    internalHeader->nodeNumber++;
}

void IndexManager::insertIntoInternalWithSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset) {
    blockNumber++;
    blockInfo *newBlock = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, blockNumber);
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);
    writeBlockHeader(blockNumber, 0, internalHeader->father, block->blockNum, internalHeader->nodeNumber/2);
    
    // move the half
    byte *newInternal = (byte *)newBlock->cBlock;
    byte *internal = (byte *)block->cBlock;
    copyNodes(newInternal, 1, internal, internalHeader->nodeNumber/2+1, maxNode/2);
    internalHeader->nodeNumber /= 2;
    
    // insert
    int fp = sizeof(BLOCKHEADER)+(maxNode/2-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    Value value = readValue(internal, fp);
    if (indexValue.isLessThan(value)) {
        short pos = findPisition(internal, indexValue);
        addNodeWithValueFirst(internal, pos, internalHeader->nodeNumber-pos, indexValue, indexOffset);
    } else {
        short pos = findPisition(internal, indexValue);
        if (pos == 1) {
            fp = sizeof(BLOCKHEADER);
            writeInt(indexOffset, newInternal, fp);
        } else {
            copyNodes(newInternal, 2, newInternal, 1, pos-2);
            fp = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
            writeValue(indexValue, newInternal, fp);
            writeInt(indexOffset, newInternal, fp);
        }
    }
    
    // update father ptr
    fp = sizeof(BLOCKHEADER);
    for (short i=0; i<=internalHeader->nodeNumber; i++) {
        int child = readInt(internal, fp);
        blockInfo *block = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, child);
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
        blockHeader->father = blockNumber;
        readValue(internal, fp);
    }
    
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    insertIntoParent(newBlock, readValue(newInternal, fp));
}

void IndexManager::insertIntoParent(blockInfo *block, Value indexValue) {
    int fatherNo = readBlockHeader((byte *)block->cBlock)->father;
    if (fatherNo == 0)
        insertIntoRoot(block, indexValue);
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, fatherNo);
    BLOCKHEADER *blockHeader = readBlockHeader((byte *)fatherBlock->cBlock);
    if (blockHeader->nodeNumber == maxNode) {
        insertIntoInternalWithSplit(blockHeader, fatherBlock, indexValue, block->blockNum);
    } else {
        insertIntoInternalWithoutSplit(blockHeader, fatherBlock, indexValue, block->blockNum);
    }
}

void IndexManager::insertIntoRoot(blockInfo *block, Value rootValue) {
    blockNumber++;
    blockInfo *newRoot = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, blockNumber);
    rootBlock = blockNumber;
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);
    blockInfo *oldRoot = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, rootBlock);
    
    // write 3 block headers
    BLOCKHEADER *blockHeader;
    blockHeader = readBlockHeader((byte *)oldRoot->cBlock);
    writeBlockHeader(oldRoot->blockNum, blockHeader->isLeaf, blockNumber, blockHeader->left, blockHeader->nodeNumber);
    blockHeader = readBlockHeader((byte *)block->cBlock);
    writeBlockHeader(block->blockNum, blockHeader->isLeaf, blockNumber, block->blockNum, blockHeader->nodeNumber);
    writeBlockHeader(blockNumber, 0, 0, 0, 1);
    
    // write root data
    int fp = sizeof(BLOCKHEADER);
    writeInt(oldRoot->blockNum, (byte *)newRoot->cBlock, fp);
    writeValue(rootValue, (byte *)newRoot->cBlock, fp);
    writeInt(block->blockNum, (byte *)newRoot->cBlock, fp);
}

#pragma mark -
#pragma mark Utilization

blockInfo *IndexManager::searchInTree(int rootNo, Value attributeValue) {
    blockInfo *block;
    int blockNo = rootNo;
    while (true) {
        block = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, blockNo);
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
        if (blockHeader->isLeaf)
            break;
        int fp = sizeof(BLOCKHEADER);
        short nodeNo = 1;
        while (nodeNo<=blockHeader->nodeNumber) {
            int leftSon = readInt((byte *)block->cBlock, fp);
            Value value = readValue((byte *)block->cBlock, fp);
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

short IndexManager::findPisition(byte *block, Value benchmark) {
    BLOCKHEADER *blockHeader =readBlockHeader(block);
    int fp = sizeof(BLOCKHEADER);
    short i;
    for (i=1; i<=blockHeader->nodeNumber; i++) {
        readInt(block, fp);
        Value value = readValue(block, fp);
        if (value.isGreatEqualTo(benchmark))
            break;
    }
    return i;
}

void IndexManager::copyNodes(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n) {
    int destfp = sizeof(BLOCKHEADER)+(destNo-1)*(BLOCKBYTES+attributeLen);
    int srcfp = sizeof(BLOCKHEADER)+(srcNo-1)*(BLOCKBYTES+attributeLen);
    int ptr;
    for (int i=0; i<n; i++) {
        ptr = readInt(srcBlock, srcfp);
        writeInt(ptr, destBlock, destfp);
        Value value = readValue(srcBlock, srcfp);
        writeValue(value, destBlock, destfp);
    }
    
    // move the tail ptr
    ptr = readInt(srcBlock, srcfp);
    writeInt(ptr, destBlock, destfp);
}

void IndexManager::addNodeWithPointerFirst(byte *block, int pos, short n, Value value, int ptr) {
    int ptrToAdd = ptr;
    Value valueToAdd = value;
    int fpRead = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen);
    for (short i=0; i<n; i++) {
        int fpWrite = fpRead;
        int tempPtr = readInt(block, fpRead);
        Value tempValue = readValue(block, fpRead);
        writeInt(ptrToAdd, block, fpWrite);
        writeValue(valueToAdd, block, fpWrite);
        ptrToAdd = tempPtr;
        valueToAdd = tempValue;
    }
    int fpWrite = fpRead;
    int tempPtr = readInt(block, fpRead);
    writeInt(ptrToAdd, block, fpWrite);
    writeValue(valueToAdd, block, fpWrite);
    writeInt(tempPtr, block, fpWrite);              // move the tail ptr
}

void IndexManager::addNodeWithValueFirst(byte *block, int pos, short n, Value value, int ptr) {
    Value valueToAdd = value;
    int ptrToAdd = ptr;
    int fpRead = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    for (short i=0; i<n; i++) {
        int fpWrite = fpRead;
        Value tempValue = readValue(block, fpRead);
        int tempPtr = readInt(block, fpRead);
        writeValue(valueToAdd, block, fpWrite);
        writeInt(ptrToAdd, block, fpWrite);
        valueToAdd = tempValue;
        ptrToAdd = tempPtr;
    }
    int fpWrite = fpRead;
    writeValue(valueToAdd, block, fpWrite);
    writeInt(ptrToAdd, block, fpWrite);
}

#pragma mark -
#pragma mark Read/Write Data

void IndexManager::readIndexFileHeader(byte *block) {
    int fp = 0;
    blockNumber = readInt(block, fp);
    rootBlock = readInt(block, fp);
    attributeLen = readInt(block, fp);
    maxNode = readShort(block, fp);
}

void IndexManager::writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attribyteType) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, 0);
    int fp = 0;
    writeInt(blockNumber, (byte *)block->cBlock, fp);
    writeInt(rootNumber, (byte *)block->cBlock, fp);
    writeInt(attributeLen, (byte *)block->cBlock, fp);
    writeShort(maxNode, (byte *)block->cBlock, fp);
}

void IndexManager::writeBlockHeader(int blockNo, byte isLeaf, int father, int left, short nodeNumber) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentTable, INDEXFILE, blockNo);
    int fp = 0;
    writeByte(isLeaf, (byte *)block->cBlock, fp);
    writeInt(father, (byte *)block->cBlock, fp);
    writeInt(left, (byte *)block->cBlock, fp);
    writeShort(nodeNumber, (byte *)block->cBlock, fp);
}

Value IndexManager::readValue(byte *block, int &fp) {
    switch (type) {
        case Value::INT:
            return Value(readInt(block, fp));
            break;
        case Value::FLOAT:
            return Value(readFloat(block, fp));
            break;
        case Value::STRING:
            return Value(readString(block, fp, attributeLen));
            break;
        default:
            return Value();
            break;
    }
}

void IndexManager::writeValue(Value value, byte *block, int &fp) {
    switch (type) {
        case Value::INT:
            writeInt(value.intValue, block, fp);
            break;
        case Value::FLOAT:
            writeFloat(value.floatValue, block, fp);
            break;
        case Value::STRING:
            writeString(value.stringValue, block, fp, attributeLen);
            break;
        default:
            break;
    }
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
