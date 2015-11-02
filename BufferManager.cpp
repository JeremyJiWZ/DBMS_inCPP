//
//  BufferManager.cpp
//  DBMSSev
//
//  Created by mr.ji on 15/10/22.
//  Copyright (c) 2015年 mr.ji. All rights reserved.
//

#include "BufferManager.h"
//initialization the member variable
int BufferManager::blockCount=0;
int BufferManager::fileCount=0;
fileInfo* BufferManager::FileHandle=NULL;
blockInfo* BufferManager::BlockHandle=NULL;

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
    //程序将cBlock初始化
    blockInfo *block;
    block=BlockHandle;
    if (block!=NULL) {//垃圾链表中的块
        BlockHandle=BlockHandle->next;
        return block;
    }
    else if(blockCount<BLOCK_MAX){//若内存中块数未达到上限，申请一块并返回
        block=new blockInfo;
        block->cBlock=new char[BLOCK_LEN];
        blockCount++;
        return block;
    }
    else    //使用LRU算法找到替换块,找到最大的itime，将该块替换出去
    {
        fileInfo* file = FileHandle;
        blockInfo* bp;
        block=file->firstBlock;
        while (file!=NULL) {//遍历找到使用时间最大的块
            bp=file->firstBlock;
            while (bp!=NULL) {
                if (bp->iTime>=block->iTime&&bp->lock!=0)//该块的使用时间大且未被锁上
                    block=bp;
                bp=bp->next;
            }
            file=file->Next;
        }
        if (block->dirtyBlock) {//换出脏块
            writeBlock(DB_Name, block);//写回磁盘
        }
    }
    return block;
}
void BufferManager::replce(fileInfo *m_fileInfo,blockInfo *m_blockInfo)//找到文件最后一块，将该块链接到块尾
{
    if (m_fileInfo==NULL){//空指针错误
        cout<<"in BufferManager::replace:传入空文件头"<<endl;
        exit(0);
    }
    if (m_blockInfo==NULL) {//空指针错误
        cout<<"in BufferManager::replace:传入空块头"<<endl;
        exit(0);
    }
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
fileInfo* BufferManager::get_file_info(string DB_Name,string File_Name,int m_fileType)//得到文件头,并且初始化
{
    fileInfo* file=FileHandle;
    while (file!=NULL) { //遍历文件链表，找到文件名与之相同的文件
        if(file->fileName==File_Name&&file->type==m_fileType)//内存中存在该文件，直接返回
        {
            return file;
        }
        file=file->Next;
    }
    //文件链表没有该文件，则重新构建
    if (fileCount<BLOCK_FILE_ACTIVE) { //直接从内存中申请,初始化，并作为fileHandle的头，返回file
        fileCount++; //文件数量++
        file=new fileInfo;
        string filePath;
        //路径名：DB_Name\data\File_Name
        //或者   DB_Name\index\File_Name
        //windows系统下用\\，Linux下用/
        //Linux系统
#ifdef linux
         if (m_fileType==0)
             filePath=DB_Name+"/data/"+File_Name;
         else
             filePath=DB_Name+"/index/"+File_Name;
#endif
        
        //windowns 系统
#ifndef linux
        if (m_fileType==0)
            filePath=DB_Name+"\\data\\"+File_Name;
        else
            filePath=DB_Name+"\\index\\"+File_Name;
#endif
        
        file->fp.open(filePath.c_str(),ios::binary|ios::out|ios::in);
        if (!file->fp.is_open()) {//文件打开失败
            cout<<"in BufferManager::get_file_info,文件打开失败"<<endl;
            exit(0);
        }
        //文件打开成功
        file->fp.seekg(0,ios::end);
        file->type=m_fileType;
        file->busy=0;
        file->FileBlockNum=file->fp.tellg()/BLOCK_LEN;
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
        closeFile(DB_Name, File_Name, m_fileType);//关闭该文件，并把其中的块放进垃圾链表
    }
    //重新申请一个文件头，初始化并设为fileHandle的头
    file=new fileInfo;
    string filePath;
    //路径名：DB_Name\data\File_Name
    //或者   DB_Name\index\File_Name
    //windows系统下用\\，Linux下用/
    
     //Linux系统
#ifdef linux
    if (m_fileType==0)
        filePath=DB_Name+"/data/"+File_Name;
    else
        filePath=DB_Name+"/index/"+File_Name;
#endif
    //windowns 系统
#ifndef linux
    if (m_fileType==0)
        filePath=DB_Name+"\\data\\"+File_Name;
    else
        filePath=DB_Name+"\\index\\"+File_Name;
#endif
    
    file->fp.open(filePath.c_str(),ios::binary);
    if (!file->fp.is_open()) {//文件打开失败
        cout<<"in BufferManager::get_file_info,文件打开失败"<<endl;
        exit(0);
    }
    //文件打开成功
    file->fp.seekg(0,ios::end);
    file->type=m_fileType;
    file->busy=0;
    file->FileBlockNum=file->fp.tellg()/BLOCK_LEN;
    file->fileName=File_Name;
    file->firstBlock=NULL;
    file->Next=FileHandle;
    FileHandle=file;
    return file;
}
void BufferManager::closeFile(string DB_Name, string File_Name, int type)//若文件在内存中，关闭文件；若不在，直接返回
{
    fileInfo *file;
    blockInfo *block,*bp;
    file=FileHandle;
    while (file!=NULL&&(file->fileName!=File_Name||type!=file->type)) {//找到符合要求的文件头
        file=file->Next;
    }
    if (file==NULL) //该文件不在内存中，直接返回
        return;
    block=bp=file->firstBlock;
    //防止不正常操作
    if (bp==NULL) {//正常情况下文件链表下必有块
        cout<<"in BufferManager::closeFile,文件链表下没有链接块"<<endl;
        exit(0);
    }
    //将块扔到垃圾链表中
    while (bp->next!=NULL) {//找到块尾，将其next指向blockHandle，并且重置blockHandle
        if(bp->dirtyBlock)//假如该块为脏块
            writeBlock(DB_Name, bp);//写入磁盘
        bp=bp->next;
    }
    if(bp->dirtyBlock)
        writeBlock(DB_Name, bp);
    bp->next=BlockHandle;
    BlockHandle=block;
    
    file->fp.close();//关闭文件
    delete file;//释放文件
    return;
}
void BufferManager::writeBlock(string DB_Name,blockInfo* block)//将该块写回磁盘，不涉及其他操作
{
    if (block==NULL||block->file==NULL) {
        cout<<"in BufferManager::writeBlock,空指针错误"<<endl;
        exit(0);
    }
    block->file->fp.seekp(BLOCK_LEN*block->blockNum);//调整写OFFSET
    block->file->fp.write(block->cBlock, BLOCK_LEN);//写数据到磁盘
}
blockInfo* BufferManager::readBlock(string DB_Name,fileInfo* file, int blockNum)//将需要的块从文件中读出，文件必须已经打开
{
    //程序将block的cBlock、file、next、dirty、lock、itime等全部初始化
    blockInfo* block=findBlock(DB_Name);//调用findBlock得到一个空块
    //初始化
    block->dirtyBlock=0;
    block->iTime=0;
    block->lock=0;
    block->file=file;
    block->blockNum=blockNum;
    block->charNum=BLOCK_LEN;

    if (!file->fp.is_open()){//文件无法打开
        cout<<"in BufferManager::readBlock,文件无法打开";
        exit(0);
    }
    file->fp.seekg(BLOCK_LEN*blockNum);//调整读指针OFFSET
    file->fp.read(block->cBlock, BLOCK_LEN);//读磁盘中的块内值
    blockInfo* bp;//工作指针
    //将该块链接到该文件头下
    bp=file->firstBlock;
    block->next=bp;
    file->firstBlock=block;
    
    return block;
}
blockInfo* BufferManager::get_file_block(string DB_Name, string File_Name, int fileType, int blockNum)
{
    fileInfo* file=FileHandle;
    blockInfo* block;
    while (file!=NULL) {
        if (file->type==fileType&&file->fileName==File_Name) {//找到该文件
            break;
        }
        file=file->Next;
    }
    if (file==NULL) {//内存中找不到该文件
        file=get_file_info(DB_Name, File_Name, fileType);//得到已经初始化的文件头
        int numOfBlock=file->FileBlockNum;
        //将所要求的块读到内存中
        if (blockNum<numOfBlock) {//申请访问的块不超出文件大小
            block=readBlock(DB_Name, file, blockNum);//从磁盘中读入此块,且该块已经初始化
            addTime(DB_Name);
            block->iTime=0;
            return block;
        }
        else            //申请访问的块在文件大小之外，重新申请一个块
        {
            file->FileBlockNum=blockNum+1;//文件变长，块数增加
            block=findBlock(DB_Name);//得到一个块，未初始化
            file->firstBlock=block;
            block->file=file;
            block->blockNum=blockNum;
            block->dirtyBlock=0;
            block->next=NULL;
            block->charNum=0;
            block->lock=0;
            addTime(DB_Name);
            block->iTime=0;
            return block;
        }
    }
    else            //内存中找到了该文件
    {
        //在文件的块链表下循环找，若找到该块直接返回即可；否则需要重新创建
        block=file->firstBlock;
        while (block!=NULL) {
            if(block->blockNum==blockNum)//找到了所要求找的块
            {
                addTime(DB_Name);
                return block;//直接返回
            }
            block=block->next;
        }
        //没有找到该块，不在内存中，则要重新构建一个块
        if (blockNum<file->FileBlockNum) {    //申请的块号在文件大小范围之内
            block=readBlock(DB_Name, file, blockNum);//从磁盘中读入此块,且该块已经初始化
            addTime(DB_Name);
            block->iTime=0;
            return block;
        }
        else
        {
            file->FileBlockNum=blockNum+1;//文件变长，块数增加
            block=findBlock(DB_Name);//得到一个块，未初始化
            block->next=file->firstBlock;//将该块置入文件链表
            file->firstBlock=block;
            block->file=file;
            block->blockNum=blockNum;
            block->dirtyBlock=0;
            block->charNum=0;
            block->lock=0;
            addTime(DB_Name);
            block->iTime=0;
            return block;
        }
    }
}
void BufferManager::CreateFile(string DB_Name, string File_Name,int type)
{
    fstream fp;
    string filePath;
    //路径名：DB_Name\data\File_Name
    //或者   DB_Name\index\File_Name
    //windows系统下用\\，Linux下用/
    
    //Linux系统
#ifdef linux
     if (type==0)
        filePath=DB_Name+"/data/"+File_Name;
     else
        filePath=DB_Name+"/index/"+File_Name;
#endif
    //windowns 系统
#ifndef linux
    if (type==0)
        filePath=DB_Name+"\\data\\"+File_Name;
    else
        filePath=DB_Name+"\\index\\"+File_Name;
#endif
    
    fp.open(filePath.c_str(),ios::binary|ofstream::out);
    if (!fp.is_open()) {//文件打开失败
        cout<<"in BufferManager::CreateFile,文件创建失败"<<endl;
        exit(0);
    }
    fp.close();
}
void BufferManager::DeleteFile(string DB_Name, string File_Name,int type)
{
    //检查文件是否在内存中，若是，先关闭文件
    closeFile(DB_Name, File_Name, type);
    
    string filePath;
    //路径名：DB_Name\data\File_Name
    //或者   DB_Name\index\File_Name
    //windows系统下用\\，Linux下用/
    
    //Linux系统
#ifdef linux
     if (type==0)
        filePath=DB_Name+"/data/"+File_Name;
     else
        filePath=DB_Name+"/index/"+File_Name;
     string fileCommand="rm "+filePath;
     system(fileCommand.c_str());
#endif
    //windowns 系统
#ifndef linux
    if (type==0)
        filePath=DB_Name+"\\data\\"+File_Name;
    else
        filePath=DB_Name+"\\index\\"+File_Name;
    string fileCommand="del "+filePath;
    system(fileCommand.c_str());
#endif
}
void BufferManager::quitProg(string DB_Name)
{
    fileInfo* file;
    fileInfo* fileWork;//工作指针
    file=FileHandle;
    //将文件头逐个关闭
    while (file!=NULL)
    {
        fileWork=file;
        file=file->Next;
        closeFile(DB_Name, fileWork->fileName, fileWork->type);//关闭工作指针所指文件头
    }
    //释放垃圾链表
    blockInfo* block,*blockWork;//块链表的头指针以及工作指针
    block=BlockHandle;
    //逐个释放垃圾链表
    while (block!=NULL) {
        blockWork=block;
        block=block->next;
        delete[] blockWork->cBlock;
        delete blockWork;
    }
}
bool BufferManager::HasFile(string DB_Name, string File_Name, int type)
{
    fstream fp;
    string filePath;
    fileInfo* file;
    file=FileHandle;
    while (file!=NULL) {//文件链表中有该文件
        if(file->fileName==File_Name&&file->type==type)
            return true;
        file=file->Next;
    }
    //若文件链表中没有该文件，那么打开文件
    
    //路径名：DB_Name\data\File_Name
    //或者   DB_Name\index\File_Name
    //windows系统下用\\，Linux下用/
    
    //Linux系统
#ifdef linux
     if (type==0)
         filePath=DB_Name+"/data/"+File_Name;
     else
         filePath=DB_Name+"/index/"+File_Name;
#endif
    //windowns 系统
#ifndef linux
    if (type==0)
        filePath=DB_Name+"\\data\\"+File_Name;
    else
        filePath=DB_Name+"\\index\\"+File_Name;
#endif
    fp.open(filePath.c_str(),ios::in);
    if (!fp) //文件没被打开
        return false;
    else    //文件被打开
        fp.close();
    return true;
}
int BufferManager::getBlockNum(string DB_Name, string File_Name, int type)
{
    fileInfo* file;
    file=FileHandle;
    while (file!=NULL) {//文件链表中有该文件
        if(file->fileName==File_Name&&file->type==type)
            return file->FileBlockNum;
        file=file->Next;
    }
    //文件链表中没有该文件，则去磁盘中寻找
    fstream fp;
    string filePath;
    //路径名：DB_Name\data\File_Name
    //或者   DB_Name\index\File_Name
    //windows系统下用\\，Linux下用/
    
    //Linux系统
#ifdef linux
     if (type==0)
         filePath=DB_Name+"/data/"+File_Name;
     else
         filePath=DB_Name+"/index/"+File_Name;
#endif
    //windowns 系统
#ifndef linux
    if (type==0)
        filePath=DB_Name+"\\data\\"+File_Name;
    else
        filePath=DB_Name+"\\index\\"+File_Name;
#endif
    fp.open(filePath.c_str(),ios::in);
    if (!fp) //文件没被打开
    {
        cout<<"in BufferManager::getBlockNum,文件不存在"<<endl;
        exit(0);
    }
    else    //文件被打开
    {
        fp.seekg(0,ios::end);
        int numOfBlock=fp.tellg()/BLOCK_LEN;
        fp.close();//文件关闭
        return numOfBlock;
    }
}
void BufferManager::UseDB(string DB_Name)//创建路径，三个目录
{
    //Linux系统
#ifdef linux
     string commandMD="mkdir "+DB_Name;
     string commandMD1="mkdir "+DB_Name+"/data";
     string commandMD2="mkdir "+DB_Name+"/index";
     system("pwd");
     system(commandMD.c_str());
     system(commandMD1.c_str());
     system(commandMD2.c_str());
#endif
    
    
    //windows系统
#ifndef linux
    string commandMD="md "+DB_Name;
    string commandMD1="md "+DB_Name+"//data";
    string commandMD2="md "+DB_Name+"//index";
    system(commandMD.c_str());
    system(commandMD1.c_str());
    system(commandMD2.c_str());
#endif
}

