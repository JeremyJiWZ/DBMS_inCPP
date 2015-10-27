#ifndef __IndexManager__
#define __IndexManager__

#include "BufferManager.h"
#include <string>
#include <vector>
using namespace std;

class IndexManager {
private:
	int blockNumber;
	int rootBlock;
	int nodeNumber;

	void readIndexFileHeader(blockInfo &block);
public:
	int create(string DBName, string tableName, string indexName, int attributeBytes);
	int insert(string DBName, string tableName, string indexName, int attributeValue, int recordOffset);
	int delete(string DBName, string tableName, string indexName, int attributeValue);
	int select(string DBName, string tableName, string indexName, int attributeValue, Condition cond, vector<char> &results);
	int drop(string DBName, string tableName, string indexName);
};

#endif

/*
typedef enum {
	EQUAL;
	LESS;
	LESSEQUAL;
	GREAT;
	GREATEQUAL;
} Condition
*/