#pragma once
// A simple data structure with movable nodes "allocated" in an internal array,
// with explicit deallocation and ability to "shuffle" the nodes, preserving
// the links between them using movable_ptr

#include <memory>
#include <array>
#include "movable_ptr.hpp"

class MovableNode : public enable_movable_ptr<MovableNode>
{
public:
	static const int MAX_REFS = 4;
	using RefArray = std::array<movable_ptr<MovableNode>, MAX_REFS>;

	MovableNode() : _isValid(false), _value(0), _refs() {}
	MovableNode(int value) : _isValid(true), _value(value), _refs() {}

	bool isValid() const { return _isValid; }

	int value() const { checkValid(); return _value; }

	RefArray& refs() { checkValid(); return _refs; }
private:
	bool _isValid;
	int _value;
	RefArray _refs;

	void checkValid() const;
};

class CompactableGraph
{
public:
	CompactableGraph(size_t capacity);

	movable_ptr<MovableNode> allocNode(int value);
	void removeNode(movable_ptr<MovableNode>& node);
	void compact();

private:
	size_t _capacity, _nextAlloc;
	std::unique_ptr<MovableNode[]> _heap;
};
