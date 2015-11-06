#include "interpreter.h"

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

//验证drop database语句是否有效
void drop_database(string SQL,int start, SQL_CLAUSE & sql_cla)
{
	int pos1, pos2, pos3;
	pos1 = SQL.find_first_not_of(' ', start);
	pos2 = SQL.find_first_of(' ', pos1);
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
	{
		sql_cla.type = ERROR;
		return ;
	}
	sql_cla.type = DROPDATABASE;
	sql_cla.name = SQL.substr(pos1, pos2-pos1);
	return ;
}

//验证drop table语句是否有效
void drop_table(string SQL,int start, SQL_CLAUSE & sql_cla)
{
	int pos1, pos2, pos3;
	pos1 = SQL.find_first_not_of(' ', start);
	pos2 = SQL.find_first_of(' ', pos1);
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
	{
		sql_cla.type = ERROR;
		return ;
	}
	sql_cla.type = DROPTABLE;
	sql_cla.name = SQL.substr(pos1, pos2-pos1);
	return ;
}

//验证drop index语句是否有效
void drop_index(string SQL,int start, SQL_CLAUSE & sql_cla)
{
	int pos1, pos2, pos3;
	pos1 = SQL.find_first_not_of(' ', start);
	pos2 = SQL.find_first_of(' ', pos1);
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
	{
		sql_cla.type = ERROR;
		return ;
	}
	sql_cla.type = DROPINDEX;
	sql_cla.name = SQL.substr(pos1, pos2-pos1);
	return ;
}

//验证drop语句是否有效
SQL_CLAUSE drop_clause(string SQL,int start)
{
	SQL_CLAUSE ret;
	string st;
	int pos1, pos2;
	while ((pos1 = SQL.find_first_of('\t')) != string::npos)
		SQL.replace(pos1, 1, " ");
	while (SQL.at(0) == ' ')
		SQL.erase(0, 1);

	pos1 = SQL.find_first_not_of(' ', 0);
	pos2 = SQL.find_first_of(' ', pos1);
	if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "drop")
	{
		ret.type = ERROR;
		return ret;
	}
	pos1 = SQL.find_first_not_of(' ', pos2);
	pos2 = SQL.find_first_of(' ', pos1);
	if (pos1 == string::npos || pos2 == string::npos)
	{
		ret.type = ERROR;
		return ret;
	}
	if (SQL.substr(pos1, pos2-pos1) == "database")
	{
		drop_database(SQL, pos2, ret);
		return ret;
	}
	if (SQL.substr(pos1, pos2-pos1) == "table")
	{
		drop_table(SQL, pos2, ret);
		return ret;
	}
	if (SQL.substr(pos1, pos2-pos1) == "index")
	{
		drop_index(SQL, pos2, ret);
		return ret;
	}
	ret.type = ERROR;
	return ret;
}

//验证use语句是否有效
SQL_CLAUSE use_clause(string SQL,int start)
{
	SQL_CLAUSE ret;
	string st;
	int pos1, pos2, pos3;
	while ((pos1 = SQL.find_first_of('\t')) != string::npos)
		SQL.replace(pos1, 1, " ");
	while (SQL.at(0) == ' ')
		SQL.erase(0, 1);

	pos1 = SQL.find_first_not_of(' ', 0);
	pos2 = SQL.find_first_of(' ', pos1);
	if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "use")
	{
		ret.type = ERROR;
		return ret;
	}
	pos1 = SQL.find_first_not_of(' ', pos2);
	pos2 = SQL.find_first_of(' ', pos1);
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
	{
		ret.type = ERROR;
		return ret;
	}
	ret.type = USE;
	ret.name = SQL.substr(pos1, pos2-pos1);
	return ret;
}

//验证execfile语句是否有效
SQL_CLAUSE execfile_clause(string SQL,int start)
{
	SQL_CLAUSE ret;
	string st;
	int pos1, pos2, pos3;
	while ((pos1 = SQL.find_first_of('\t')) != string::npos)
		SQL.replace(pos1, 1, " ");
	while (SQL.at(0) == ' ')
		SQL.erase(0, 1);

	pos1 = SQL.find_first_not_of(' ', 0);
	pos2 = SQL.find_first_of(' ', pos1);
	if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "execfile")
	{
		ret.type = ERROR;
		return ret;
	}
	pos1 = SQL.find_first_not_of(' ', pos2);
	pos2 = SQL.find_first_of(' ', pos1);
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos1 == string::npos || pos2 == string::npos || pos3 == string::npos || SQL.at(pos3) != ';')
	{
		ret.type = ERROR;
		return ret;
	}
	ret.type = EXECFILE;
	ret.name = SQL.substr(pos1, pos2-pos1);
	return ret;
}

//验证quit语句是否有效
SQL_CLAUSE quit_clause(string SQL,int start)
{
	SQL_CLAUSE ret;
	string st;
	int pos1, pos2, pos3;
	while ((pos1 = SQL.find_first_of('\t')) != string::npos)
		SQL.replace(pos1, 1, " ");
	while (SQL.at(0) == ' ')
		SQL.erase(0, 1);

	pos1 = SQL.find_first_not_of(' ', 0);
	pos2 = SQL.find_first_of(' ', pos1);
	if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "use")
	{
		ret.type = ERROR;
		return ret;
	}
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos3 == string::npos || SQL.at(pos3) != ';')
	{
		ret.type = ERROR;
		return ret;
	}
	ret.type = QUIT;
	return ret;
}

//验证help语句是否有效
SQL_CLAUSE help_clause(string SQL,int start)
{
	SQL_CLAUSE ret;
	string st;
	int pos1, pos2, pos3;
	while ((pos1 = SQL.find_first_of('\t')) != string::npos)
		SQL.replace(pos1, 1, " ");
	while (SQL.at(0) == ' ')
		SQL.erase(0, 1);

	pos1 = SQL.find_first_not_of(' ', 0);
	pos2 = SQL.find_first_of(' ', pos1);
	if (pos1 == string::npos || pos2 == string::npos || SQL.substr(pos1, pos2-pos1) != "use")
	{
		ret.type = ERROR;
		return ret;
	}
	pos3 = SQL.find_first_not_of(' ', pos2);
	if (pos3 == string::npos || SQL.at(pos3) != ';')
	{
		ret.type = ERROR;
		return ret;
	}
	ret.type = QUIT;
	return ret;
}