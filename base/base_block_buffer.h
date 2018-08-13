/*************************************************************************************
*filename:	base_block_buffer.h
*
*读循环队列的实现
*Create on: 2016-03-25
*Author:	luochj
*check list:
*************************************************************************************/
#ifndef __BASE_BLOCK_BUFFER_H
#define __BASE_BLOCK_BUFFER_H

#include "base_namespace.h"
#include "base_typedef.h"
#include "base_core_packet.h"
#include <string>
#include <ostream>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

BASE_NAMESPACE_BEGIN_DECL

//最大缓冲大小:30M
#define MAX_BUFFER_SIZE 31457280

template<uint32_t CAPACITY>
class CBlockBuffer_T
{
public:
	CBlockBuffer_T()
	{
		read_pos_ = 0;
		write_pos_ = 0;
		buffer_ = (uint8_t *)malloc(CAPACITY * sizeof(uint8_t));
		buffer_size_ = CAPACITY;
	};

	virtual ~CBlockBuffer_T()
	{
		read_pos_ = 0;
		write_pos_ = 0;

		free(buffer_);
		buffer_size_  = 0;
	};

	bool realloc_buffer(uint32_t size) //分配一个2倍的BUFFER
	{
		//剩余空间够用
		if(size < remaining_length() || buffer_size_ > MAX_BUFFER_SIZE)
			return false;

		uint32_t buffer_size = buffer_size_ * 2;
		while(buffer_size -  data_length() < size) //计算新的剩余空间是否够用
		{
			buffer_size = buffer_size * 2;
		}

		buffer_ = (uint8_t *)realloc(buffer_, buffer_size);

		buffer_size_ = buffer_size;

		return true;
	}

	void reset()
	{
		if(buffer_size_ > CAPACITY)
		{
			free(buffer_);

			buffer_ = (uint8_t *)malloc(CAPACITY * sizeof(uint8_t));
			buffer_size_ = CAPACITY;
		}
		
		memset(buffer_, 0x00, buffer_size_);

		read_pos_ = 0;
		write_pos_ = 0;
	}

	//获取读位置
	uint8_t* get_rptr()
	{
		return buffer_ + read_pos_;
	};
	//移动读位置
	void	 move_rptr(uint32_t n)
	{
		read_pos_ += n;
	};
	//获取写位置
	uint8_t* get_wptr()
	{
		return buffer_ + write_pos_;
	};
	//移动写位置
	void	 move_wptr(uint32_t n)
	{
		write_pos_ += n;
	};

	//BUFFER缓冲区大小
	uint32_t length() const
	{
		return buffer_size_;
	};
	//数据长度
	uint32_t data_length()
	{
		if(write_pos_ > read_pos_)
			return write_pos_ - read_pos_;
		else
		{
			recycle();
			return 0;
		}
	};
	//剩余缓冲区长度
	uint32_t remaining_length() const
	{
		return buffer_size_ - write_pos_;
	};

	bool	is_full() const
	{
		return (remaining_length() == 0);
	};

	void	 recycle()
	{
		if(write_pos_ <= read_pos_)
		{
			write_pos_ = 0;
			read_pos_ = 0;
		}
		else if(read_pos_ > 0)
		{
			memmove(buffer_, buffer_ + read_pos_, write_pos_ - read_pos_);
			write_pos_ = write_pos_ - read_pos_;
			read_pos_ = 0;
		}	
	};

protected:
	uint8_t*	buffer_;
	uint32_t	buffer_size_;

	uint32_t	read_pos_;
	uint32_t	write_pos_;
};

//连接接受队列(用于libevent的bufferevent接收)
/*
 * T为BinStream
 * CAPACITY 为定义的大小
 */
template<class T,  uint32_t CAPACITY>
class CReciverBuffer_T : public CBlockBuffer_T<CAPACITY>
{
public:
	CReciverBuffer_T()
	{

	};

	virtual ~CReciverBuffer_T()
	{

	};

	//从接受对象中收取报文
	int32_t recv(struct bufferevent *bev)
	{
		if(!bev)
		{
			return 0;
		}
		if(this->remaining_length() <= 0) //无空间，尝试开辟更大的空间
		{
			this->realloc_buffer(this->buffer_size_ / 8);
		}

		int32_t read_size = (int32_t)bufferevent_read(bev,this->get_wptr(), this->remaining_length());
		if(read_size > 0)
		{
			this->move_wptr(read_size);
			this->recycle();
		}

		return read_size;
	};

	//判断一个报文是否收取完毕
	int32_t split(T& packet)
	{
		if(this->data_length() < sizeof(PackProtocol))
			return 1;

		PackProtocol packPro;
		uint32_t packet_size = 0;
		uint32_t head_size = 0;
		
		//解析包PackProtocol获得包头的大小，包的大小
		packet.rewind(true);
		packet.resize(sizeof(PackProtocol));
		memcpy(packet.get_wptr(), this->get_rptr(), sizeof(PackProtocol));
		packet.set_used_size(sizeof(PackProtocol));
		try{
			packPro.UnPack(packet);
			packet_size = packPro.datalen;
			head_size = packPro.headlen;
		}
		catch (...){
			return -1;
		}

		if(packet_size > MAX_BUFFER_SIZE / 2)
			return -1;

		//收取报文体
		if(this->data_length() >= packet_size + head_size)
		{
			packet.rewind(true);
			packet.resize(packet_size);
			memcpy(packet.get_wptr(), this->get_rptr() + head_size, packet_size);
			packet.set_used_size(packet_size);

			this->move_rptr(packet_size + head_size);

			this->recycle();

			return 0;
		}
		else if(this->data_length() < packet_size  + head_size && this->remaining_length() <= 0) //缓冲区太小了
		{
			uint32_t alloc_size = packet_size + head_size - this->data_length();
			if(alloc_size > MAX_BUFFER_SIZE)
				alloc_size = MAX_BUFFER_SIZE;

			this->realloc_buffer(alloc_size);
		}

		return 1;
	};
};

BASE_NAMESPACE_END_DECL
#endif

/*************************************************************************************/


