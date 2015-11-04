#include<interpreter.h>

string read_input()
{
    string SQL;
    string temp;
    char str[100];
    bool finish=false;
    while(!finish)
    {
        cin>>str;
        temp=str;
        SQL=SQL+" "+temp;
        if(SQL.at(SQL.length()-1)==';')
        {
            SQL[SQL.length()-1] = ' ';
            SQL+=";";
            finish=true;
        }
    }
    //将输入大写转化为小写
    //SQL.MakeLower();
    //返回用户输入
    return SQL;
}