#include "../inc/passwordConfirm.h"
#include "../inc/main.h"

static void getSalt(char *salt, const char *systemPassword){
	int i, j;
	//取出salt，i记录密码字符下标，j记录$出现次数
	for(i = 0,j = 0; systemPassword[i] && j != 3; ++i){
		if(systemPassword[i] == '$'){
			j++;
		}
	}
	strncpy(salt, systemPassword, i - 1);
}

int passwordConfirm(const char *userName, const char *password){
	struct spwd *sp;
	//得到用户名
	sp = getspnam(userName);
	if(sp == NULL){
		printf("错误：用户名不存在！\n");
		return -1;
	}

	//得到salt
	char salt[512] = {0};
	getSalt(salt, sp->sp_pwdp);
	
	//进行密码验证
	if(strcmp(sp->sp_pwdp, crypt(password,salt)) == 0){
		printf("验证通过！\n");
		return 0;
	}
	else{
		printf("验证失败！\n");
		return -1;
	}






}