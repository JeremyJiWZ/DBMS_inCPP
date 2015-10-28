//
//  BufferManager.cpp
//  DBMSSev
//
//  Created by mr.ji on 15/10/22.
//  Copyright (c) 2015年 mr.ji. All rights reserved.
//

#include "BufferManager.h"
void BufferManager::addTime(string DB_Name) //遍历文件块，将其itime++
{
    blockInfo *block;
    fileInfo *file;
    file=FileHandle;
    while(file!=NULL)//遍历文件头
    {
        block=FileHandle->firstBlock;
        while (block!=NULL) {//遍历文件块
            block->iTime++;
            block=block->next;
        }
        file=file->Next;
    }
}

blockInfo *BufferManager::findBlock(string DB_Name)
{
    blockInfo *block;
    block=BlockHandle;
    if (block!=NULL) {
        BlockHandle=BlockHandle->next;
        return block;
    }
    else if(blockCount<BLOCK_MAX){
        block=new blockInfo;
        block->cBlock=new Block;
        blockCount++;
        return block;
    }
    else    //使用LRU算法找到替换块,找到最大的itime，将该块替换出去
    {
        fileInfo* file = FileHandle;
        blockInfo* bp;
        block=file->firstBlock;
        while (file!=NULL) {
            bp=file->firstBlock;
            while (bp!=NULL) {
                if (bp->iTime>=block->iTime&&bp->lock!=0)//该块的使用时间大且未被锁上
                    block=bp;
                bp=bp->next;
            }
            file=file->Next;
        }
    }
    return block;
}
void replce(fileInfo *m_fileInfo,blockInfo *m_blockInfo)
{
    
}
fileInfo *get_file_info(string DB_Name,string File_Name,int m_fileType)
{
    
}
void closeFile(string DB_Name,string File_Name,int m_fileType)
{
    
}
void writeBlock(string DB_Name,blockInfo* block)
{
    
}
blockInfo *readBlock(FILE *fp, int blockNum)
{
    
}