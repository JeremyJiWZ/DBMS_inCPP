### IndexManager Interface

向IndexManager传属性值的时候先封装成一个Value类，构造一个Value类的接口见Value.hpp

#### create index

attributeBytes：属性值的长度

传入一串属性值和记录的偏移量

``` c++
int create(string DBName, string tableName, string indexName, int attributeBytes, vector<Value> attributeValues, vector<int> recordOffsets);
```

#### insert into

传入属性值和记录偏移量

``` c++
int insertInto(string DBName, string tableName, string indexName, Value attributeValue, int recordOffset);
```

#### delete from

传入属性值

``` c++
int deleteFrom(string DBName, string tableName, string indexName, Value attributeValue);
```

#### select

传入要比较的属性值和比较条件，返回的一串记录偏移量通过 vector<char> 的引用获取

``` c++
int select(string DBName, string tableName, string indexName, Value attributeValue, Condition cond, vector<char> &results);
```

比较条件定义（要定义在外面）

``` c++
typedef enum {
    EQUAL,
    LESS,
    LESSEQUAL,
    GREAT,
    GREATEQUAL
} Condition;
```

#### drop index/table

``` c++
int drop(string DBName, string tableName, string indexName);
```

