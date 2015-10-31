//
//  BplusTree.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/31.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#include "BplusTree.hpp"

#define SUCCESS 1
#define FAIL 0
#define nil 0

#pragma mark -
#pragma mark Insertion

int BplusTree::insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset) {
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockInfo *block = BufferManager::get_file_block(DBName, indexName, INDEXFILE, 0);
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

void BplusTree::insertIntoLeafWithoutSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset) {
    byte *leaf = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(leaf);
    short pos = findPisition(leaf, attributeValue);
    addNodeWithPointerFirst(leaf, pos, blockHeader->nodeNumber-pos+1, attributeValue, recordOffset);
    leafHeader->nodeNumber++;
}

void BplusTree::insertIntoLeafWithSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset) {
    blockNumber++;
    blockInfo *newBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNumber);
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);
    writeBlockHeader(blockNumber, 1, leafHeader->father, block->blockNum, leafHeader->nodeNumber/2);
    
    // move the half
    byte *newLeaf = (byte *)newBlock->cBlock;
    byte *leaf = (byte *)block->cBlock;
    copyNodesWithPointerFirst(newLeaf, 1, leaf, leafHeader->nodeNumber/2+1, maxNode/2);
    leafHeader->nodeNumber /= 2;
    
    // change right pointer
    int fp = sizeof(BLOCKHEADER)+(maxNode/2)*(BLOCKBYTES+attributeLen);
    DataTransfer::writeInt(blockNumber, leaf, fp);
    
    // insert
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    Value value = readValue(newLeaf, fp);
    if (attributeValue.isLessThan(value))
        insertIntoLeafWithoutSplit(leafHeader, block, attributeValue, recordOffset);
    else
        insertIntoLeafWithoutSplit(readBlockHeader(newLeaf), newBlock, attributeValue, recordOffset);
    
    insertIntoParent(newBlock, value);
}

void BplusTree::insertIntoInternalWithoutSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset) {
    byte *internal = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(internal);
    short pos = findPisition(internal, indexValue);
    addNodeWithValueFirst(internal, pos, blockHeader->nodeNumber-pos+1, indexValue, indexOffset);
    internalHeader->nodeNumber++;
}

void BplusTree::insertIntoInternalWithSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset) {
    blockNumber++;
    blockInfo *newBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNumber);
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);
    writeBlockHeader(blockNumber, 0, internalHeader->father, block->blockNum, internalHeader->nodeNumber/2);
    
    // move the half
    byte *newInternal = (byte *)newBlock->cBlock;
    byte *internal = (byte *)block->cBlock;
    copyNodesWithPointerFirst(newInternal, 1, internal, internalHeader->nodeNumber/2+1, maxNode/2);
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
            DataTransfer::writeInt(indexOffset, newInternal, fp);
        } else {
            copyNodesWithPointerFirst(newInternal, 2, newInternal, 1, pos-2);
            fp = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
            writeValue(indexValue, newInternal, fp);
            DataTransfer::writeInt(indexOffset, newInternal, fp);
        }
    }
    
    // update father ptr
    fp = sizeof(BLOCKHEADER);
    for (short i=0; i<=internalHeader->nodeNumber; i++) {
        int child = DataTransfer::readInt(internal, fp);
        blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, child);
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
        blockHeader->father = blockNumber;
        readValue(internal, fp);
    }
    
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    insertIntoParent(newBlock, readValue(newInternal, fp));
}

void BplusTree::insertIntoParent(blockInfo *block, Value indexValue) {
    int fatherNo = readBlockHeader((byte *)block->cBlock)->father;
    if (fatherNo == 0)
        insertIntoRoot(block, indexValue);
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, fatherNo);
    BLOCKHEADER *blockHeader = readBlockHeader((byte *)fatherBlock->cBlock);
    if (blockHeader->nodeNumber == maxNode) {
        insertIntoInternalWithSplit(blockHeader, fatherBlock, indexValue, block->blockNum);
    } else {
        insertIntoInternalWithoutSplit(blockHeader, fatherBlock, indexValue, block->blockNum);
    }
}

void BplusTree::insertIntoRoot(blockInfo *block, Value rootValue) {
    blockNumber++;
    blockInfo *newRoot = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNumber);
    rootBlock = blockNumber;
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);      //update index file header
    blockInfo *oldRoot = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, rootBlock);
    
    // write 3 block headers
    BLOCKHEADER *blockHeader;
    blockHeader = readBlockHeader((byte *)oldRoot->cBlock);
    writeBlockHeader(oldRoot->blockNum, 0, blockNumber, blockHeader->left, blockHeader->nodeNumber);
    blockHeader = readBlockHeader((byte *)block->cBlock);
    writeBlockHeader(block->blockNum, blockHeader->isLeaf, blockNumber, block->blockNum, blockHeader->nodeNumber);
    writeBlockHeader(blockNumber, 0, 0, 0, 1);
    
    // write root data
    int fp = sizeof(BLOCKHEADER);
    DataTransfer::writeInt(oldRoot->blockNum, (byte *)newRoot->cBlock, fp);
    writeValue(rootValue, (byte *)newRoot->cBlock, fp);
    DataTransfer::writeInt(block->blockNum, (byte *)newRoot->cBlock, fp);
}

#pragma mark -
#pragma mark Deletion

int BplusTree::deleteFrom(string DBName, string tableName, string indexName, Value attributeValue) {
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
    blockInfo *block;
    block = BufferManager::get_file_block(DBName, tableName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    deleteNode(leaf, attributeValue);
    return SUCCESS;
}

void BplusTree::deleteNode(blockInfo *block, Value value) {
    byte *data = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(data);
    short pos = findPisition(data, value);
    copyNodesWithValueFirst(data, pos, data, pos+1, blockHeader->nodeNumber-pos);
    blockHeader->nodeNumber--;
    
    if (blockHeader->father == 0)
        adjustRoot(blockHeader, block);
    if (blockHeader->nodeNumber>=maxNode/2)
        exit(1);
#warning return
    int neighbor;
    if (blockHeader->left == nil) {
        int father = blockHeader->father;
        blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, father);
        int fp = sizeof(BLOCKHEADER)+(BLOCKBYTES+attributeLen);
        neighbor = DataTransfer::readInt((byte *)fatherBlock->cBlock, fp);
#warning right neighbor
    } else
        neighbor = blockHeader->left;
    
    blockInfo *neighborBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, neighbor);
    BLOCKHEADER *neighborBlockHeader = readBlockHeader((byte *)neighborBlock->cBlock);
    if (blockHeader->nodeNumber+neighborBlockHeader->nodeNumber<maxNode)
        coalesce(neighborBlockHeader, neighborBlock, blockHeader, block);
    else
        redistribute(neighborBlockHeader, neighborBlock, blockHeader, block);
}

void BplusTree::coalesce(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock) {
    int fp = sizeof(BLOCKHEADER);
    byte *right = (byte *)rightBlock->cBlock;
    
    // read child value
    int child = DataTransfer::readInt(right, fp);
    blockInfo *childBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, child);
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    Value value = readValue((byte *)childBlock->cBlock, fp);
#warning wrong
    // write child value
    fp = sizeof(BLOCKHEADER)+leftBlockHeader->nodeNumber*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    byte *left = (byte *)leftBlock->cBlock;
    writeValue(value, left, fp);
    leftBlockHeader->nodeNumber++;
    
    copyNodesWithPointerFirst(left, leftBlockHeader->nodeNumber++, right, 1, rightBlockHeader->nodeNumber);
    
    // delete block
#warning memory, blockNumber
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, rightBlockHeader->father);
    deleteNode(fatherBlock, value);
}

void BplusTree::redistribute(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock) {
    int fp = sizeof(BLOCKHEADER);
    byte *right = (byte *)rightBlock->cBlock;
#warning wrong
    // read child value
    int child = DataTransfer::readInt(right, fp);
    blockInfo *childBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, child);
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    Value value = readValue((byte *)childBlock->cBlock, fp);
    
    // read ptr
    byte *left = (byte *)leftBlock->cBlock;
    int ptr = DataTransfer::readInt(left, fp);
    
    leftBlockHeader->nodeNumber--;
    addNodeWithPointerFirst(right, 1, rightBlockHeader->nodeNumber, value, ptr);
    rightBlockHeader->nodeNumber++;
}

void BplusTree::adjustRoot(BLOCKHEADER *blockHeader, blockInfo *block) {
    int fp = sizeof(BLOCKHEADER);
    int root = DataTransfer::readInt((byte *)block->cBlock, fp);
    if (blockHeader->nodeNumber == 0) {
#warning memory, blockNumber
        rootBlock = root;
        blockInfo *newRootBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, root);
        BLOCKHEADER *newRootBlockHeader = readBlockHeader((byte *)newRootBlock->cBlock);
        byte isLeaf = (blockNumber == 1) ? 1:0;
        writeBlockHeader(root, isLeaf, 0, 0, newRootBlockHeader->nodeNumber);
        writeIndexFileHeader(blockNumber, root, attributeLen, maxNode, type);
    }
}

#pragma mark -
#pragma mark Selection

int BplusTree::select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results) {
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
    int offset = DataTransfer::readInt(data, fpPtr);
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

#pragma mark -
#pragma mark Utility

blockInfo *BplusTree::searchInTree(int rootNo, Value attributeValue) {
    blockInfo *block;
    int blockNo = rootNo;
    while (true) {
        block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNo);
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
        if (blockHeader->isLeaf)
            break;
        int fp = sizeof(BLOCKHEADER);
        short nodeNo = 1;
        while (nodeNo<=blockHeader->nodeNumber) {
            int leftSon = DataTransfer::readInt((byte *)block->cBlock, fp);
            Value value = readValue((byte *)block->cBlock, fp);
            if (attributeValue.isLessThan(value)) {
                blockNo = leftSon;
                break;
            }
            nodeNo++;
        }
        if (nodeNo>blockHeader->nodeNumber)                      //last son
            blockNo = DataTransfer::readInt((byte *)block->cBlock, fp);
    }
    return block;
}

short BplusTree::findPisition(byte *block, Value benchmark) {
    BLOCKHEADER *blockHeader =readBlockHeader(block);
    int fp = sizeof(BLOCKHEADER);
    short i;
    for (i=1; i<=blockHeader->nodeNumber; i++) {
        DataTransfer::readInt(block, fp);
        Value value = readValue(block, fp);
        if (value.isGreatEqualTo(benchmark))
            break;
    }
    return i;
}

void BplusTree::copyNodesWithPointerFirst(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n) {
    int destfp = sizeof(BLOCKHEADER)+(destNo-1)*(BLOCKBYTES+attributeLen);
    int srcfp = sizeof(BLOCKHEADER)+(srcNo-1)*(BLOCKBYTES+attributeLen);
    int ptr;
    for (int i=0; i<n; i++) {
        ptr = DataTransfer::readInt(srcBlock, srcfp);
        DataTransfer::writeInt(ptr, destBlock, destfp);
        Value value = readValue(srcBlock, srcfp);
        writeValue(value, destBlock, destfp);
    }
    
    // move the tail ptr
    ptr = DataTransfer::readInt(srcBlock, srcfp);
    DataTransfer::writeInt(ptr, destBlock, destfp);
}

void BplusTree::copyNodesWithValueFirst(byte *destBlock, int destNo, byte *srcBlock, int srcNo, int n) {
    int destfp = sizeof(BLOCKHEADER)+(destNo-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    int srcfp = sizeof(BLOCKHEADER)+(srcNo-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    int ptr;
    for (int i=0; i<n; i++) {
        Value value = readValue(srcBlock, srcfp);
        writeValue(value, destBlock, destfp);
        ptr = DataTransfer::readInt(srcBlock, srcfp);
        DataTransfer::writeInt(ptr, destBlock, destfp);
    }
}

void BplusTree::addNodeWithPointerFirst(byte *block, int pos, short n, Value value, int ptr) {
    int ptrToAdd = ptr;
    Value valueToAdd = value;
    int fpRead = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen);
    for (short i=0; i<n; i++) {
        int fpWrite = fpRead;
        int tempPtr = DataTransfer::readInt(block, fpRead);
        Value tempValue = readValue(block, fpRead);
        DataTransfer::writeInt(ptrToAdd, block, fpWrite);
        writeValue(valueToAdd, block, fpWrite);
        ptrToAdd = tempPtr;
        valueToAdd = tempValue;
    }
    int fpWrite = fpRead;
    int tempPtr = DataTransfer::readInt(block, fpRead);
    DataTransfer::writeInt(ptrToAdd, block, fpWrite);
    writeValue(valueToAdd, block, fpWrite);
    DataTransfer::writeInt(tempPtr, block, fpWrite);              // move the tail ptr
}

void BplusTree::addNodeWithValueFirst(byte *block, int pos, short n, Value value, int ptr) {
    Value valueToAdd = value;
    int ptrToAdd = ptr;
    int fpRead = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    for (short i=0; i<n; i++) {
        int fpWrite = fpRead;
        Value tempValue = readValue(block, fpRead);
        int tempPtr = DataTransfer::readInt(block, fpRead);
        writeValue(valueToAdd, block, fpWrite);
        DataTransfer::writeInt(ptrToAdd, block, fpWrite);
        valueToAdd = tempValue;
        ptrToAdd = tempPtr;
    }
    int fpWrite = fpRead;
    writeValue(valueToAdd, block, fpWrite);
    DataTransfer::writeInt(ptrToAdd, block, fpWrite);
}

#pragma mark -
#pragma mark Read/Write Data

void BplusTree::readIndexFileHeader(byte *block) {
    int fp = 0;
    blockNumber = DataTransfer::readInt(block, fp);
    rootBlock = DataTransfer::readInt(block, fp);
    attributeLen = DataTransfer::readInt(block, fp);
    maxNode = DataTransfer::readShort(block, fp);
}

void BplusTree::writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode, byte attribyteType) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, 0);
    int fp = 0;
    DataTransfer::writeInt(blockNumber, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(rootNumber, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(attributeLen, (byte *)block->cBlock, fp);
    DataTransfer::writeShort(maxNode, (byte *)block->cBlock, fp);
}

void BplusTree::writeBlockHeader(int blockNo, byte isLeaf, int father, int left, short nodeNumber) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNo);
    int fp = 0;
    DataTransfer::writeByte(isLeaf, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(father, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(left, (byte *)block->cBlock, fp);
    DataTransfer::writeShort(nodeNumber, (byte *)block->cBlock, fp);
}

Value BplusTree::readValue(byte *block, int &fp) {
    switch (type) {
        case Value::INT:
            return Value(DataTransfer::readInt(block, fp));
            break;
        case Value::FLOAT:
            return Value(DataTransfer::readFloat(block, fp));
            break;
        case Value::STRING:
            return Value(DataTransfer::readString(block, fp, attributeLen));
            break;
        default:
            return Value();
            break;
    }
}

void BplusTree::writeValue(Value value, byte *block, int &fp) {
    switch (type) {
        case Value::INT:
            DataTransfer::writeInt(value.intValue, block, fp);
            break;
        case Value::FLOAT:
            DataTransfer::writeFloat(value.floatValue, block, fp);
            break;
        case Value::STRING:
            DataTransfer::writeString(value.stringValue, block, fp, attributeLen);
            break;
        default:
            break;
    }
}
