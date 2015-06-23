#include "safe_switch.hpp"

struct cc {};

bool operator == ( cc, char ) { return false; }

int main() {
    auto case_ = sstar_safe_switch::bind_comparator( cc{} );
    if ( case_({ 'a', 'b', 'c' }) ) {}
}
