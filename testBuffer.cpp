#include"BufferManager.h"
int main()
{
    BufferManager::UseDB("TEST1");
    BufferManager::CreateFile("TEST1", "tt.data", 0);
    cout<<BufferManager::getBlockNum("TEST1", "tt.data", 0)<<endl;
    cout<<BufferManager::HasFile("TEST1", "tt.data", 0)<<endl;
    cout<<BufferManager::HasFile("TEST1", "tt", 0)<<endl;
    blockInfo* block;
    block= BufferManager::get_file_block("TEST1", "tt.data", 0, 0);//blocknum从0开始计数
    block->lock=1;
    for (int i=0; i<BLOCK_LEN; i++) {
        block->cBlock[i]=i;
    }
    block->dirtyBlock=1;
    block->lock=0;
    BufferManager::quitProg("TEST");
    
    
    
}

