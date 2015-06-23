// safe_switch_macros.hpp: Put the brakes on "break;" breakage, with preprocessor stylin'.
// Copyright 2015 by David Krauss.
// This source is released under the MIT license, http://opensource.org/licenses/MIT

#ifndef INCLUDED_SSTAR_SAFE_SWITCH_MACROS_HPP
#define INCLUDED_SSTAR_SAFE_SWITCH_MACROS_HPP

#include "safe_switch.hpp"

namespace sstar_safe_switch {
namespace impl {

template< typename base >
struct ladder_control : base {
    ladder_control( base && b )
        : base( std::move( b ) ) {}
    
    explicit operator bool () const { return false; }
};

template< typename base >
ladder_control< base > make_ladder_control( base && b )
    { return { std::move( b ) }; }

}}

#define SSTAR_SAFE_SWITCH( ... ) \
if ( auto SSTAR_SAFE_SWITCH_CONTROL = ::sstar_safe_switch::impl::make_ladder_control( \
    ::sstar_safe_switch::bind_comparator( __VA_ARGS__ ) ) ) ;

#define SSTAR_SAFE_SWITCH_CASE( ... ) \
else if ( SSTAR_SAFE_SWITCH_CONTROL( __VA_ARGS__ ) )

#define SSTAR_SAFE_SWITCH_DEFAULT else

#define SSTAR_SAFE_SWITCH_VALUE (SSTAR_SAFE_SWITCH_CONTROL.get_value())

#endif
