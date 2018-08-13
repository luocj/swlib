/*
 * base64.h
 *
 *  Created on: 2016-7-15
 *      Author: luochj
 */

#ifndef CORE_FRAME_H_
#define CORE_FRAME_H_

#include "base_namespace.h"
#include "base_os.h"


BASE_NAMESPACE_BEGIN_DECL

class Base64
{
public:
	static std::string encode(const std::string & data);
	static std::string decode(const std::string & data);
	static std::string encodeFromArray(const char * data, size_t len);
private:
	static const std::string Base64Table;
	static const std::string::size_type DecodeTable[];
};

BASE_NAMESPACE_END_DECL

#endif /* CORE_FRAME_H_ */
