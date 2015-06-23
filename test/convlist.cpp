#include "safe_switch.hpp"
#include <cstdlib>

struct cc {
    cc( char ) {}
    cc( int ) { throw 0; }
};

bool operator == ( cc, cc ) { return true; }

int main() {
    try {
        auto case_ = sstar_safe_switch::bind_comparator( cc{ 'x' } );
        if ( case_({ 'a', 'b', 3 }) ) {}
        std::abort();
    } catch ( int ) {}
}
