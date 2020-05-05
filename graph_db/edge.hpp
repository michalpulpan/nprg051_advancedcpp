#ifndef EDGE_HPP
#define EDGE_HPP

#include "graph_db.hpp"

template <class GraphSchema>
class graph_db;

template <class GraphSchema>
class edge {
public:

    edge(std::size_t internal_id, std::size_t src_id, std::size_t dst_id, graph_db<GraphSchema>* db)
        : internal_id_(internal_id), src_id_(src_id), dst_id_(dst_id), db_(db) {}


    /**
     * @brief Returns the immutable user id of the element.
     */
    auto id() const{
        //returns user defined id from vector stored in main database
        return db_->edge_user_ids_[internal_id_];
    }

    /**
     * @brief Returns all immutable properties of the element in tuple.
     * @note The return type is GraphSchema::vertex_property_t for vertexes and GraphSchema::edge_property_t for edges.
     */
    auto get_properties() const{
        //return properties returned from function specified in collumns.hpp
        return db_->edge_cols_.get_properties(internal_id_, std::make_index_sequence<std::tuple_size<typename GraphSchema::edge_property_t>::value>{});
    }

    /**
     *
     * @brief Returns a single immutable property of the I-th element.
     * @tparam I An index of the property.
     * @return The value of the property.
     * @note The first property is on index 0.
     */
    template<size_t I>
    decltype(auto) get_property() const{
        //return property returned from function specified in collumns.hpp
        return db_->edge_cols_.template get_property<I>(internal_id_);
    }

    /**
     * @brief Sets the values of properties of the element.
     * @tparam PropsType Types of the properties.
     * @param props The value of each individual property.
     * @note Should not compile if not provided with all properties.
     */
    template<typename ...PropsType>
    void set_properties(PropsType &&...props){
        //sets properties using function specified in collumns.hpp
        auto index_seq = std::make_index_sequence<std::tuple_size<typename GraphSchema::edge_property_t>::value>{};
        return db_->edge_cols_.assign_properties(internal_id_, index_seq, std::forward<PropsType>(props)...);
    }

    /**
     * @brief Set a value of the given property of the I-th element
     * @tparam I An index of the property.
     * @tparam PropType The type of the property.
     * @param prop The new value of the property.
     * @note The first property is on index 0.
     */
    template<size_t I, typename PropType>
    void set_property(const PropType &prop){
        //sets property using function specified in collumns.hpp
        return db_->edge_cols_.template assign_property<I>(internal_id_, prop);
    }

    /**
     * @brief Returns the source vertex of the edge.
     * @return The vertex.
     */
    auto src() const{

        //returns source vertex of this edge fromm vector of vertices stored in main database
        return db_->vertices_[src_id_];
    }

    /**
     * @brief Returns the destination vertex of the edge.
     * @return The vertex.
     */
    auto dst() const{
        //returns destionation vertex of this edge fromm vector of vertices stored in main database
        return db_->vertices_[dst_id_];
    }

private:

    friend class graph_db<GraphSchema>;

    std::size_t internal_id_, src_id_, dst_id_; //id used for indexing vectors, id of source vertex and destionation vertex
    graph_db<GraphSchema> *db_; //pointer to main database storing all neccessary data

};

#endif //EDGE_HPP