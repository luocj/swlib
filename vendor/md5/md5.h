#pragma once
#include <string>
#include <string.h>
#include <fstream>


typedef unsigned char byte;

using std::string;
using std::ifstream;


class MD5 {
public:
	MD5();
    /*
     @desc          输入内存地址与长度的构造信息
     */
	MD5(const void *input, size_t length);
	MD5(const string &str);
    /*
     @param in      文件输入流
     输入流的构造函数
     */
	MD5(ifstream &in);
    
	void update(const void *input, size_t length);
	
    void update(const string &str);
	
    void update(ifstream &in);
	
    const byte* digest();
    /*
     @desc将md5转换成string返回
     */
	string toString();
	void reset();
private:
	void update(const byte *input, size_t length);
	void final();
	void transform(const byte block[64]);
	void encode(const unsigned int *input, byte *output, size_t length);
	void decode(const byte *input, unsigned int *output, size_t length);
	string bytesToHexString(const byte *input, size_t length);
    

	MD5(const MD5&);
	MD5& operator=(const MD5&);
private:
	unsigned int _state[4];	
	unsigned int _count[2];	
	byte _buffer[64];	
	byte _digest[16];	
	bool _finished;		
    
	static const byte PADDING[64];	
	static const char HEX[16];
	static const size_t BUFFER_SIZE = 1024;
};
