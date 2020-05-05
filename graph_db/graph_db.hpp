#ifndef GRAPH_DB_HPP
#define GRAPH_DB_HPP

#include "edge.hpp"
#include "vertex.hpp"
#include "collumns.hpp"
#include "iterators.hpp"


#include <vector>
#include <tuple>
#include <utility>

template <class GraphSchema>
class edge;
template <class GraphSchema>
class vertex;
template <class GraphSchema>
class graph_db;
template <class GraphSchema>
class edge;

template <class C, typename T2>
class columns;

template <class GraphSchema, typename T>
class my_iterator;
template <typename Ret, class GraphSchema>
class neighbour_iterator;


/**
 * @brief A graph database that takes its schema (types and number of vertex/edge properties, user id types) from a given trait
 * @tparam GraphSchema A trait which specifies the schema of the graph database.
 * @see graph_schema
 */
template<class GraphSchema>
class graph_db {
public:
    /**
     * @brief A type representing a vertex.
     * @see vertex
     */
    using vertex_t = vertex<GraphSchema>;
    /**
     * @brief A type representing an edge.
     * @see edge
     */
    using edge_t = edge<GraphSchema>;

    /**
     * @brief A type representing a vertex iterator. Must be at least of output iterator. Returned value_type is a vertex.
     * @note Iterate in insertion order.
     */
    using vertex_it_t = my_iterator<GraphSchema, vertex_t>;

    /**
     * @brief A type representing a edge iterator. Must be at least an output iterator. Returned value_type is an edge.
     * @note Iterate in insertion order.
     */
    using edge_it_t = my_iterator<GraphSchema, edge_t>;

    /**
     * @brief A type representing a neighbor iterator. Must be at least an output iterator. Returned value_type is an edge.
     * @note Iterate in insertion order.
     */
    using neighbor_it_t = neighbour_iterator<edge<GraphSchema>, GraphSchema>;

    /**
     * @brief Insert a vertex into the database.
     * @param vuid A user id of the newly created vertex.
     * @return The newly created vertex.
     * @note The vertex's properties have default values.
     */
    vertex_t add_vertex(typename GraphSchema::vertex_user_id_t &&vuid)
    {

        vertex_user_ids_.push_back(vuid);
        vertex_cols_.append_empty();
        vertex v(vertex_user_ids_.size()-1, this);
        //vertex_added(v);

        vertices_.push_back(v);
        neighbours_.emplace_back();
        
        return v;

    }
    vertex_t add_vertex(const typename GraphSchema::vertex_user_id_t &vuid)
    {
        vertex_user_ids_.push_back(vuid);
        vertex_cols_.append_empty();
        vertex_t v(vertex_user_ids_.size()-1, this);
        //vertex_added(v);

        vertices_.push_back(v);
        neighbours_.emplace_back();
        
        return v;
    }

    /**
     * @brief Insert a vertex into the database with given values of the vertex's properties.
     * @tparam Props All types of properties.
     * @param vuid A user id of the newly created vertex.
     * @param props Properties of the new vertex.
     * @return The newly created vertex.
     * @note Should not compile if not provided with all properties.
     */
    template<typename ...Props>
    vertex_t add_vertex(typename GraphSchema::vertex_user_id_t &&vuid, Props &&...props)
    {
        auto v = add_vertex(vuid);
        v.set_properties(props...);
        return v;
    }
    template<typename ...Props>
    vertex_t add_vertex(const typename GraphSchema::vertex_user_id_t &vuid, Props &&...props)
    {
        auto v = add_vertex(vuid);
        v.set_properties(props...);
        return v;    
    }

    /**
     * @brief Returns begin() and end() iterators to all vertexes in the database.
     * @return A pair<begin(), end()> of vertex iterators.
     * @note The iterator can iterate in any order.
     */
    std::pair<vertex_it_t, vertex_it_t> get_vertexes() const
    {
        return std::make_pair(vertex_it_t(&vertices_, 0),vertex_it_t(&vertices_, vertices_.size()));
    }

    /**
     * @brief Insert a directed edge between v1 and v2 with a given user id.
     * @param euid An user id of the edge.
     * @param v1 A source vertex of the edge.
     * @param v2 A destination vertex of the edge.
     * @return The newly create edge.
     * @note The edge's properties have default values.
     */
    edge_t add_edge(typename GraphSchema::edge_user_id_t &&euid, const vertex_t &v1, const vertex_t &v2)
    {
        edge_user_ids_.push_back(euid);
        edge_cols_.append_empty();
        edge_t e(edge_user_ids_.size()-1, v1.internal_id_, v2.internal_id_, this);
        //edge_added(e);

        edges_.push_back(e);
        neighbours_[e.src_id_].push_back(e.internal_id_);
        //(vertices_[e.src_id_]).neighbours_.push_back(e.internal_id_);
        return e;
    }
    edge_t add_edge(const typename GraphSchema::edge_user_id_t &euid, const vertex_t &v1, const vertex_t &v2)
    {
        edge_user_ids_.push_back(euid);
        edge_cols_.append_empty();
        edge_t e(edge_user_ids_.size()-1, v1.internal_id_, v2.internal_id_, this);
        //edge_added(e);

        edges_.push_back(e);
        neighbours_[e.src_id_].push_back(e.internal_id_);

        //(vertices_[e.src_id_]).neighbours_.push_back(e.internal_id_);

        return e;
    }

    /**
     * @brief Insert a directed edge between v1 and v2 with a given user id and given properties.
     * @tparam Props Types of properties
     * @param euid An user id of the edge.
     * @param v1 A source vertex of the edge.
     * @param v2 A destination vertex of the edge.
     * @param props All properties of the edge.
     * @return The newly create edge.
     * @note Should not compile if not provided with all properties.
     */
    template<typename ...Props>
    edge_t add_edge(typename GraphSchema::edge_user_id_t &&euid, const vertex_t &v1, const vertex_t &v2, Props &&...props){
        auto e = add_edge(euid, v1, v2);
        e.set_properties(props...);
        return e;
    }
    template<typename ...Props>
    edge_t add_edge(const typename GraphSchema::edge_user_id_t &euid, const vertex_t &v1, const vertex_t &v2, Props &&...props)
    {
        auto e = add_edge(euid, v1, v2);
        e.set_properties(props...);
        return e;
    }

    /**
     * @brief Returns begin() and end() iterators to all edges in the database.
     * @return A pair<begin(), end()> of edge iterators.
     * @note The iterator can iterate in any order.
     */
    std::pair<edge_it_t, edge_it_t> get_edges() const
    {
        return std::make_pair( edge_it_t(&edges_, 0), edge_it_t(&edges_, edges_.size())
        );
    }

private:

    friend class vertex<GraphSchema>;
    friend class edge<GraphSchema>;
    friend class neighbour_iterator<edge<GraphSchema>, GraphSchema>;
    friend class my_iterator<GraphSchema, vertex_t>;
    friend class my_iterator<GraphSchema, edge_t>;

    std::vector<edge_t> edges_; //vector of all edges -> indexes are internal ids
    std::vector<vertex_t> vertices_; //vector of all verticies -> indexes are internal ids

    std::vector<std::vector<std::size_t>> neighbours_; //2D vector of edges going from the same source

    std::vector<typename GraphSchema::vertex_user_id_t> vertex_user_ids_; //vector of user ids for vertexes -> indexes are internal ids
    std::vector<typename GraphSchema::edge_user_id_t> edge_user_ids_; //vector of user ids for edges -> indexes are internal ids

    columns<GraphSchema, typename GraphSchema::vertex_property_t> vertex_cols_; //collumnar database for properties of verties
    columns<GraphSchema, typename GraphSchema::edge_property_t> edge_cols_; //collumnar database for properties of edges

};

#endif //GRAPH_DB_HPP
