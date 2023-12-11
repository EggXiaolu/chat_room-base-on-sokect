#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <mysql/mysql.h>
#include <pthread.h>
#include <unistd.h>
#include <inet.h>
using namespace std;
int main()
{
    MYSQL *mysql = mysql_init(NULL);
    cout << inet_addr("127.0.0.1") << endl;
    return 0;
}
