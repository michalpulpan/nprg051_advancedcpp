#include <iostream>
#include <queue>
#include <variant>
#include <string>

#include "graph_db.hpp"
#include "tests.hpp"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Wrong number of arguments";
        return -1;
    }
    size_t idx = std::stoi(argv[1]);
    test_bench t;
    if (idx > t.test_count()) {
        std::cerr << "Wrong argument";
        return -2;
    }

    if (idx == 0) {
        t.run_all_tests();
    } else {
        t.run_test(idx - 1);
    }
    return 0;
}

/*
        struct gs {
            using vertex_user_id_t = std::string;
            using vertex_property_t = std::tuple<std::string, int, double, char>;

            using edge_user_id_t = float;
            using edge_property_t = std::tuple<std::string, bool>;
        };

int main() {

    using db = graph_db<gs>;
    graph_db<gs> mydb;


    auto v1 = mydb.add_vertex("v1", "str1", 1, 1, 'k');
    v1.get_properties();
    auto prop1 = v1.get_property<0>();
    
    auto v2 = mydb.add_vertex("v2");
    v2.set_properties("str2", 2, 2, 'p');

    auto e1 = mydb.add_edge(1.2, v1, v2);
    e1.set_properties("e1_props", false);

    auto e2 = mydb.add_edge(28.0, v2, v1);
    e2.set_properties("e2_props", true);

    auto iii = e2.src().id();

    auto props = e2.get_properties();
    auto e3 = mydb.add_edge(1.2, v1, v2, "e3_props", false);

        using gdb_t = graph_db<gs>;
        gdb_t gdb;

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

        // A simple check that storage is columnar & get_property doesn't return a copy.
        assert(&v1.template get_property<0>() + 1 == &v2.template get_property<0>());

       
    return 0;
}*/