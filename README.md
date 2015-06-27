`safe_switch` library
=====================

A small C++ utility class to let `if`-`else` work like `switch`.

*by David Krauss (potatoswatter)*
<!-- language: lang-cxx -->

Summary
=======

This utility provides the strengths of the `switch`-`case`-`break` idiom without the potential to forget a `break` statement.

Features include:

- arbitrary values, not only integers and enumerations
- several values in one case
- comparisons besides strict equality
  - less-than (`std::less<>`), for selection between numeric ranges
  - approximate equality, for floating-point values (comparison algorithm not included)
- predicates besides simple value comparison
- heterogeneous-type comparisons (avoiding implicit conversion)
- safe with rvalues (captured by value) and efficient with lvalues (captured by reference)

A typical `switch` goes like this:

    switch ( condition ) {
    case A:
        foo();
        break;
    case B:
    case C:
        bar();
        break;
    default:
        baz();
        break;
    }

With this utility it would be written like this:

    auto case_ = bind_comparator( condition );
    if ( case_( A ) ) {
        foo();
    } else if ( case_({ B, C }) ) {
        bar();
    } else {
        baz();
    }

Or this (having enabled the macros):

    SWITCH ( condition )
    CASE ( A ) {
        foo();
    }
    CASE ({ B, C }) {
        bar();
    }
    DEFAULT {
        baz();
    }


Usage
=====

The entire interface is the function `sstar_safe_switch::bind_comparator`.

    template< typename comparison = std::equal_to<>, typename operand >
    impl::bound_comparison< operand, comparison >
    bind_comparator( operand && o, comparison c = {} );

The name is long to write every time, so you might write inside one of your namespaces:

    using sstar_safe_switch::bind_comparator;

It returns the class `bound_comparison`. This could perhaps be useful separately, but better options include `std::bind` with
a comparator object, or a lambda function. The result is not intended for encapsulation. It should only be used as a local variable.


`bind_comparator()`
-------------------

The first parameter, `operand && o`, is the subject of the comparison.
If it is an lvalue, such as the name of a variable `x` or `vec[3]`, that variable will be passed to the comparison.
If it is an rvalue like `x+3` or `std::vector<int>{1,2}`, a copy is saved and passed to the comparison.

The second parameter, `comparison c = {}`, is optional. It defaults to the `==` operator via the `std::equal_to` wrapper.
A stateless comparison can be specified by an explicit template argument like `bind_comparator< std::less<> >( var )`.
A stateful comparison can be passed as a second parameter like `bind_comparator( value, fuzzy_compare( 0.001 ) )`.

The bound parameter will be passed as the left-hand side or first argument to the comparator.
The case values will be passed as the right-hand side or second argument.
So, using `std::less` creates conditions like `var < 3`, and the cases should be listed from least to greatest (ascending order).


The case object
---------------

The result is a function-like object. Given a value or set of values, it performs the comparison and returns `true` or `false`.
A good, canonical name for it is `case_`. There is no need to name it differently for different "switches."
Behind the scenes, there are three overloads. The first handles single-value cases:

    template< typename rhs >
    bool operator () ( rhs const & r ) const;
    
    // usage: if ( case_( 5 ) ) …

The type `rhs` can be anything. Often it will be the same type as `operand`, but it doesn't matter, as long as the comparison works.
The default comparison, `std::equal_to<>`, forms an expression using `==` *without* first performing conversions.

### List-based cases

Likewise, there is a template for handling lists of values:

    template< typename rhs >
    bool operator () ( std::initializer_list< rhs > il ) const;
    
    // usage: if ( case_({ "foo", "bar", "baz" }) ) …

If the comparison returns `true` for any value in the list, the function returns `true`.
In this example, if the `operand` type is `std::string`, it will be compared to the literals `"foo"` etc. without creating
`std::string` temporaries. This saves some overhead.

However, template type deduction of `rhs` in `std::initializer_list< rhs >` requires that every expression in the list have
the same type. When this isn't the case, or if the expressions are all of type `operand`, a non-template overload is used:

    bool operator () ( std::initializer_list< std::decay_t< operand > > il ) const;
    
    // usage: if ( case_({ std::string( "foo" ), "bar" }) ) …

(The `decay_t` business is simply stripping reference qualification.
The list contains values even if the class encapsulates a reference.)

In this example, `"bar"` will be converted to `std::string`. For optimal performance, you might forgo the list syntax like so:

    if ( case_( std::string( "foo" ) || case_( "bar" ) ) …

(For the sake of argument, using `case_({ std::string( "foo" ).c_str(), "bar" })` would achieve the same.)

### Retrieving the value

If you don't want to use the the encapsulated comparator, the case object provides a method `get_value`.

    std::decay_t< operand > const & get_value() const;
    
    // usage: if ( std::isspace( case_.get_value() ) ) …


Macros
======

The `"safe_switch_macros.hpp"` header provides macros so you can write
`SSTAR_SAFE_SWITCH` and `SSTAR_SAFE_SWITCH_CASE` instead of `bind_comparator` and `else if`:

    SSTAR_SAFE_SWITCH ( control_value )
    SSTAR_SAFE_SWITCH_CASE ( case_value ) {
        do_something();
    } SSTAR_SAFE_SWITCH_CASE ({ case2, case3 }) {
        something_else();
    } SSTAR_SAFE_SWITCH_DEFAULT {
        fallback();
    }

Under the hood, this is still an `if`-`else` ladder. You can still write `else if`, and if you do, the `SSTAR_SAFE_SWITCH_VALUE`
macro expands to call the `get_value()` method of the control object.

The case object introduced by `SSTAR_SAFE_SWITCH` is only in scope until the last `else` clause, then it's destroyed.

The macros have no slot for explicit template arguments. For a default-constructed custom comparison object, explicitly pass it
as a second parameter.

    SSTAR_SAFE_SWITCH ( control_value, std::less<>{} )

These macro names are verbose, to prevent name collisions. You probably want to alias them to something else.
A convenience interface is provided to this end: `#include "begin_safe_switch"` will create a "preprocessing scope"
mapping the macro names:

    SWITCH => SSTAR_SAFE_SWITCH
    CASE => SSTAR_SAFE_SWITCH_CASE
    DEFAULT => SSTAR_SAFE_SWITCH_DEFAULT
    SWITCH_VALUE => SSTAR_SAFE_SWITCH_VALUE

The "scope" created by ``#include "begin_safe_switch"`` should be terminated by `#include "end_safe_switch"`.
At the end of the outermost scope, the short macro names will be `#undef`ed.

These scopes may nest, but the nesting levels are limited to 16 deep. This can be extended if you file a bug report.
(Nesting `begin_safe_switch` and `end_safe_switch` is not important to safety, anyway. You can still nest `SWITCH` statements
without a second `#include "begin_safe_switch"`.)

    #include "begin_safe_switch"
    SWITCH ( control_value )
    CASE ( case_value ) {
        do_something();
    } CASE ({ case2, case3 }) {
        something_else();
    } else if ( complex_predicate( SWITCH_VALUE ) ) {
        #include "begin_safe_switch"
        SWITCH ( inner_value )
        CASE ( inner_case ) {
            ugh();
        }
        #include "end_safe_switch"
    } DEFAULT {
        fallback();
    }
    #include "end_safe_switch"

Note that no C++ scope is generated by these book-end headers. No error occurs if you forget `end_safe_switch`.


FAQ
===

### Why even bother?

Good question. Writing `if ( var == 5 )` is hardly error-prone. Some folks see `switch`…`case`…`break` as superior due to
less repetition. (Ironically, this argument tends to forget about `break`.)
If you already eschew `switch` and have no problem with `if`…`else`, then maybe this utility isn't for you.

This utility ensures that the condition variable and the comparison are specified at a single point, so they can be adjusted
without running through all the `if` statements. It's more handy when the comparison is more complicated than `==`.

### Performance?

There are several ways of implementing a `switch` at the machine level: a lookup table with indirect branching, a series of
conditional branches, or a tree of conditional branches. The compiler is responsible for choosing the right one.
Likewise for `if`…`else`. Any modern compiler *should* be able to translate seamlessly between the two.
If yours loses performance in a particular case, file a bug against it.

Before you file a bug, though, try using profile-guided optimization (PGO). This makes a huge difference for programs that
spend lots of time on branches, whether by `switch`, `if`, `goto`, or whatever.
Note that an indirect branch can have at most one predicted result, so a `switch` implemented that way needs to pick the same
path in the vast majority of cases.
CPUs are much better at predicting conditional branches, even when they're lined up in sequence. PGO gives the compiler
statistics to direct the CPU toward good predictions.
This is likely to cause table-based `switch`es to become more like `if`-`else`.

### C++03?

Aside from `std::initializer_list`, which is only sugar, nothing here really needs C++11. It could be back-ported to C++98/C++03.
However, we're well into the 2010's and nobody should be using such old compilers.
The attitude that compiler upgrades lead to dangerous random breakage is very obsolete. That is dinosaur thinking.

This is open source, though, and submissions are welcome.

### What about fallthrough not handled by the `initializer_list` overloads?

Use `goto`.
