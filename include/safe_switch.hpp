// safe_switch.hpp: Put the brakes on "break;" breakage.
// Copyright 2015 by David Krauss.
// This source is released under the MIT license, http://opensource.org/licenses/MIT

#ifndef INCLUDED_SSTAR_SAFE_SWITCH_HPP
#define INCLUDED_SSTAR_SAFE_SWITCH_HPP

#include <initializer_list>
#include <utility>

#if __cplusplus < 201402
    namespace sstar_safe_switch {
    namespace impl {

    template< typename = void >
    struct equal_to {
        template< typename lhs, typename rhs >
        bool operator () ( lhs const & l, rhs const & r ) const
            { return l == r; }
    };
#else
#   include <functional>

    namespace sstar_safe_switch {
    namespace impl {
    using std::equal_to;
#endif

template< typename operand, typename comparison = equal_to<> >
class bound_comparison {
    operand o;
    comparison c;
public:
    bound_comparison( operand && in_o, comparison in_c = {} )
        : o( std::forward< operand >( in_o ) )
        , c( std::move( in_c ) ) {}
     
    typename std::decay< operand >::type const & get_value() const
        { return o; }
   
    template< typename rhs >
    bool operator () ( rhs const & r ) const
        { return { c( get_value(), r ) }; }
    
    template< typename rhs >
    bool operator () ( std::initializer_list< rhs > il ) const {
        for ( auto && r : il ) {
            if ( c( get_value(), r ) ) return true;
        }
        return false;
    }
    bool operator () ( std::initializer_list< typename std::decay< operand >::type > il ) const {
        for ( auto && r : il ) {
            if ( c( get_value(), r ) ) return true;
        }
        return false;
    }
};

} // end namespace impl

template< typename comparison = impl::equal_to<>, typename operand >
impl::bound_comparison< operand, comparison >
bind_comparator( operand && o, comparison c = {} )
    { return { std::forward< operand >( o ), std::move( c ) }; }

} // end namespace sstar_safe_switch

#endif
