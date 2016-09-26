#pragma once

 #define PATH_SIZE 4096
 #include <unistd.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>



#include <stdint.h>	// for non-c++0x
#include <vector>
#include <string>
#include <map>
#include "xia.h"

class Graph;

class Node
{
public:
	static const std::size_t ID_LEN = 20;

	static const unsigned int XID_TYPE_UNKNOWN = 0;
	static const unsigned int XID_TYPE_DUMMY_SOURCE = 0xff;
	static const unsigned int XID_TYPE_AD = 0x10;  // TODO: why does swig complain when these are uint32_t?
	static const unsigned int XID_TYPE_HID = 0x11;
	static const unsigned int XID_TYPE_CID = 0x12;
	static const unsigned int XID_TYPE_SID = 0x13;
	static const unsigned int XID_TYPE_IP = 0x14;

	static const std::string XID_TYPE_UNKNOWN_STRING;
	static const std::string XID_TYPE_DUMMY_SOURCE_STRING;
	static const std::string XID_TYPE_AD_STRING;
	static const std::string XID_TYPE_HID_STRING;
	static const std::string XID_TYPE_CID_STRING;
	static const std::string XID_TYPE_SID_STRING;
	static const std::string XID_TYPE_IP_STRING;

public:
	Node();
	Node(const Node& r);
	Node(uint32_t type, const void* id, int dummy); // NOTE: dummy is so compiler doesn't complain about ambigous constructors  TODO: fix.
	Node(int type, const std::string id_string); // NOTE: type is an int here because the consts above are ints, otherwise swig will complain again
	Node(const std::string type_string, const std::string id_string);
	Node(const std::string node_string);

	~Node();

	const uint32_t& type() const { return ptr_->type; }
	const unsigned char* id() const { return ptr_->id; }
	std::string type_string() const;
	std::string id_string() const;
	std::string to_string() const;

	Node& operator=(const Node& r);
	bool operator==(const Node& r) const { return ptr_ == r.ptr_; }
	bool operator!=(const Node& r) const { return ptr_ != r.ptr_; }
	Graph operator*(const Node& r) const;
	Graph operator*(const Graph& r) const;
	Graph operator+(const Node& r) const;
	Graph operator+(const Graph& r) const;

	bool equal_to(const Node& r) const;

private:
	typedef std::map<int, std::string> XidMap;
	static XidMap xids;
	static XidMap load_xids();

	

	void acquire() const;
	void release() const;

	struct container
	{
		uint32_t type;
		unsigned char id[ID_LEN];
		std::size_t ref_count;
	};

	mutable container* ptr_;
	static container dummy_source_;

	void construct_from_strings(const std::string type_string, const std::string id_string);
};

class Graph
{
public:
	Graph();
	Graph(const Node& n);
	Graph(const Graph& r);
	Graph(std::string dag_string);
	Graph(const sockaddr_x *s);

	Graph& operator=(const Graph& r);
	Graph& operator*=(const Graph& r);
	Graph operator*(const Graph& r) const;
	Graph& operator+=(const Graph& r);
	Graph operator+(const Graph& r) const;
	Graph operator*(const Node& r) const;
	Graph operator+(const Node& r) const;

	void print_graph() const;
	std::string dag_string() const;
	std::string intent_AD_str() const;
	std::string intent_HID_str() const;
	bool is_final_intent(const Node& n);
	bool is_final_intent(const std::string xid_string);
	Graph next_hop(const Node& n);
	Graph next_hop(const std::string xid_string);
	Graph first_hop();
	uint8_t num_nodes() const;
	Node get_node(int i) const;
	std::vector<std::size_t> get_out_edges(int i) const;
	void fill_sockaddr(sockaddr_x *s) const;
	void from_sockaddr(const sockaddr_x *s);
	void replace_final_intent(const Node& new_intent);
	Node get_final_intent() const;
private:
	std::size_t add_node(const Node& p, bool allow_duplicate_nodes = false);
	void add_edge(std::size_t from_id, std::size_t to_id);

	void replace_node_at(int i, const Node& new_node);
	std::vector<const Node*> get_nodes_of_type(unsigned int type) const;


	bool is_source(std::size_t id) const;
	bool is_sink(std::size_t id) const;

	std::size_t source_index() const;
	std::size_t final_intent_index() const;

	void merge_graph(const Graph& r, std::vector<std::size_t>& node_mapping, bool allow_duplicate_nodes = false);

	std::string out_edges_for_index(std::size_t i, std::size_t source_index, std::size_t sink_index) const;
	std::size_t index_in_dag_string(std::size_t index, std::size_t source_index, std::size_t sink_index) const;
	std::size_t index_from_dag_string_index(int32_t dag_string_index, std::size_t source_index, std::size_t sink_index) const;

	void from_wire_format(uint8_t num_nodes, const node_t *buf);
	void construct_from_dag_string(std::string dag_string);
	int check_dag_string(std::string dag_string);
	void construct_from_re_string(std::string re_string);
	int check_re_string(std::string re_string);

	std::vector<Node> nodes_;
	std::vector<std::vector<std::size_t> > out_edges_;
	std::vector<std::vector<std::size_t> > in_edges_;
};

/**
* @brief Make a graph by appending a node
*
* Make a graph by appending a node to this node. The resulting graph will have
* one edge from this node to the supplied node.
*
* @param r The node to append
*
* @return The resulting graph
*/
inline Graph Node::operator*(const Node& r) const
{
	return Graph(*this) * Graph(r);
}

/**
* @brief Append a node
*
* Append a node to the end of this graph and return the result.
*
* @param r The node to append
*
* @return The resulting graph
*/
inline Graph Graph::operator*(const Node& r) const
{
	return Graph(*this) * Graph(r);
}

/**
* @brief Make a graph by appending a graph
*
* Make a graph by appending an existing graph to this node. The resulting
* graph will have an edge from this node to the source node of the supplied
* one. The edges of the supplied graph will not be affected.
*
* @param r The graph to append
*
* @return The resulting graph
*/
inline Graph Node::operator*(const Graph& r) const
{
	return Graph(*this) * r;
}

/**
* @brief Make a graph by merging with a node
*
* Make a graph by merging this node with another node. If the nodes are equal,
* the resulting graph will contain one node and no edges. Otherwise, the
* resulting graph will contain two nodes and no edges. Each node will be both
* a source and a sink.
*
* @param r The node with which to merge
*
* @return The resulting graph
*/
inline Graph Node::operator+(const Node& r) const
{
	return Graph(*this) + Graph(r);
}

/**
* @brief Merge with a node
*
* Merge the supplied node with this graph. If the node is already present in
* this graph, the resulting graph will be the same. If the node is different,
* the new node will be added to the graph but will not be connected by any
* edges.
*
* @param r The node with which to merge
*
* @return The resulting graph
*/
inline Graph Graph::operator+(const Node& r) const
{
	return Graph(*this) + Graph(r);
}

/**
* @brief Make a graph by merging with a graph
*
* Make a graph by merging this node with the supplied graph. If the node is
* already present in this graph, the resulting graph will be the same as the
* supplied graph. If the node is different, this node will be added to the
* graph but will not be connected by any edges.
*
* @param r The graph with which to merge
*
* @return The resulting graph
*/
inline Graph Node::operator+(const Graph& r) const
{
	return Graph(*this) + r;
}
