//
//  main.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/28.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#include <iostream>
#include <string>
#include <sstream>
#include "IndexManager.hpp"

typedef struct {
    int intValue;
    float floatValue;
    string stringValue;
} Tuple;

int main(int argc, const char * argv[]) {
    // insert code here...
    vector<Tuple> value;
    ifstream fin("/Users/Sylvanus/Sylvanus's Library/Studies/CS/Database/Design/DBMS/DBMS/data.txt");
    if (fin.is_open()) {
        string line;
        getline(fin, line);
        getline(fin, line);
        int st, ed;
        Tuple tuple;
        while (getline(fin, line)) {
            st = (int)line.find("(", 0)+1;
            ed = (int)line.find(",", st);
            stringstream stream;
            stream<<line.substr(st, ed-st);
            int aInt;
            stream>>aInt;
            tuple.intValue = aInt;
            
            st = (int)line.find("'", ed)+1;
            ed = (int)line.find("'", st);
            tuple.stringValue = line.substr(st, ed-st);
            
            st = ed+2;
            ed = (int)line.find(")", st);
            stream.clear();
            stream<<line.substr(st, ed);
            float aFloat;
            stream>>aFloat;
            tuple.floatValue = aFloat;
            
            value.push_back(tuple);
        }
    }
    
    random_shuffle(value.begin(), value.end());
    
    vector<Tuple>::iterator itor;
    vector<int> offset;
    vector<Value> intValue;
    int count = 0;
    for (itor = value.begin(); itor!=value.end(); itor++) {
        offset.push_back(++count);
        intValue.push_back(Value((*itor).intValue));
//        if (count>1000)
//            break;
    }
    
    BufferManager::UseDB("TEST1");
    BufferManager::CreateFile("TEST1", "t1.index", 1);
    IndexManager indexManager;
    indexManager.create("TEST1", "Table", "t1.index", sizeof(int), intValue, offset);
    indexManager.tree.printTree(0);
    cout<<"create done."<<endl;
//    Value val(1080108009);
//    vector<int> results;
//    indexManager.select("TEST1", "Table", "t1.index", val, EQUAL, results);
//    vector<int>::iterator i;
//    for (i = results.begin(); i != results.end(); i++) {
//        cout<<*i<<" ";
//    }
    random_shuffle(intValue.begin(), intValue.end());
    vector<Value>::iterator intItor;
    int i = 300;
    for (intItor = intValue.begin(); intItor != intValue.end(); intItor++) {
        indexManager.deleteFrom("TEST1", "Table", "t1.index", *intItor);
//        indexManager.tree.printTree(++i);
        cout<<"delete"<<i<<": "<<(*intItor).intValue<<endl;
    }
    indexManager.tree.printTree(1);
    
    return 0;
}
