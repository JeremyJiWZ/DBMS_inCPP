#include "IndexManager.h"

int IndexManager::create(string DBName, string tableName, string indexName, int attributeBytes) {
	blockNumber = rootNumber = 1;
}

int IndexManager::insert(string DBName, string tableName, string indexName, int attributeValue, int recordOffset) {
	blockInfo block;
	get_file_block(DBName, tableName, INDEXFILE, 0, block);
	readIndexFileHeader(block);
}

int IndexManager::delete(string DBName, string tableName, string indexName, int attributeValue) {
	blockInfo block;
	get_file_block(DBName, tableName, INDEXFILE, 0, block);
	readIndexFileHeader(block);
}

int IndexManager::select(string DBName, string tableName, string indexName, int attributeValue, Condition cond, vector<char> &results) {
	blockInfo block;
	get_file_block(DBName, tableName, INDEXFILE, 0, block);
	readIndexFileHeader(block);
}

int IndexManager::drop(string DBName, string tableName, string indexName) {
	blockInfo block;
	get_file_block(DBName, tableName, INDEXFILE, 0, block);
	readIndexFileHeader(block);
}

void IndexManager::readIndexFileHeader(blockInfo &block) {
}
