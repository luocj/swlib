/*
 * json_lua.h
 *
 *  Created on: 2016-3-8
 *      Author: luochj
 */

#ifndef JSON_LUA_H_
#define JSON_LUA_H_

#include "../vendor/lua/lua_tinker.h"
#include "../vendor/jsonc/json.h"
#include <string>

using namespace lua_tinker;
using namespace std;

class CJsonLuaTable
{
public:
	CJsonLuaTable(){};
	~CJsonLuaTable(){};

	/*
	将一个json字符串解码为table
	@param[in] jstr:   json字符串
	@param[in] ret:	   转换得到的table
	*/
	static bool JsonToTable(const char *jchar, table& ret);
	/*
	将json对象解为table
	@param[in] jo: json对象
	@param[in] ret:	   转换得到的table
	*/
	static bool PushJsonToTable(json_object *jo, table& ret);
};


#endif /* JSON_LUA_H_ */
