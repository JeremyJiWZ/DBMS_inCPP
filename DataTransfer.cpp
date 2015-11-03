//
//  DataTransfer.cpp
//  DBMS
//
//  Created by 张函祎 on 15/10/31.
//  Copyright © 2015年 Sylvanus. All rights reserved.
//

#include "DataTransfer.hpp"

byte DataTransfer::readByte(byte *block, int &fp) {
    byte *data = block+fp;
    fp += sizeof(byte);
    return *data;
}

void DataTransfer::writeByte(byte byteData, byte *block, int &fp) {
    byte *data = (byte *)(block+fp);
    *data = byteData;
    fp += sizeof(byteData);
}

short DataTransfer::readShort(byte *block, int &fp) {
    short *data = (short *)(block+fp);
    fp += sizeof(short);
    return *data;
}

void DataTransfer::writeShort(short shortData, byte *block, int &fp) {
    short *data = (short *)(block+fp);
    *data = shortData;
    fp += sizeof(shortData);
}

int DataTransfer::readInt(byte *block, int &fp) {
    int *data = (int *)(block+fp);
    fp += sizeof(int);
    return *data;
}

void DataTransfer::writeInt(int intData, byte *block, int &fp) {
    int *data = (int *)(block+fp);
    *data = intData;
    fp += sizeof(intData);
}

float DataTransfer::readFloat(byte *block, int &fp) {
    float *data = (float *)(block+fp);
    fp += sizeof(float);
    return *data;
}

void DataTransfer::writeFloat(float floatData, byte *block, int &fp) {
    float *data = (float *)(block+fp);
    *data = floatData;
    fp += sizeof(floatData);
}

string DataTransfer::readString(byte *block, int &fp, int length) {
    char data[256];
    strncpy(data, (char *)(block+fp), length);
    fp += length;
    return string(data);
}

void DataTransfer::writeString(string stringData, byte *block, int &fp, int length) {
    strncpy((char*)(block+fp), stringData.c_str(), length);
    fp += length;
}
