#ifndef TESTS_HPP
#define TESTS_HPP

#include <vector>
#include <functional>
#include <type_traits>
#include <string>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <type_traits>


class test_bench {
    class test_example {
        struct gs {
            using vertex_user_id_t = std::string;
            using vertex_property_t = std::tuple<std::string, int, double, char>;

            using edge_user_id_t = float;
            using edge_property_t = std::tuple<std::string, bool>;
        };
        using gdb_t = graph_db<gs>;
        gdb_t gdb;

        void fill() {
            typename gdb_t::vertex_t v1 = gdb.add_vertex("prvni");
            v1.set_properties("str1", 1, 1, false);
            auto v2 = gdb.add_vertex("druhy");
            v2.set_property<2>(2.2);
            auto v3 = gdb.add_vertex("treti", "str3", 3, 3.3, 3);
            gdb.add_vertex("ctvrty");

            typename gdb_t::edge_t e1 = gdb.add_edge(1.2, v1, v2);
            e1.set_properties("str1", false);
            gdb.add_edge(2.3, v2, v3, "str2", true);
            auto e3 = gdb.add_edge(1.3, v1, v3);
            e3.set_property<1>(true);

            auto ii = v1.template get_property<0>();

            // A simple check that storage is columnar & get_property doesn't return a copy.
            assert(&v1.template get_property<0>() + 1 == &v2.template get_property<0>());

            auto[neigbor_edges_begin, neighbor_edges_end] = v1.edges();
            auto i = *neigbor_edges_begin;
        }

        void print() {
            std::cout << "Graph:\n";
            auto[vertexes_begin, vertexes_end] = gdb.get_vertexes();
            static_assert(std::is_same_v<decltype(vertexes_begin), typename gdb_t::vertex_it_t>,
                          "Wrong vertex iterator type");
            std::for_each(vertexes_begin, vertexes_end, [](const typename gdb_t::vertex_t &vertex) {
                std::cout << "(" << vertex.id() << ") with p1=='" << vertex.template get_property<0>()
                          << "', p2=='" << std::get<1>(vertex.get_properties()) << "\n";
                auto[neigbor_edges_begin, neighbor_edges_end] = vertex.edges();
                static_assert(std::is_same_v<decltype(neigbor_edges_begin), typename gdb_t::neighbor_it_t>,
                              "Wrong neighbor iterator type");
                std::for_each(neigbor_edges_begin, neighbor_edges_end, [&vertex](auto &&edge) {
                    assert(edge.src().id() == vertex.id());
                    std::cout << "  (" << edge.src().id() << ")-[" << edge.id() << "]->("
                              << edge.dst().id() << ") with p1=='" << std::get<0>(edge.get_properties())
                              << "', p2=='" << edge.template get_property<1>() << "'\n";
                });
            });
        }

        void clear() {
            // Clear
            auto[vertexes_begin, vertexes_end] = gdb.get_vertexes();
            std::for_each(vertexes_begin, vertexes_end, [](auto &&vertex) {
                vertex.set_properties("", 0, 0, 0);
                vertex.template set_property<0>(" ");
                auto[neigbor_edges_begin, neighbor_edges_end] = vertex.edges();
                std::for_each(neigbor_edges_begin, neighbor_edges_end, [&vertex](auto &&edge) {
                    assert(edge.src().id() == vertex.id());
                    edge.set_properties("", false);
                    edge.template set_property<0>(" ");
                });
            });
        }

    public:
        void run() {
            fill();
            print();
            clear();
            print();
        }
    };

    std::vector<std::function<void()>> tests;
public:
    test_bench() {
        tests.push_back([](){ test_example t; t.run(); });
    }

    void run_test(size_t i) const {
        tests[i]();
    }

    size_t test_count() const { return tests.size(); }

    void run_all_tests() const {
        for(auto &&t : tests) {
            t();
        }
    }
};

#endif //TESTS_HPP
