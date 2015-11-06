
#ifndef CONDITION_H
#define CONDITION_H
#include <vector>
#include <string>
#include "value.hpp"

#define DATAFILE 0
#define CTG_INT  0
#define CTG_CHAR 1
#define CTG_FLOAT 2


typedef enum {
    EQUAL,
    LESS,
    LESSEQUAL,
    GREAT,
    GREATEQUAL,
    NOTEQUAL
}Condition;




#endif 
