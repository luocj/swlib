/*
* base_nodes_load.h
*	节点负载的管理，节点的选择
*  Created on: 2016-7-15
*      Author: luochj
*/
#ifndef __BASE_NODES_LOAD_H
#define __BASE_NODES_LOAD_H


#define _CRT_RAND_S 
#include <stdlib.h>

#include "base_namespace.h"
#include "base_typedef.h"
#include "base_object_pool.h"
#include "../vendor/lua/lua_tinker.h"

#include <map>
#include <vector>

using namespace std;
using namespace lua_tinker;

BASE_NAMESPACE_BEGIN_DECL

const int32_t MAX_LOAD_VALUE = 95;

typedef vector<uint32_t> SERVER_ID_ARRAY;

typedef struct NodeLoadInfo
{
	uint32_t	node_id;		//节点ID
	uint16_t	node_load;		//节点负载值，0到100,100表示负载最大

	NodeLoadInfo()
	{
		node_id = 0;
		node_load = 100;
	};
	void InitLoadInfo(table& info)
	{
		node_id = info.get<int>("id");
		node_load = info.get<int>("load");;
	};
}NodeLoadInfo;



typedef map<uint32_t, NodeLoadInfo>	NodeLoadInfoMap;

typedef struct NodeRange
{
	int32_t		min_value;
	int32_t		max_value;
	uint32_t	node_id;

	NodeRange()
	{
		min_value = 0;
		max_value = 0;
		node_id = 0;
	};
}NodeRange;

typedef vector<NodeRange>	NodeRangeArray;

class CNodeLoadManager
{
public:
	CNodeLoadManager();
	~CNodeLoadManager();

	void			add_node(const NodeLoadInfo& info);
	void			update_node(const NodeLoadInfo& info);
	void			LUpdateNode(table info);
	void			del_node(uint32_t node_id);

	uint32_t		select_node();
	uint32_t		LSelectNode();
	bool			select_node(NodeLoadInfo& info);
	uint32_t		size() const {return node_info_map_.size();};
private:
	uint32_t		locate_server(int32_t region);

private:
	NodeLoadInfoMap	node_info_map_;		//节点负载表
	NodeRangeArray	node_ranges_;		//一定周期的概率区间表
	
	bool			create_range_;		//是否需要重建概率选取表
	int32_t			region_;			//概率全区间
};

typedef  ObjectPool<CNodeLoadManager, 8> LodeMgrPool;
extern LodeMgrPool	m_LodeMgrPool;

BASE_NAMESPACE_END_DECL
#endif

/************************************************************************************/
