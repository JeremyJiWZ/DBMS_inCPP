//
//  BplusTree.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/31.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#include <iostream>
#include <queue>
#include "BplusTree.hpp"

#define SUCCESS 1
#define FAIL 0
#define nil 0

#pragma mark -
#pragma mark Insertion

int BplusTree::insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset) {
    init(DBName, tableName, indexName);
    blockInfo *block = BufferManager::get_file_block(DBName, indexName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    BLOCKHEADER *blockHeader = readBlockHeader((byte *)leaf->cBlock);
    if (blockHeader->nodeNumber == maxNode) {
        insertIntoLeafWithSplit(blockHeader, leaf, attributeValue, recordOffset);
    } else {
        insertIntoLeafWithoutSplit(blockHeader, leaf, attributeValue, recordOffset);
    }
//    printTree();
    free(blockHeader);
    return SUCCESS;
}

void BplusTree::insertIntoLeafWithoutSplit(BLOCKHEADER *leafHeader, blockInfo *block, Value attributeValue, int recordOffset) {
    byte *leaf = (byte *)block->cBlock;
    short pos = findPosition(leaf, attributeValue);
    addNodeWithPointerFirst(leaf, pos, leafHeader->nodeNumber-pos+1, attributeValue, recordOffset);
    leafHeader->nodeNumber++;
    writeBlockHeader(block->blockNum, leafHeader->isLeaf, leafHeader->father, leafHeader->left, leafHeader->nodeNumber);
    block->dirtyBlock = true;
//    cout<<"insertIntoLeafWithoutSplit:: ";
//    printBlock(block->blockNum);
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
    writeBlockHeader(block->blockNum, leafHeader->isLeaf, leafHeader->father, leafHeader->left, leafHeader->nodeNumber);
    
    // change right pointer
    int fp = sizeof(BLOCKHEADER)+(maxNode/2)*(BLOCKBYTES+attributeLen);
    DataTransfer::writeInt(blockNumber, leaf, fp);
    
    // change left pointer
    fp = sizeof(BLOCKHEADER)+(maxNode/2)*(BLOCKBYTES+attributeLen);
    int neighbor = DataTransfer::readInt(newLeaf, fp);
    if (neighbor != nil) {
        blockInfo *neighborBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, neighbor);
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)neighborBlock->cBlock);
        writeBlockHeader(neighbor, blockHeader->isLeaf, blockHeader->father, blockNumber, blockHeader->nodeNumber);
        free(blockHeader);
    }
    
    // insert
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    Value value = readValue(newLeaf, fp);
    if (attributeValue.isLessThan(value))
        insertIntoLeafWithoutSplit(leafHeader, block, attributeValue, recordOffset);
    else
        insertIntoLeafWithoutSplit(readBlockHeader(newLeaf), newBlock, attributeValue, recordOffset);
    block->dirtyBlock = true;
    newBlock->dirtyBlock = true;
    
    insertIntoParent(newBlock, value);
}

void BplusTree::insertIntoInternalWithoutSplit(BLOCKHEADER *internalHeader, blockInfo *block, Value indexValue, int indexOffset) {
    byte *internal = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(internal);
    short pos = findPosition(internal, indexValue);
    addNodeWithValueFirst(internal, pos, blockHeader->nodeNumber-pos+1, indexValue, indexOffset);
    internalHeader->nodeNumber++;
    writeBlockHeader(block->blockNum, internalHeader->isLeaf, internalHeader->father, internalHeader->left, internalHeader->nodeNumber);
    block->dirtyBlock = true;
    free(blockHeader);
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
    writeBlockHeader(block->blockNum, internalHeader->isLeaf, internalHeader->father, internalHeader->left, internalHeader->nodeNumber);
    
    // insert
    int fp = sizeof(BLOCKHEADER)+(maxNode/2-1)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
    Value value = readValue(internal, fp);
    if (indexValue.isLessThan(value)) {
        short pos = findPosition(internal, indexValue);
        addNodeWithValueFirst(internal, pos, internalHeader->nodeNumber-pos, indexValue, indexOffset);
    } else {
        short pos = findPosition(newInternal, indexValue);
        if (pos == 1) {
            fp = sizeof(BLOCKHEADER);
            DataTransfer::writeInt(indexOffset, newInternal, fp);
        } else {
            copyNodesWithPointerFirst(newInternal, 1, newInternal, 2, pos-2);
            fp = sizeof(BLOCKHEADER)+(pos-2)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
            writeValue(indexValue, newInternal, fp);
            DataTransfer::writeInt(indexOffset, newInternal, fp);
        }
    }
    block->dirtyBlock = true;
    newBlock->dirtyBlock = true;
    
    // update father ptr
    updateFatherPointer(internalHeader->nodeNumber+1, newInternal, blockNumber);
    
    int leafNo = getLeftMostLeaf(blockNumber);
    blockInfo *leafBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, leafNo);
    byte *leaf = (byte *)leafBlock->cBlock;
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    insertIntoParent(newBlock, readValue(leaf, fp));
}

void BplusTree::insertIntoParent(blockInfo *block, Value indexValue) {
    int fatherNo = readBlockHeader((byte *)block->cBlock)->father;
    if (fatherNo == 0) {
        insertIntoRoot(block, indexValue);
        return;
    }
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, fatherNo);
    BLOCKHEADER *blockHeader = readBlockHeader((byte *)fatherBlock->cBlock);
    if (blockHeader->nodeNumber == maxNode) {
        insertIntoInternalWithSplit(blockHeader, fatherBlock, indexValue, block->blockNum);
    } else {
        insertIntoInternalWithoutSplit(blockHeader, fatherBlock, indexValue, block->blockNum);
    }
    free(blockHeader);
}

void BplusTree::insertIntoRoot(blockInfo *block, Value rootValue) {
    blockNumber++;
    blockInfo *newRoot = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNumber);
    blockInfo *oldRoot = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, rootBlock);
    rootBlock = blockNumber;
    writeIndexFileHeader(blockNumber, rootBlock, attributeLen, maxNode, type);      //update index file header
    
    // write 3 block headers
    BLOCKHEADER *blockHeader;
    blockHeader = readBlockHeader((byte *)oldRoot->cBlock);
    writeBlockHeader(oldRoot->blockNum, blockHeader->isLeaf, blockNumber, blockHeader->left, blockHeader->nodeNumber);
    blockHeader = readBlockHeader((byte *)block->cBlock);
    writeBlockHeader(block->blockNum, blockHeader->isLeaf, blockNumber, block->blockNum, blockHeader->nodeNumber);
    writeBlockHeader(blockNumber, 0, 0, 0, 1);
    
    // write root data
    int fp = sizeof(BLOCKHEADER);
    DataTransfer::writeInt(oldRoot->blockNum, (byte *)newRoot->cBlock, fp);
    writeValue(rootValue, (byte *)newRoot->cBlock, fp);
    DataTransfer::writeInt(block->blockNum, (byte *)newRoot->cBlock, fp);
    
    free(blockHeader);
}

#pragma mark -
#pragma mark Deletion

int BplusTree::deleteFrom(string DBName, string tableName, string indexName, Value attributeValue) {
    init(DBName, tableName, indexName);
    blockInfo *block;
    block = BufferManager::get_file_block(DBName, indexName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    deleteNode(leaf, attributeValue);
    return SUCCESS;
}

void BplusTree::deleteNode(blockInfo *block, Value value) {
    byte *data = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(data);
    short pos = findPosition(data, value);
    copyNodesWithPointerFirst(data, pos, data, pos+1, blockHeader->nodeNumber-pos);
    blockHeader->nodeNumber--;
    writeBlockHeader(block->blockNum, blockHeader->isLeaf, blockHeader->father, blockHeader->left, blockHeader->nodeNumber);
    block->dirtyBlock = true;
#warning delete the most left
    
    handleTooFewNodes(blockHeader, block);
    free(blockHeader);
}

void BplusTree::deleteNode(blockInfo *block, int ptr) {
    byte *data = (byte *)block->cBlock;
    BLOCKHEADER *blockHeader = readBlockHeader(data);
    short pos = findPosition(data, ptr)-1;
    copyNodesWithValueFirst(data, pos, data, pos+1, blockHeader->nodeNumber-pos);
    blockHeader->nodeNumber--;
    writeBlockHeader(block->blockNum, blockHeader->isLeaf, blockHeader->father, blockHeader->left, blockHeader->nodeNumber);
    block->dirtyBlock = true;
#warning delete the most left
    
    handleTooFewNodes(blockHeader, block);
    free(blockHeader);
}

void BplusTree::handleTooFewNodes(BLOCKHEADER *blockHeader, blockInfo *block) {
    if (blockHeader->father == 0) {
        adjustRoot(blockHeader, block);
        return;
    }
    if (blockHeader->nodeNumber>=maxNode/2)
        return;
    int father = blockHeader->father;
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, father);
    int neighbor = getRightNeighbor((byte *)fatherBlock->cBlock, block->blockNum);
    if (neighbor == nil) {
        int pos = findPosition((byte *)fatherBlock->cBlock, block->blockNum);
        int fp = sizeof(BLOCKHEADER)+(pos-2)*(BLOCKBYTES+attributeLen);
        int leftNeighbor = DataTransfer::readInt((byte *)fatherBlock->cBlock, fp);
        blockInfo *neighborBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, leftNeighbor);
        BLOCKHEADER *neighborBlockHeader = readBlockHeader((byte *)neighborBlock->cBlock);
        if (blockHeader->nodeNumber+neighborBlockHeader->nodeNumber<maxNode)
            coalesce(neighborBlockHeader, neighborBlock, blockHeader, block);
        else
            redistributeFromLeftToRight(neighborBlockHeader, neighborBlock, blockHeader, block);
        free(neighborBlockHeader);
    } else {
        blockInfo *neighborBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, neighbor);
        BLOCKHEADER *neighborBlockHeader = readBlockHeader((byte *)neighborBlock->cBlock);
        if (blockHeader->nodeNumber+neighborBlockHeader->nodeNumber<maxNode)
            coalesce(blockHeader, block, neighborBlockHeader, neighborBlock);
        else
            redistributeFromRightToLeft(blockHeader, block, neighborBlockHeader, neighborBlock);
        free(neighborBlockHeader);
    }
}

void BplusTree::coalesce(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock) {
    int fp = sizeof(BLOCKHEADER);
    byte *right = (byte *)rightBlock->cBlock;
    byte *left = (byte *)leftBlock->cBlock;
    
    if (!leftBlockHeader->isLeaf) {
        // read child value
        int child = DataTransfer::readInt(right, fp);
        blockInfo *childBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, child);
        fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
        Value value = readValue((byte *)childBlock->cBlock, fp);
        
        // write child value
        fp = sizeof(BLOCKHEADER)+leftBlockHeader->nodeNumber*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
        writeValue(value, left, fp);
        leftBlockHeader->nodeNumber++;
    }
    
    copyNodesWithPointerFirst(left, leftBlockHeader->nodeNumber+1, right, 1, rightBlockHeader->nodeNumber);
    leftBlockHeader->nodeNumber += rightBlockHeader->nodeNumber;
    writeBlockHeader(leftBlock->blockNum, leftBlockHeader->isLeaf, leftBlockHeader->father, leftBlockHeader->left, leftBlockHeader->nodeNumber);
    
    if (!rightBlockHeader->isLeaf)
        updateFatherPointer(rightBlockHeader->nodeNumber+1, right, leftBlock->blockNum);
    
    // delete block
#warning memory, blockNumber
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, rightBlockHeader->father);
    deleteNode(fatherBlock, rightBlock->blockNum);
    leftBlock->dirtyBlock = true;
    rightBlock->dirtyBlock = true;
}

void BplusTree::redistributeFromLeftToRight(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock) {
    int fp = sizeof(BLOCKHEADER);
    byte *right = (byte *)rightBlock->cBlock;
    byte *left = (byte *)leftBlock->cBlock;

    Value value;
    int ptr;
    if (leftBlockHeader->isLeaf) {
        fp = sizeof(BLOCKHEADER)+(leftBlockHeader->nodeNumber-1)*(BLOCKBYTES+attributeLen);
        ptr = DataTransfer::readInt(left, fp);
        value = readValue(left, fp);
        
        // move tail pointer
        int tail = DataTransfer::readInt(left, fp);
        fp = sizeof(BLOCKHEADER)+(leftBlockHeader->nodeNumber-1)*(BLOCKBYTES+attributeLen);
        DataTransfer::writeInt(tail, left, fp);
    } else {
        // read child value
        int child = DataTransfer::readInt(right, fp);
        blockInfo *childBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, child);
        fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
        value = readValue((byte *)childBlock->cBlock, fp);
        
        // read ptr
        fp = sizeof(BLOCKHEADER)+leftBlockHeader->nodeNumber*(BLOCKBYTES+attributeLen);
        ptr = DataTransfer::readInt(left, fp);
    }
    
    leftBlockHeader->nodeNumber--;
    writeBlockHeader(leftBlock->blockNum, leftBlockHeader->isLeaf, leftBlockHeader->father, leftBlockHeader->left, leftBlockHeader->nodeNumber);
    
    addNodeWithPointerFirst(right, 1, rightBlockHeader->nodeNumber, value, ptr);
    rightBlockHeader->nodeNumber++;
    writeBlockHeader(rightBlock->blockNum, rightBlockHeader->isLeaf, rightBlockHeader->father, rightBlockHeader->left, rightBlockHeader->nodeNumber);
    leftBlock->dirtyBlock = true;
    rightBlock->dirtyBlock = true;
    
    // update father pointer
    if (!rightBlockHeader->isLeaf) {
        blockInfo *childBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, ptr);
        byte *child = (byte *)childBlock->cBlock;
        BLOCKHEADER *childBlockHeader = readBlockHeader(child);
        childBlockHeader->father = rightBlock->blockNum;
        writeBlockHeader(ptr, childBlockHeader->isLeaf, childBlockHeader->father, childBlockHeader->left, childBlockHeader->nodeNumber);
        childBlock->dirtyBlock = true;
        free(childBlockHeader);
    }
    
    int leafNo = getLeftMostLeaf(rightBlock->blockNum);
    blockInfo *leafBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, leafNo);
    byte *leaf = (byte *)leafBlock->cBlock;
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    value = readValue(leaf, fp);
    
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, rightBlockHeader->father);
    updateFatherBlock(rightBlock->blockNum, fatherBlock, value);
    fatherBlock->dirtyBlock = true;
}

void BplusTree::redistributeFromRightToLeft(BLOCKHEADER *leftBlockHeader, blockInfo *leftBlock, BLOCKHEADER *rightBlockHeader, blockInfo *rightBlock) {
    int fp = sizeof(BLOCKHEADER);
    byte *right = (byte *)rightBlock->cBlock;
    byte *left = (byte *)leftBlock->cBlock;
    
    if (rightBlockHeader->isLeaf) {
        int ptr = DataTransfer::readInt(right, fp);
        Value value = readValue(right, fp);
        
        // move tail pointer
        fp = sizeof(BLOCKHEADER)+leftBlockHeader->nodeNumber*(BLOCKBYTES+attributeLen);
        int tail = DataTransfer::readInt(left, fp);
        fp = sizeof(BLOCKHEADER)+leftBlockHeader->nodeNumber*(BLOCKBYTES+attributeLen);
        DataTransfer::writeInt(ptr, left, fp);
        writeValue(value, left, fp);
        DataTransfer::writeInt(tail, left, fp);
        leftBlockHeader->nodeNumber++;
        writeBlockHeader(leftBlock->blockNum, leftBlockHeader->isLeaf, leftBlockHeader->father, leftBlockHeader->left, leftBlockHeader->nodeNumber);
    } else {
        // read child value
        int ptr = DataTransfer::readInt(right, fp);
        blockInfo *childBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, ptr);
        byte *child = (byte *)childBlock->cBlock;
        fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
        Value value = readValue(child, fp);
        
        // write node
        fp = sizeof(BLOCKHEADER)+leftBlockHeader->nodeNumber*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
        writeValue(value, left, fp);
        DataTransfer::writeInt(ptr, left, fp);
        leftBlockHeader->nodeNumber++;
        writeBlockHeader(leftBlock->blockNum, leftBlockHeader->isLeaf, leftBlockHeader->father, leftBlockHeader->left, leftBlockHeader->nodeNumber);
        
        // update father pointer
        BLOCKHEADER *childBlockHeader = readBlockHeader(child);
        childBlockHeader->father = leftBlock->blockNum;
        writeBlockHeader(ptr, childBlockHeader->isLeaf, childBlockHeader->father, childBlockHeader->left, childBlockHeader->nodeNumber);
        childBlock->dirtyBlock = true;
        free(childBlockHeader);
    }
    
    rightBlockHeader->nodeNumber--;
    writeBlockHeader(rightBlock->blockNum, rightBlockHeader->isLeaf, rightBlockHeader->father, rightBlockHeader->left, rightBlockHeader->nodeNumber);
    copyNodesWithPointerFirst(right, 1, right, 2, rightBlockHeader->nodeNumber);
    leftBlock->dirtyBlock = true;
    rightBlock->dirtyBlock = true;
    
    int leafNo = getLeftMostLeaf(rightBlock->blockNum);
    blockInfo *leafBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, leafNo);
    byte *leaf = (byte *)leafBlock->cBlock;
    fp = sizeof(BLOCKHEADER)+BLOCKBYTES;
    Value value = readValue(leaf, fp);
    
    blockInfo *fatherBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, rightBlockHeader->father);
    updateFatherBlock(rightBlock->blockNum, fatherBlock, value);
    fatherBlock->dirtyBlock = true;
}

void BplusTree::adjustRoot(BLOCKHEADER *blockHeader, blockInfo *block) {
    int fp = sizeof(BLOCKHEADER);
    int root = DataTransfer::readInt((byte *)block->cBlock, fp);
    if (blockHeader->nodeNumber == 0 && !blockHeader->isLeaf) {
#warning memory, blockNumber
        rootBlock = root;
        blockInfo *newRootBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, root);
        BLOCKHEADER *newRootBlockHeader = readBlockHeader((byte *)newRootBlock->cBlock);
        writeBlockHeader(root, newRootBlockHeader->isLeaf, 0, 0, newRootBlockHeader->nodeNumber);
        writeIndexFileHeader(blockNumber, root, attributeLen, maxNode, type);
        free(newRootBlockHeader);
    }
}

int BplusTree::getRightNeighbor(byte *block, int ptr) {
    BLOCKHEADER *blockHeader = readBlockHeader(block);
    short pos = findPosition(block, ptr);
    if (pos == blockHeader->nodeNumber+1)
        return nil;
    else {
        int fp = sizeof(BLOCKHEADER)+pos*(BLOCKBYTES+attributeLen);
        return DataTransfer::readInt(block, fp);
    }
    free(blockHeader);
}

#pragma mark -
#pragma mark Selection

int BplusTree::select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<int> &results) {
    init(DBName, tableName, indexName);
    blockInfo *block = BufferManager::get_file_block(DBName, indexName, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    blockInfo *leaf = searchInTree(rootBlock, attributeValue);
    cout<<leaf->blockNum<<": ";
    byte *data = (byte *)leaf->cBlock;
    short pos = findPosition(data, attributeValue);
    int fpPtr = sizeof(BLOCKHEADER)+(pos-1)*(BLOCKBYTES+attributeLen);
    int fpValue = fpPtr+BLOCKBYTES;
    int offset = DataTransfer::readInt(data, fpPtr);
    Value benchmark = readValue(data, fpValue);
    int start = 1, end = readBlockHeader(data)->nodeNumber, direction = 0;
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
            direction = -1;
            break;
        case LESSEQUAL:
            if (attributeValue.isEqualTo(benchmark))
                results.push_back(offset);
            end = pos-1;
            direction = -1;
            break;
        case GREAT:
            if (attributeValue.isNotEqualTo(benchmark))
                results.push_back(offset);
            start = pos+1;
            direction = 1;
            break;
        case GREATEQUAL:
            start = pos;
            direction = 1;
            break;
        default:
            break;
    }
    int fp = sizeof(BLOCKHEADER)+(start-1)*(BLOCKBYTES+attributeLen);
    for (int i=start; i<=end; i++) {
        int tmp = DataTransfer::readInt(data, fp);
//        cout<<tmp<<" ";
        results.push_back(tmp);
        readValue(data, fp);
    }
    int leafNo;
    if (direction == 1)
        leafNo = DataTransfer::readInt(data, fp);
    else
        leafNo = getLeftMostLeaf(rootBlock);
    while (leafNo != nil && leafNo != leaf->blockNum) {
        blockInfo *leafBlock = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, leafNo);
        byte *leafData = (byte *)leafBlock->cBlock;
        BLOCKHEADER *leafHeader = readBlockHeader(leafData);
        int fp = sizeof(BLOCKHEADER);
        for (int i=1; i<=leafHeader->nodeNumber; i++) {
            int tmp = DataTransfer::readInt(leafData, fp);
//            cout<<tmp<<" ";
            results.push_back(tmp);
            readValue(leafData, fp);
        }
        leafNo = DataTransfer::readInt(leafData, fp);
        free(leafHeader);
    }
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
        free(blockHeader);
    }
    return block;
}

short BplusTree::findPosition(byte *block, Value benchmark) {
    BLOCKHEADER *blockHeader = readBlockHeader(block);
    int fp = sizeof(BLOCKHEADER);
    short i;
    for (i=1; i<=blockHeader->nodeNumber; i++) {
        DataTransfer::readInt(block, fp);
        Value value = readValue(block, fp);
        if (value.isGreatEqualTo(benchmark))
            break;
    }
    free(blockHeader);
    return i;
}

short BplusTree::findPosition(byte *block, int ptr) {
    BLOCKHEADER *blockHeader = readBlockHeader(block);
    int fp = sizeof(BLOCKHEADER);
    short i;
    for (i=1; i<=blockHeader->nodeNumber; i++) {
        int pointer = DataTransfer::readInt(block, fp);
        if (pointer == ptr)
            break;
        readValue(block, fp);
    }
    free(blockHeader);
    return i;
}

int BplusTree::getLeftMostLeaf(int root) {
    int left = root;
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, left);
    byte *data = (byte *)block->cBlock;
    while (!readBlockHeader(data)->isLeaf) {
        int fp = sizeof(BLOCKHEADER);
        left = DataTransfer::readInt(data, fp);
        block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, left);
        data = (byte *)block->cBlock;
    }
    return left;
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

// update blockHeader->father
void BplusTree::updateFatherPointer(short n, byte *data, int fatherNo) {
    int fp = sizeof(BLOCKHEADER);
    for (short i=0; i<n; i++) {
        int child = DataTransfer::readInt(data, fp);
        blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, child);
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
        blockHeader->father = fatherNo;
        writeBlockHeader(block->blockNum, blockHeader->isLeaf, blockHeader->father, blockHeader->left, blockHeader->nodeNumber);
        block->dirtyBlock = true;
        readValue(data, fp);
        free(blockHeader);
    }
}

// first node of child changes, update value in father block
void BplusTree::updateFatherBlock(int childNo, blockInfo *fatherBlock, Value value) {
    byte *father = (byte *)fatherBlock->cBlock;
    int pos = findPosition(father, childNo);
    if (pos>1) {
        int fp = sizeof(BLOCKHEADER)+(pos-2)*(BLOCKBYTES+attributeLen)+BLOCKBYTES;
        writeValue(value, father, fp);
    }
}

#pragma mark -
#pragma mark Read/Write Data

void BplusTree::init(string DBName, string tableName, string indexName) {
    currentDB = DBName;
    currentTable = tableName;
    currentIndex = indexName;
}

void BplusTree::readIndexFileHeader(byte *block) {
    int fp = 0;
    blockNumber = DataTransfer::readInt(block, fp);
    rootBlock = DataTransfer::readInt(block, fp);
    attributeLen = DataTransfer::readInt(block, fp);
    maxNode = DataTransfer::readShort(block, fp);
    type = (enum Value::AttributeType)DataTransfer::readInt(block, fp);
}

void BplusTree::writeIndexFileHeader(int blockNumber, int rootNumber, int attributeLen, short maxNode, int attributeType) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, 0);
    int fp = 0;
    DataTransfer::writeInt(blockNumber, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(rootNumber, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(attributeLen, (byte *)block->cBlock, fp);
    DataTransfer::writeShort(maxNode, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(attributeType, (byte *)block->cBlock, fp);
    block->dirtyBlock = true;
}

BLOCKHEADER *BplusTree::readBlockHeader(byte *block) {
    int fp = 0;
    BLOCKHEADER *blockHeader = (BLOCKHEADER *)malloc(sizeof(BLOCKHEADER));
    blockHeader->isLeaf = DataTransfer::readByte(block, fp);
    blockHeader->father = DataTransfer::readInt(block, fp);
    blockHeader->left = DataTransfer::readInt(block, fp);
    blockHeader->nodeNumber = DataTransfer::readShort(block, fp);
    return blockHeader;
}

void BplusTree::writeBlockHeader(int blockNo, byte isLeaf, int father, int left, short nodeNumber) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNo);
    int fp = 0;
    DataTransfer::writeByte(isLeaf, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(father, (byte *)block->cBlock, fp);
    DataTransfer::writeInt(left, (byte *)block->cBlock, fp);
    DataTransfer::writeShort(nodeNumber, (byte *)block->cBlock, fp);
    block->dirtyBlock = true;
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

void BplusTree::printValue(byte *block, int &fp) {
    switch (type) {
        case Value::INT:
            cout<<DataTransfer::readInt(block, fp);
            break;
        case Value::FLOAT:
            cout<<DataTransfer::readFloat(block, fp);
            break;
        case Value::STRING:
            cout<<DataTransfer::readString(block, fp, attributeLen);
            break;
    }
}

void BplusTree::printValue(byte *block, int &fp, ofstream &fout) {
    switch (type) {
        case Value::INT:
            fout<<DataTransfer::readInt(block, fp)%10000;
            break;
        case Value::FLOAT:
            fout<<DataTransfer::readFloat(block, fp);
            break;
        case Value::STRING:
            fout<<DataTransfer::readString(block, fp, attributeLen);
            break;
    }
}

void BplusTree::printTree(int count) {
    char fileName[255];
    sprintf(fileName, "/Users/Sylvanus/Sylvanus's Library/Studies/CS/Database/Design/DBMS/DBMS/log%d.txt", count);
    ofstream fout(fileName);
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, 0);
    readIndexFileHeader((byte *)block->cBlock);
    fout<<"total block:"<<blockNumber<<endl;
    queue<int> q;
    q.push(rootBlock);
    int re = 1;
    int s = 0;
    while (!q.empty()) {
        block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, q.front());
        fout<<block->blockNum<<":";
        BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
        fout<<"("<<(short)(blockHeader->isLeaf)<<", "<<blockHeader->father<<", "<<blockHeader->left<<", "<<blockHeader->nodeNumber<<")[";
        int fp = sizeof(BLOCKHEADER);
        for (int i=0; i<blockHeader->nodeNumber; i++) {
            int ptr = DataTransfer::readInt((byte *)block->cBlock, fp);
            fout<<ptr<<"{";
            s++;
            if (!blockHeader->isLeaf)
                q.push(ptr);
            printValue((byte *)block->cBlock, fp, fout);
            fout<<"}";
        }
        int ptr = DataTransfer::readInt((byte *)block->cBlock, fp);
        fout<<ptr<<"]  ";
        s++;
        if (!blockHeader->isLeaf)
            q.push(ptr);
        re--;
        if (re == 0) {
            fout<<endl;
            re = s;
            s = 0;
        }
        free(blockHeader);
        q.pop();
    }
    fout<<endl;
    fout.close();
}

void BplusTree::printBlock(int blockNo) {
    blockInfo *block = BufferManager::get_file_block(currentDB, currentIndex, INDEXFILE, blockNo);
    cout<<block->blockNum<<":";
    BLOCKHEADER *blockHeader = readBlockHeader((byte *)block->cBlock);
    cout<<"("<<(short)(blockHeader->isLeaf)<<", "<<blockHeader->father<<", "<<blockHeader->left<<", "<<blockHeader->nodeNumber<<")[";
    int fp = sizeof(BLOCKHEADER);
    for (int i=0; i<blockHeader->nodeNumber; i++) {
        int ptr = DataTransfer::readInt((byte *)block->cBlock, fp);
        cout<<ptr<<"{";
        printValue((byte *)block->cBlock, fp);
        cout<<"}";
    }
    int ptr = DataTransfer::readInt((byte *)block->cBlock, fp);
    cout<<ptr<<"]"<<endl<<endl;
    free(blockHeader);
}
