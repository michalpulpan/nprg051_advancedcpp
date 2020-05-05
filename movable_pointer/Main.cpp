#include <iostream>
#include <memory>
#include <string>

#include "CompactableGraph.hpp"
#include "movable_ptr.hpp"
using namespace std;

// Simple movable class used in the first two test sets
class A : public enable_movable_ptr<A>
{
public:
	int val;

	A(int val) : val(val) {}
};

// Simple error reporting mechanism similar to classic assert, but visible in ReCodEx

void test_assert(bool cond, const char* assertion, const char* file, size_t line) {
	if (!cond) {
		cout
			<< "Assertion '" << assertion << "' failed" << endl
			<< "in file " << file << ", line " << line << endl;
		
		exit(0);	// Return value other than zero prevents ReCodEx from showing the log
	}
}

#define TEST_ASSERT(cond) test_assert(cond, #cond, __FILE__, __LINE__)

// Test #1 - basic semantics of all operations
//
// Expected output: OK

void test_ptr_construction() {
	

	movable_ptr<A> pempty;

	A x(42);
	movable_ptr<A> pcons(&x);

	auto pget = get_movable(x);
	movable_ptr<A> preset;
	preset.reset(&x);

	TEST_ASSERT(pempty.get() == nullptr);
	TEST_ASSERT(pcons.get() == &x);
	TEST_ASSERT(pget.get() == &x);
	TEST_ASSERT(preset.get() == &x);
}

void test_ptr_compare(){
	movable_ptr<A> pempty;

	TEST_ASSERT(pempty == nullptr);

	A x(42);
	auto px1 = get_movable(x);
	auto px2 = get_movable(x);

	TEST_ASSERT(px1 == &x);
	TEST_ASSERT(px2 == &x);
	TEST_ASSERT(px1 == px1);
	TEST_ASSERT(px1 == px2);
	TEST_ASSERT(px2 == px1);

	A y(6);
	auto py = get_movable(y);

	TEST_ASSERT(py == &y);
	TEST_ASSERT(py != &x);
	TEST_ASSERT(px1 != &y);
	TEST_ASSERT(px1 != py);
	TEST_ASSERT(py != px1);
}

void test_ptr_dereference() {
	A x(42);
	auto px = get_movable(x);

	TEST_ASSERT(&*px == &x);
	TEST_ASSERT(&px->val == &x.val);
}

void test_ptr_reset() {
	A x(42);
	auto px = get_movable(x);

	px.reset();
	TEST_ASSERT(px.get() == nullptr);
	TEST_ASSERT(!px);

	px.reset(&x);
	TEST_ASSERT(px.get() == &x);
	TEST_ASSERT((bool)px);
}

void test_ptr_copy() {
	A x(42);
	auto p1 = get_movable(x);
	auto p2 = p1;
	movable_ptr<A> p3;

	TEST_ASSERT(p1.get() == p2.get());
	
	p3 = p1;
	TEST_ASSERT(p3.get() == p1.get());
}

void test_ptr_move() {
	A x(42);
	auto p1 = get_movable(x);
	auto p2 = move(p1);
	movable_ptr<A> p3;

	TEST_ASSERT(p1.get() == nullptr);
	TEST_ASSERT(p2.get() == &x);

	p3 = move(p2);
	TEST_ASSERT(p2.get() == nullptr);
	TEST_ASSERT(p3.get() == &x);
}

void test_trg_destruction() {
	movable_ptr<A> p;

	{
		A scoped(42);
		p.reset(&scoped);

		TEST_ASSERT(p.get() == &scoped);
	}

	TEST_ASSERT(p.get() == nullptr);
}

void test_trg_copy() {
	A x(42);
	auto p = get_movable(x);
	A y(x);
	A z(666);

	TEST_ASSERT(p.get() == &x);
	//tady se to nejak sere...
	z = x;
	TEST_ASSERT(p.get() == &x);
}

void test_trg_move() {
	A x(42);
	auto p1 = get_movable(x);
	auto p2 = get_movable(x);

	A y = move(x);
	A z(666);

	TEST_ASSERT(p1.get() == &y);
	TEST_ASSERT(p2.get() == &y);

	z = move(y);
	TEST_ASSERT(p1.get() == &z);
	TEST_ASSERT(p2.get() == &z);
}

// Test #2 - more complex scenarios and corner cases
//
// Expected output: OK

void test_trg_assign() {
	A x(42);
	A y(666);
	auto px = get_movable(x);
	auto py = get_movable(y);
	x = y;

	TEST_ASSERT(px.get() == nullptr);
}

void test_ptr_self_assign() {
	A x(42);
	auto p = get_movable(x);
	auto pp = &p;

	*pp = p;
	TEST_ASSERT(p.get() == &x);

	*pp = move(p);
	TEST_ASSERT(p.get() == &x);
}

void test_trg_self_assign() {
	A x(42);
	auto p = get_movable(x);

	x = *p;
	TEST_ASSERT(p.get() == &x);

	x = move(*p);
	TEST_ASSERT(p.get() == &x);
}

void test_updates() {
	A x(42);
	A y(6);
	A z(666);

	auto p1 = get_movable(x);
	auto p2 = get_movable(x);
	auto p3 = get_movable(x);
	TEST_ASSERT(p1.get() == &x);
	TEST_ASSERT(p2.get() == &x);
	TEST_ASSERT(p3.get() == &x);

	y = move(x);
	TEST_ASSERT(p1.get() == &y);
	TEST_ASSERT(p2.get() == &y);
	TEST_ASSERT(p3.get() == &y);
	TEST_ASSERT(p1->val == 42);
	
	auto p4 = move(p3);
	TEST_ASSERT(p1.get() == &y);
	TEST_ASSERT(p2.get() == &y);
	TEST_ASSERT(p3.get() == nullptr);
	TEST_ASSERT(p4.get() == &y);

	p4.reset();
	z = move(y);
	TEST_ASSERT(p1.get() == &z);
	TEST_ASSERT(p2.get() == &z);
	TEST_ASSERT(p3.get() == nullptr);
	TEST_ASSERT(p4.get() == nullptr);

	z = A(1);
	TEST_ASSERT(p1.get() == nullptr);
	TEST_ASSERT(p2.get() == nullptr);
	TEST_ASSERT(p3.get() == nullptr);
	TEST_ASSERT(p4.get() == nullptr);
}

// Test #3 - simple usage in a data structure with movable elements
//
// Expected output: see out3.txt

void print_node_ref(const movable_ptr<MovableNode>& node) {
	if (node) {
		cout << node->value();
	} else {
		cout << '-';
	}
}

void print_node(const movable_ptr<MovableNode>& node) {
	if (node) {
		cout << '[' << node->value() << "] ";
		print_node_ref(node->refs()[0]);
		cout << ',';
		print_node_ref(node->refs()[1]);
		cout << ',';
		print_node_ref(node->refs()[2]);
		cout << ',';
		print_node_ref(node->refs()[3]);
		cout << endl;
	} else {
		cout << "[-]" << endl;
	}
}

void test_graph1() {
	CompactableGraph g(3);
	auto node1 = g.allocNode(1);
	auto node2 = g.allocNode(2);
	auto node3 = g.allocNode(3);

	node1->refs()[0] = node3;
	node1->refs()[1] = node2;
	node2->refs()[0] = node1;
	node2->refs()[1] = node3;
	node3->refs()[0] = node2;
	node3->refs()[1] = node1;

	print_node(node1);
	print_node(node2);
	print_node(node3);
	cout << endl;

	g.removeNode(node1); //remove node nefunguje 
	auto node4 = g.allocNode(4);

	print_node(node1);
	print_node(node2);
	print_node(node3);
	print_node(node4);
	cout << endl;

	node2->refs()[0] = node2;
	node4->refs()[0] = node4;
	g.removeNode(node2);
	g.compact();

	print_node(node1);
	print_node(node2);
	print_node(node3);
	print_node(node4);

	cout << endl << endl;
}

// Test #4 - complex usage in the data structure
//
// Expected output: see out4.txt

void test_graph2() {
	const size_t nodeCount = 50;
	CompactableGraph g(nodeCount);

	array<movable_ptr<MovableNode>, nodeCount> nodePtrs;
	for (size_t i = 0; i < nodeCount; i++)
	{
		nodePtrs[i] = g.allocNode((int)i);
	}

	for (size_t i = 0; i < nodeCount; i++)
	{
		nodePtrs[i]->refs()[0] = nodePtrs[(i + 1) % nodeCount];
		nodePtrs[i]->refs()[1] = nodePtrs[(i + (nodeCount / 6)) % nodeCount];
		nodePtrs[i]->refs()[2] = nodePtrs[(i + (nodeCount / 3)) % nodeCount];
		nodePtrs[i]->refs()[3] = nodePtrs[(i + (nodeCount / 2)) % nodeCount];
	}

	for (size_t i = 5; i < nodeCount; i += 7)
	{
		g.removeNode(nodePtrs[i]);
	}

	g.compact();

	for (size_t i = 0; i < nodeCount; i++)
	{
		print_node(nodePtrs[i]);
	}

	cout << endl;

	for (size_t i = 3; i < nodeCount; i += 4)
	{
		g.removeNode(nodePtrs[i]);

		auto ptr = nodePtrs[(i + 1) % nodeCount];
		if (ptr) {
			ptr->refs()[3] = nodePtrs[(i + 3) % nodeCount];
		}
	}

	g.compact();

	for (size_t i = 0; i < nodeCount; i++)
	{
		print_node(nodePtrs[i]);
	}

	cout << endl << endl;
}

// Dispatch the tests
int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Expects the number of the test to run" << endl;
		return 1;
	}

	int testNo = stoi(argv[1]);
	switch (testNo)
	{
	case 1:
		test_ptr_construction();
		test_ptr_compare();
		test_ptr_dereference();
		test_ptr_reset();
		test_ptr_copy();
		test_ptr_move();
		test_trg_destruction();
		test_trg_copy();
		test_trg_move();
		cout << "OK" << endl;
		return 0;
	case 2:
		test_trg_assign();
		test_ptr_self_assign();
		test_trg_self_assign();
		test_updates();
		cout << "OK" << endl;
		return 0;
	case 3:
		test_graph1();
		return 0;
	case 4:
		test_graph2();
		return 0;
	default:
		cout << "Invalid test number" << endl;
		return 1;
	}
}
