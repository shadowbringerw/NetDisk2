#ifndef __PASSWORDCONFIRM_H__
#define __PASSWORDCONFIRM_H__
#include <shadow.h>
#include <crypt.h>
#include <string.h>
#include <stdio.h>

//对客户端传来的密码进行验证
int passwordConfirm(const char *userName, const char *password);

#endif