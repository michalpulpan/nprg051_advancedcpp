#include <cassert>
#include <exception>
#include <algorithm>
#include "CompactableGraph.hpp"

using namespace std;

void MovableNode::checkValid() const {
	if (!_isValid) {
		throw std::runtime_error("Accessing data of a deleted or not yet initialized node");
	}
}

CompactableGraph::CompactableGraph(size_t capacity) :
	_capacity(capacity),
	_nextAlloc(0),
	_heap(make_unique<MovableNode[]>(capacity))
{
}

movable_ptr<MovableNode> CompactableGraph::allocNode(int value)
{
	if (_nextAlloc == _capacity) {
		compact();
		if (_nextAlloc == _capacity) {
			throw runtime_error("Insufficient capacity");
		}
	}

	MovableNode* newNode = &_heap[_nextAlloc++];
	assert(!newNode->isValid());

	*newNode = MovableNode(value);
	return movable_ptr<MovableNode>(newNode);
}

void CompactableGraph::removeNode(movable_ptr<MovableNode>& node)
{
	if (!node) {
		return;
	}

	// The node must be deleted from _heap
	assert(node.get() >= _heap.get() && node.get() < _heap.get() + _capacity);

	// By assigning an invalid node we:
	// 1) Call the implicit destructor
	// 2) Set _isValid to false
	*node = MovableNode();
}

void CompactableGraph::compact()
{
	auto validNodeCallback = [this](const MovableNode& node) { return node.isValid(); };
	auto invalidNodeCallback = [this](const MovableNode& node) { return !node.isValid(); };
	MovableNode* heapBegin = _heap.get();
	MovableNode* heapEnd = heapBegin + _capacity;

	// Traverse simultaneously with two pointers and fill all the gaps in the heap by moving the nodes
	MovableNode* nextFree = find_if(heapBegin, heapEnd, invalidNodeCallback);
	MovableNode* nextToMove = find_if(nextFree, heapEnd, validNodeCallback);

	while (nextToMove != heapEnd) {
		assert(nextToMove > nextFree);

		*nextFree = move(*nextToMove);
		nextFree++;

		*nextToMove = MovableNode();	// Make sure to set _isValid to false
		nextToMove++;

		nextFree = find_if(nextFree, heapEnd, invalidNodeCallback);
		nextToMove = find_if(nextToMove, heapEnd, validNodeCallback);
	}

	_nextAlloc = nextFree - heapBegin;
	assert(nextFree - heapBegin >= 0);
	assert(_nextAlloc <= _capacity);
}
