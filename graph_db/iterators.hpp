#ifndef ITERATORS_HPP
#define ITERATORS_HPP

#include "graph_db.hpp"

#include <vector>

template <class GraphSchema>
class graph_db;

template <class GraphSchema, typename T>
class my_iterator{ //iteartor used for vertex/edge iterating

public:
    my_iterator(const std::vector<T>* vec, std::size_t index): vec_(vec), index_(index) {}; 

    my_iterator operator++() {
        my_iterator it = *this;
        ++index_;
        return it;
    }
    my_iterator operator++(int) {
        ++index_;
        return *this;
    }

    T operator*() { //tady mozna pretypovani na T???
        return T((*vec_)[index_]);
    }

    bool operator!=(const my_iterator& it2) {
        return index_ != it2.index_;
    }
    
    bool operator==(const my_iterator& it2) {
        return !(this != it2);
    }

private:
    const std::vector<T>* vec_;
    std::size_t index_;

};

template <typename Ret, class GraphSchema>
class neighbour_iterator{ //iterator used for iterating over neighbours of specified vertex

public:
    neighbour_iterator(const std::vector< std::size_t>* ptr, graph_db<GraphSchema>* db, std::size_t index): ptr_(ptr), index_(index), db_(db) {};

    neighbour_iterator operator++ ()
    {
        neighbour_iterator it = *this;
        ++index_;
        return it;
    }

    neighbour_iterator operator++ (int)
    {
        ++index_;

        return *this;
    }

    Ret operator*() {
        
        return Ret((db_)->edges_[(*ptr_)[index_]]);
    }

    bool operator!=(const neighbour_iterator& it2) {
        return this->ptr_ != it2.ptr_ || this->index_ != it2.index_;
    }
    
    bool operator==(const neighbour_iterator& it2) {
        return !(this != it2);
    }

private:

    friend class graph_db<GraphSchema>;

    const std::vector<std::size_t>* ptr_;
    std::size_t index_;
    const graph_db<GraphSchema>* db_;

};



#endif //ITERATORS_HPP
