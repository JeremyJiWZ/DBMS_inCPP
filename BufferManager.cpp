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

blockInfo* BufferManager::findBlock(string DB_Name)//找到一个空块并将他返回;空块来自：垃圾链表、内存申请、替换块
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
void BufferManager::replce(fileInfo *m_fileInfo,blockInfo *m_blockInfo)//找到文件最后一块，将该块链接到块尾
{
    m_blockInfo->file=m_fileInfo;
    blockInfo *b=m_fileInfo->firstBlock;
    if (b==NULL)    //该文件无块
        m_fileInfo->firstBlock=m_blockInfo;
    else
    {
        while (b->next!=NULL) {//找到最后一块
            b=b->next;
        }
        b->next=m_blockInfo;
    }
}
fileInfo* BufferManager::get_file_info(string DB_Name,string File_Name,int m_fileType)//得到文件名
{
    fileInfo* file=FileHandle;
    while (file!=NULL) { //遍历文件链表，找到文件名与之相同的文件
        if(file->fileName==File_Name)
        {
            return file;
        }
        file=file->Next;
    }
    //文件链表没有该文件，则重新构建
    if (fileCount<5) { //直接从内存中申请,初始化，并作为fileHandle的头，返回file
        file=new fileInfo;
        file->busy=0;
        file->FileBlockNum=0;
        file->fileName=File_Name;
        file->firstBlock=NULL;
        file->Next=FileHandle;
        FileHandle=file;
        return file;
    }
    else//文件已满，选择一个文件来替换
    {
        file=FileHandle;
        while (file->busy) { //找到一个文件的块未被使用的
            file=file->Next;
        }
        closeFile(DB_Name, file, m_fileType);//关闭该文件，并把其中的块放进垃圾链表
    }
    //重新申请一个文件头，初始化并设为fileHandle的头
    file=new fileInfo;
    file->busy=0;
    file->FileBlockNum=0;
    file->fileName=File_Name;
    file->firstBlock=NULL;
    file->Next=FileHandle;
    FileHandle=file;
    return file;
}
void BufferManager::closeFile(string DB_Name, fileInfo *file, int type)
{
    
}
void BufferManager::writeBlock(string DB_Name,blockInfo* block)
{
    
}
blockInfo* BufferManager::readBlock(FILE *fp, int blockNum)
{
    
}