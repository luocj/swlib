/*
* system_cookie.h
* cookie 生成与校验
*  Created on: 2016-5-23
*      Author: luochj
*/

#ifndef __SYSTEM_COOKIE_H
#define __SYSTEM_COOKIE_H

#include "base_def.h"
#include "base_namespace.h"
#include "../vendor/md5/md5.h"
#include "../vendor/aes/aes.h"

#include <string>

using namespace std;
using namespace BASE_NAMEPSACE_DECL;

enum CheckCerfityType
{
	eCerfitySucc = 0,
	eCerfityTimeOut,
	eUserIDError,
	eSignatureError,
	eCerfityDataError,
	eCerfityVersionError,
	eUnknownError,
};

uint32_t get_encrypt_version();
void	generate_key(uint32_t param, uint8_t* key, int32_t key_size, uint32_t version);
void	generate_key(const string& param, uint8_t* key, int32_t key_size, uint32_t version);

//创建一个LOGIN SERVER的授权签名
string create_login_cookie(RoleID user_id, uint32_t cookie_delay = 10800000);
//检查用户的Login server授权信息是合法
int32_t check_login_cookie(uint32_t version, const string& cookie_data, RoleID user_id);

#endif

