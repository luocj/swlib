#include "system_cookie.h"
#include "base_bin_stream.h"
#include "base_date_time.h"
#include "base_log_macro.h"
#include "base64.h"


using namespace BASE_NAMEPSACE_DECL;

#define DEVIATION  60*5  //时间误差5分钟
const string WATERMARK = "I'm here";

//生成KEY的对照表
const uint8_t const_key_table[256] = {	0xcd, 0x23, 0xee, 0x9a, 0x21, 0xdb, 0xee, 0x62, 0xab, 0xcf, 0x95, 0x1f, 0x9e, 0xbc, 0xbf, 0x0c,
										0xd4, 0x60, 0x44, 0x71, 0x08, 0xdb, 0xcd, 0x25, 0x45, 0x5b, 0xe6, 0x51, 0x9e, 0xf8, 0x54, 0x76,
										0x20, 0xdd, 0xa8, 0xb2, 0x05, 0xa9, 0x14, 0x3f, 0x2a, 0x15, 0x7e, 0x3c, 0xc4, 0xd6, 0xd3, 0xb0,
										0x4e, 0xc5, 0x9e, 0xcc, 0xc6, 0x3d, 0xd9, 0x70, 0x18, 0xc4, 0x02, 0xf1, 0xdb, 0xee, 0x72, 0x1a,
										0x3b, 0x7f, 0xeb, 0x70, 0x36, 0xd0, 0x70, 0xba, 0x0c, 0x74, 0x59, 0xc1, 0xf0, 0x1a, 0xa6, 0x55,
										0x04, 0xb5, 0x94, 0x90, 0x84, 0xdb, 0x6e, 0x5f, 0x41, 0x6b, 0xa6, 0x3d, 0x52, 0x73, 0x24, 0x42,
										0x06, 0x51, 0xde, 0x5d, 0x1d, 0x18, 0xa9, 0xde, 0x37, 0x35, 0x4e, 0x37, 0x8c, 0x52, 0xe1, 0x03,
										0xde, 0x7a, 0x4e, 0x47, 0xac, 0x7e, 0x35, 0xf8, 0xa9, 0x98, 0xf8, 0xbe, 0x6c, 0x4f, 0x12, 0xf8,
										0x69, 0x9a, 0xaa, 0x00, 0x20, 0x48, 0x67, 0xb0, 0x94, 0x9f, 0x87, 0x25, 0xff, 0x44, 0x2b, 0xc2,
										0xc4, 0x5b, 0xf5, 0x78, 0xa5, 0xee, 0xd5, 0x45, 0x36, 0x93, 0x20, 0xfb, 0x91, 0x49, 0xe3, 0x43,
										0x4c, 0xa4, 0x76, 0xe1, 0xa8, 0x2a, 0x54, 0x3a, 0x0b, 0xfc, 0x29, 0x13, 0xb1, 0xb9, 0x2d, 0x9b,
										0x9e, 0x9f, 0xb1, 0xac, 0xd7, 0xf3, 0xf7, 0x4e, 0xd1, 0xa3, 0x47, 0x7d, 0x2a, 0x2b, 0x3f, 0x2c,
										0x97, 0xb5, 0x6b, 0x89, 0x1e, 0x84, 0x16, 0x83, 0x85, 0x92, 0x5f, 0x89, 0x0a, 0x78, 0x8f, 0x95,
										0x54, 0x8f, 0xa9, 0x6a, 0xaa, 0x55, 0x43, 0x1a, 0x63, 0x12, 0x95, 0xca, 0x9d, 0xbb, 0xd0, 0xb9,
										0x32, 0x16, 0xb1, 0x7f, 0xe8, 0x1f, 0x55, 0x93, 0xe8, 0xab, 0x4f, 0x10, 0x72, 0x4b, 0xf8, 0xb9,
										0xce, 0x73, 0x06, 0x3a, 0x86, 0xdb, 0x60, 0xb1, 0xd2, 0x26, 0x31, 0x6c, 0x54, 0xc2, 0x3b, 0xf4 };


//此版本号是为了做KEY版本兼容
const uint32_t encrypt_version = 1;

uint32_t get_encrypt_version()
{
	return encrypt_version;
}

void generate_key(uint32_t param, uint8_t* key, int32_t key_size, uint32_t version)
{
	string param_data;
	param_data.resize(sizeof(param));

	memcpy((void *)(param_data.data()), &param, sizeof(param));

	generate_key(param_data, key, key_size, version);
}

void generate_key(const string& param, uint8_t* key, int32_t key_size, uint32_t version)
{
	uint8_t digest[16] = { 0 };
	MD5 md5_ctx(param.data(), param.size());
	memcpy(digest,md5_ctx.digest(),16);

	for (int32_t i = 0; i < key_size; i++)
	{
		uint8_t index = digest[i % 16];
		switch (version)
		{
		case 1:
			key[i] = const_key_table[index];
			break;
		}
	}
}

typedef struct tagLoginCookie
{
	tagLoginCookie()
	{
		signature = "";
	}

	uint32_t	version;
	RoleID		user_id;
	uint64_t	begin_timestamp;
	uint64_t	end_timestamp;
	string		signature;
}LoginCookieCTX;


string create_login_cookie(RoleID user_id, uint32_t cookie_delay /* = 10800000 */)
{
	uint32_t version = get_encrypt_version();
	//产生一个AES KEY
	uint8_t aes_key[AESKEYLEN_256] = {0};
	generate_key(user_id, aes_key, AESKEYLEN_256, version);

	aes_context aes_ctx;
	aes_set_key(&aes_ctx, aes_key, 256);

	LoginCookieCTX cookie;
	cookie.version = version;
	cookie.user_id = user_id;
	cookie.begin_timestamp = DateTime::GetNow().GetSeconds() - DEVIATION;
	cookie.end_timestamp = cookie.begin_timestamp + cookie_delay;

	//md5水印
	BinStream strm;
	strm << cookie.version << cookie.user_id << cookie.begin_timestamp << WATERMARK << cookie.end_timestamp;
	MD5 sigMd5(strm.get_data_ptr(), strm.data_size());
	cookie.signature = sigMd5.toString();

	strm << cookie.signature;

	//进行加密
	uint8_t input_data[AES_SIZE] = {0};
	int32_t strm_size = strm.data_size();
	uint8_t* pos = (uint8_t *)strm.get_rptr();

	uint8_t* encode_data = NULL;
	uint32_t encode_data_size = 0;
	//长度为16的整数倍
	if(strm_size % AES_SIZE > 0)
	{
		encode_data_size = AES_SIZE * (strm_size / AES_SIZE + 1);
	}
	else
	{
		encode_data_size = strm_size;
	}

	encode_data = new uint8_t[encode_data_size];

	uint8_t* dst = encode_data;
	//分为16字节加密一次
	while(strm_size > 0)
	{
		memset(input_data, 0x00, AES_SIZE);
		int32_t aes_size = (strm_size > AES_SIZE ? AES_SIZE : strm_size);

		memcpy(input_data, pos, aes_size);
		aes_encrypt(&aes_ctx, input_data, dst);

		strm_size -= aes_size;
		pos += aes_size;
		dst += AES_SIZE;
	}

	string ret;
	ret.assign((char *)encode_data, encode_data_size);

	delete []encode_data;
	return Base64::encode(ret);
}

int32_t check_login_cookie(uint32_t version, const string& cookie_data, RoleID user_id)
{
	string strCookie = Base64::decode(cookie_data);
	//产生一个AES KEY
	uint8_t aes_key[AESKEYLEN_256] = {0};
	generate_key(user_id, aes_key, AESKEYLEN_256, version);

	aes_context aes_ctx;
	aes_set_key(&aes_ctx, aes_key, 256);

	uint8_t* pos = (uint8_t *)strCookie.data();
	int32_t cookie_size = strCookie.size();

	//检查数据长度是否是合法的
	if(cookie_size % AES_SIZE > 0)
		return eUnknownError;

	//数据解密
	BinStream strm;
	strm.resize(cookie_size);
	strm.set_used_size(cookie_size);

	uint8_t* dst = strm.get_wptr();
	uint8_t input_data[AES_SIZE] = {0};
	while(cookie_size > 0)
	{
		memcpy(input_data, pos, AES_SIZE);
		aes_decrypt(&aes_ctx, input_data, dst);

		pos += AES_SIZE;
		dst += AES_SIZE;
		cookie_size -= AES_SIZE;
	}

	string strMark;
	//解封数据
	LoginCookieCTX cookie;
	try
	{
		strm >> cookie.version >> cookie.user_id >> cookie.begin_timestamp >> strMark >> cookie.end_timestamp  >> cookie.signature;
	}
	catch (...)
	{
		return eCerfityDataError;
	}
	
	if(cookie.version != version)
		return eCerfityVersionError;

	//合法性判断
	if(cookie.user_id != user_id)
		return eUserIDError;

	//时间超时
	uint64_t cur_timestamp = DateTime::GetNow().GetSeconds();
	if (cur_timestamp < cookie.begin_timestamp || cur_timestamp > cookie.end_timestamp)
	{
		CORE_DEBUG("begin time:"<< cookie.begin_timestamp << " end time:" << cookie.end_timestamp << " now : "<<cur_timestamp);
		return eCerfityTimeOut;
	}
		
	BinStream strmMd5;
	strmMd5 << cookie.version << cookie.user_id << cookie.begin_timestamp << strMark << cookie.end_timestamp;
	MD5 sigMd5(strmMd5.get_data_ptr(), strmMd5.data_size());
	string sigStr = sigMd5.toString();
	//判断摘要水印

	if(memcmp(sigStr.data(), cookie.signature.data(), 16) != 0)
		return eSignatureError;

	return eCerfitySucc;
}


