// (C) Copyright Jonathan Turkanis 2004
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// This test should compile and execute successfully for both static_move_ptr
// and dynamic_move_ptr.

#include <boost/interfaces/unique_obj.hpp>
#include "../detail/counter.hpp"
#include "../detail/hierarchies.hpp"
#include "../detail/unit_test_framework.hpp"

using namespace boost::interfaces;
using namespace boost::interfaces::test;
using boost::unit_test_framework::test_suite; 

unique_obj<ifirst> ifirst_source()
{ return unique_obj<ifirst>(new cfirst<1>); }

unique_obj<iderived1> iderived1_source()
{ return unique_obj<iderived1>(new cderived1<1>); }

unique_obj<iderived4> iderived4_source()
{ return unique_obj<iderived4>(new cderived4<1>); }

void sink(unique_obj<ifirst> obj) 
{ 
    BOOST_CHECK(obj ? true : false);
    BOOST_CHECK(!!obj);
    BOOST_CHECK(get(obj) != 0);
}

void source_sink_test()
{
    sink(ifirst_source());
    BOOST_CHECK(counter< cfirst<1> >::value() == 0);

    sink(iderived1_source());
    BOOST_CHECK(counter< cderived1<1> >::value() == 0);

    sink(iderived4_source());
    BOOST_CHECK(counter< cderived4<1> >::value() == 0);
}

test_suite* init_unit_test_suite(int, char* []) 
{
    test_suite* test = BOOST_TEST_SUITE("unique_obj source sink test");
    test->add(BOOST_TEST_CASE(&source_sink_test));
    return test;
}
