//
//  DataTransfer.hpp
//  DBMS
//
//  Created by 张函祎 on 15/10/31.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#ifndef DataTransfer_hpp
#define DataTransfer_hpp

#include <stdio.h>
#include <string>
#include "BufferManager.h"
#include "Value.hpp"
using namespace std;

typedef unsigned char byte;

class DataTransfer {
public:
    static byte readByte(byte *block, int &fp);
    static void writeByte(byte byteData, byte *block, int &fp);
    static short readShort(byte *block, int &fp);
    static void writeShort(short shortData, byte *block, int &fp);
    static int readInt(byte *block, int &fp);
    static void writeInt(int intData, byte *block, int &fp);
    static float readFloat(byte *block, int &fp);
    static void writeFloat(float floatData, byte *block, int &fp);
    static string readString(byte *block, int &fp, int length);
    static void writeString(string stringData, byte *block, int &fp, int length);
};

#endif /* DataTransfer_hpp */
