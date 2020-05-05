#ifndef COLLUMNS_HPP
#define COLLUMNS_HPP

#include <vector>
#include <tuple>

template <class C, typename T2>
class columns;

template <class GraphSchema, typename ...Props>
class columns<GraphSchema, std::tuple<Props...>> {

public:

    void append_empty() noexcept {
        //used for initializing new vertex/edge to create empty row
        append_empty_props(std::make_index_sequence<sizeof...(Props)>{});
    }
    
    template <std::size_t ...I>
    auto get_properties(std::size_t row, std::index_sequence<I...>) const noexcept {
        //return row from table
        return std::make_tuple(std::get<I>(properties_)[row]...);
    }

    template<std::size_t I>
    decltype(auto) get_property(std::size_t row){
        //returns property from in given collumn from given row
        return std::get<I>(properties_)[row];
    }

    template <std::size_t ...I, typename ...Ts>
    void assign_properties(std::size_t row, std::index_sequence<I...>, Ts &&...props) noexcept {
        //asigns given props to given row for vertex/edge
        if (sizeof...(props) == std::tuple_size<std::tuple<Props...>>::value)
            ( assign<I>(row, std::forward<Ts>(props)), ... );

    }

    template<std::size_t I, typename PropType>
    void assign_property(std::size_t row, const PropType &prop)  noexcept {
        std::get<I>(properties_)[row] = prop;
    }

private:
    std::tuple<std::vector<Props>...> properties_;

    template <std::size_t ...I>
    void append_empty_props(std::index_sequence<I...>) noexcept {
        //creates empty row
        ( std::get<I>(properties_).emplace_back(), ... );
    }

    template <std::size_t I, typename T>
    void assign(std::size_t row, T &&src) noexcept {
        //helper function called from assign_properties function for setting values into specified row
        std::get<I>(properties_)[row] = src;
    }

};


#endif //COLLUMNS_HPP