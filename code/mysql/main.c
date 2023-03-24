#include <stdio.h>
#include <mysql.h>

bool connectdatabase(const MYSQL* mysql, const char* dbName, unsigned long clientflag)
{
	//设置连接超时的时间
	int timeout = 3;
	if (0 != mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout))
	{
		printf("mysql_options failed %s\n", mysql_error(mysql));
	}
	//设置断开重连
	bool reconnect = true;
	if (0 != mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect))
	{
		printf("mysql_options failed %s\n", mysql_error(mysql));
	}

	// 设置编码格式
	mysql_options(mysql, MYSQL_INIT_COMMAND, "SET NAMES GBK");
	// 连接mysql服务器
	if (!mysql_real_connect(mysql, "localhost", "root", "741106", dbName, 3306, NULL, clientflag))
	{
		printf("connect failed: %s", mysql_error(mysql));
		return -1;
	}
}
//执行SQL语句并获取结果集
void test01(const MYSQL* mysql)
{
	connectdatabase(mysql, "shop", 0);  // 连接数据库
	//1，执行SQL语句
	const char* sql = "select * from category";
	//mysql_real_query(mysql，Sq1，strlen(sq1)）
	if (0 != mysql_query(mysql, sql))
	{
		printf("mysql_query failed:%s\n", mysql_error(mysql));
	}
	//2，获取结果集
	MYSQL_RES* result = mysql_store_result(mysql); // 会把结果集保存到本机
	//MYSQL_RES* result = mysql_use_result(mysql); //不会把结果集保存起来，而是一条记录一条记录的从服务器
	if (!result)
	{
		printf("mysql_store_result failed %s\n", mysql_error(mysql));
	}
	else
	{
		//获取字段数量
		//unsigned int fieldCnt = mysql_field_count(mysql);
		unsigned int fieldCnt = mysql_num_fields(result);
		//获取记录的条数
		unsigned int rowCnt = mysql_num_rows(result);
		//获取每个字段，并输出
		MYSQL_FIELD* field = NULL;
#define FIELDNO 1
#if FIELDNO == 0
		// 方法1:
		while (field = mysql_fetch_field(result))
		{
			printf("%-10s", field->name);
		}
		putchar('\n');
#elif FIELDNO == 1
		//方法2: 先把所有的字段都抓取出来，然后去遍历(推荐)
		field = mysql_fetch_fields(result);
		if (field)
		{
			for (int i = 0; i < fieldCnt; i++)
			{
				printf("%-*s ", field[i].max_length, field[i].name);
			}
		}
		putchar('\n');
#elif FIELDNO == 2
		for (int i = 0; i < fieldCnt; i++)
		{
			printf("%-10s", mysql_fetch_field_direct(result, i)->name);
		}
		putchar('\n');
#endif
		//获取每条记录
		MYSQL_ROW row = NULL;
		while (row = mysql_fetch_row(result))
		{
			for (int i = 0; i < fieldCnt; i++)
			{
				printf("%-*s ", field[i].max_length, row[i] ? row[i] : "NULL");
			}
			putchar('\n');
		}
		//必须释放结果集
		mysql_free_result(result);
	}
}

//执行多条SQL语句并获取结果集
void test02(const MYSQL* mysql)
{
	//CLIENT_MULTI_STATEMENTS支持多条
	connectdatabase(mysql, "shop", CLIENT_MULTI_STATEMENTS);  // 连接数据库
	//1，执行SQL语句
	const char* sql = "select * from category;  select * from account; insert into account(id, name, cash) values(3, 'jj', 200);";
	//mysql_real_query(mysql，Sq1，strlen(sq1)）
	if (0 != mysql_query(mysql, sql))
	{
		printf("mysql_query failed:%s\n", mysql_error(mysql));
	}
	//2，获取结果集
	do
	{
		MYSQL_RES* result = mysql_store_result(mysql); // 会把结果集保存到本机
		//MYSQL_RES* result = mysql_use_result(mysql); //不会把结果集保存起来，而是一条记录一条记录的从服务器
		if (!result)
		{
			//没有结果集 但是有字段，就产生了错误
			if (mysql_field_count(mysql) > 0)
			{
				printf("mysql_store_result failed %s\n", mysql_error(mysql));
			}
			//当执行insert update delete没有结果集，有的只是影响的行数
			else
			{
				printf("affected row： %1lu\n", mysql_affected_rows(mysql)); // 显示影响数
			}
		}
		else
		{// select语句结果
			//获取字段数量
			//unsigned int fieldCnt = mysql_field_count(mysql);
			unsigned int fieldCnt = mysql_num_fields(result);
			//获取记录的条数
			unsigned int rowCnt = mysql_num_rows(result);
			//获取每个字段，并输出
			MYSQL_FIELD* field = NULL;
			//方法2: 先把所有的字段都抓取出来，然后去遍历(推荐)
			field = mysql_fetch_fields(result);
			if (field)
			{
				for (int i = 0; i < fieldCnt; i++)
				{
					printf("%-*s ", field[i].max_length + 1, field[i].name);
				}
			}
			putchar('\n');

			//获取每条记录
			MYSQL_ROW row = NULL;
			while (row = mysql_fetch_row(result))
			{
				for (int i = 0; i < fieldCnt; i++)
				{
					printf("%-*s ", field[i].max_length + 1, row[i] ? row[i] : "NULL");
				}
				printf("\n");
			}
			//必须释放结果集
			mysql_free_result(result);
		}
		putchar('\n');
	} while (mysql_next_result(mysql) == 0);
	if (mysql_errno(mysql) != 0)
	{
		printf("error :%s\n", mysql_error(mysql));
	}
}

int main(void)
{
	// 初始化mysql
	MYSQL* mysql = mysql_init(NULL);
	//test01(mysql);
	test02(mysql);
	mysql_close(mysql);
	return 0;
}