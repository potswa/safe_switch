#include "safe_switch_macros.hpp"
#include <cstdlib>

int main() {
    #include "begin_safe_switch"
    SWITCH ( 42 )
    CASE ( 2 ) abort();
    CASE ( 42 ) {
        #include "begin_safe_switch"
        SWITCH ( 21 )
        CASE ( -100 ) abort();
        CASE ({ 1, 2, 21 }) {
            #include "begin_safe_switch"
            SWITCH ( 5 )
            else if ( SWITCH_VALUE == 18 ) abort();
            DEFAULT {}
            #include "end_safe_switch"
        } DEFAULT abort();
        #include "end_safe_switch"
    } DEFAULT abort();
    #include "end_safe_switch"
}

#ifdef SWITCH
#   error test failure
#endif
