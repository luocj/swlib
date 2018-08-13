/****************************************************************************************
* Copyright (c) swsoft. All rights reserved.
*
* filename: swdef.h
* summary:  常用宏、类型等的定义
* version:  1.0.0
* author:   Wizard.Young
* modified: 
*****************************************************************************************/
#pragma once
#define _SWDEF_H_

#if defined (SWBUILD_EXPORTS)
#define DLL _declspec(dllexport)
#else
#define DLL
#endif

#if !defined _CRT_RAND_S
#define _CRT_RAND_S
#endif
#if !defined _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#if !defined _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#if !defined _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#endif

// 引用库
// crt
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// std
#include <string>
#include <xstring>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>

// 文字量、字符串相关宏
#define MAKESTR(x)     #x
#define JOIN(a, b)     a##b

// 常用类型
namespace sw
{
    typedef unsigned short      ushort;
    typedef unsigned int        uint;
    typedef unsigned long       ulong;
    typedef char                int8;
    typedef short               int16;
    typedef long                int32;
    typedef __int64             int64;
    typedef unsigned char       uint8;
    typedef unsigned short      uint16;
    typedef unsigned long       uint32;
    typedef unsigned __int64    uint64;
    typedef unsigned long       ty_size;
}

// 常用宏
#define DEL(p)                  { delete (p); (p) = NULL; }
#define DEL_ARR(p)              { delete[] (p); (p) = NULL; }
#define RELEASE(p)              if (p) { (p)->Release(); (p) = NULL; }
#define RETAIN(p)               if (p) { (p)->Retain(); }
#define SET_BIT(word, flag)     ((word) |= (flag))
#define RESET_BIT(word, flag)   ((word) &= ~(flag))
#define IS_SET(word, flag)      (((word) & (flag)) == (flag))
#define NOT_SET(word, flag)     (((word) & (flag)) == 0)
#define CALL(obj, func)         ((obj)->*(func))
#define CALL_C(obj, func)       if (obj) ((obj)->*(func))
#define CALL_CPP(obj, func)     if (obj) obj->func
#define CALL_FUNC(func)         if (func) func
#define SWAP(a, b)              if (&(a) != &(b)) { (a) ^= (b); (b) ^= (a); (a) ^= (b); }
#define DECLARE_SINGLETON(T)    public: static T &Singleton()
#define DEFINE_SINGLETON(T)     T &T::Singleton() { static T s; return s; }
#define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#define INLINE                  inline
#define PROPERTY(TYPE, NAME, METHOD)                private: TYPE NAME;\
                                                    public: void Set##METHOD(TYPE var) { NAME = var; }\
                                                    public: TYPE Get##METHOD() const { return NAME; }
#define PROPERTY_READONLY(TYPE, NAME, METHOD)       private: TYPE NAME;\
                                                    public: TYPE Get##METHOD() const { return NAME; }
#define PROPERTY_REF(TYPE, NAME, METHOD)            private: TYPE NAME;\
                                                    public: void Set##METHOD(const TYPE &var) { NAME = var; }\
                                                    public: TYPE &Get##METHOD() { return NAME; }
#define PROPERTY_READONLY_REF(TYPE, NAME, METHOD)   private: TYPE NAME;\
                                                    public: TYPE &Get##METHOD() { return NAME; }
#define PROPERTY_RETAIN(TYPE, NAME, METHOD)         private: TYPE NAME;\
                                                    public: void Set##METHOD(TYPE var) { RETAIN(var); RELEASE(NAME); NAME = var; }\
                                                    public: TYPE Get##METHOD() { return NAME; }

#define BEGIN_SW namespace sw {
#define END_SW }
#define USING_SW using namespace sw

// 调试用内存分配
#if defined(_WIN32) && defined(_DEBUG)
#define NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif


