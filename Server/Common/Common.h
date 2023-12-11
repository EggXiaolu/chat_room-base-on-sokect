
#ifndef _COMMON_H
#define _COMMON_H
#include<stdio.h>
#include<time.h>

typedef struct {
	int year;
	int month;
	int day;
}user_date_t;

typedef struct {
	int hour;
	int minute;
	int second;
}user_time_t;


//安全gets
char *sgets(char *str,int len);

//清空输入缓冲区
void ffflush();

//比较日期dt1, dt2的大小。相等返回0，dt1<dt2返回-1，否则1
int DateC\
mp(user_date_t dt1, user_date_t dt2);

//获取系统当前日期
user_date_t DateNow();

//获取系统当前时间
user_time_t TimeNow();

#endif
