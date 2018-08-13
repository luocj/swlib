
#include "json_lua.h"

bool CJsonLuaTable::JsonToTable(const char *jchar, table& ret)
{
	if (!jchar)
	{
		return false;
	}
	string jstr = jchar;
	json_tokener *tok = json_tokener_new();
	json_object *jo = NULL;

	// 解析json对象
	jo = json_tokener_parse_ex(tok, jstr.c_str(), jstr.length());
	if (tok->err != json_tokener_success)
	{
		json_tokener_free(tok);
		return false;
	}

	bool retBool = PushJsonToTable(jo, ret);
	json_object_put(jo);
	json_tokener_free(tok);
	return retBool;
}

// 将json对象解为table，压入lua堆栈，递归使用
bool CJsonLuaTable::PushJsonToTable(json_object *jo, table& ret)
{
	if (ret.m_obj == NULL)
	{
		return false;
	}
	lua_State *L = ret.m_obj->m_L;
	if (!L || !jo)
	{
		return false;
	}
#define PUSH_VALUE(k,v, t) if (v == NULL) { } \
                            else { \
                            t = json_object_get_type(v); \
                            switch (t) \
                            { \
                            case json_type_int:  ret.set(k,json_object_get_int64(v));     break; \
                            case json_type_double:ret.set(k, json_object_get_double(v));   break; \
                            case json_type_string: ret.set(k, json_object_get_string(v));   break; \
                            case json_type_boolean: ret.set(k, !!json_object_get_boolean(v)); break; \
                            case json_type_array: \
                            case json_type_object: \
							{\
								table retTable(L); \
								PushJsonToTable(v, retTable);\
								ret.set(k,retTable);\
								break; \
							}\
                            default:   \
								break; \
                            } }

	json_type jt = json_object_get_type(jo);
	int count = 0;

	// json数组
	if (jt == json_type_array)
	{
		// 数组长度
		count = json_object_array_length(jo);
		if (count == 0)
		{
			return false;
		}
		// 遍历数组，压入到lua
		json_object *value = NULL;
		for (int i = 0; i < count; ++i)
		{
			value = json_object_array_get_idx(jo, i);
			int key = i + 1;
			PUSH_VALUE(key,value, jt);
		}
	}
	// json对象
	else if (jt == json_type_object)
	{
		// 对象元素个数
		count = json_object_get_object(jo)->count;
		if (count == 0)
		{
			return false;
		}
		// 遍历对象属性，压入到lua
		json_object_object_foreach(jo, key, value)
		{
			PUSH_VALUE(key,value, jt);
		}
	}
	else
	{
		return false;
	}

	return true;
#undef PUSH_VALUE
}

