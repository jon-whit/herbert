// (C) Copyright Jonathan Turkanis 2004
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// This test should compile and execute successfully for both static_move_ptr
// and dynamic_move_ptr.

#include <boost/interfaces/shared_obj.hpp>
#include "../detail/counter.hpp"
#include "../detail/hierarchies.hpp"
#include "../detail/unit_test_framework.hpp"

using namespace boost::interfaces;
using namespace boost::interfaces::test;
using boost::unit_test_framework::test_suite; 

shared_obj<ifirst> ifirst_source()
{
    return shared_obj<ifirst>(new cfirst<1>);
}

shared_obj<iderived1> iderived1_source()
{
    return shared_obj<iderived1>(new cderived1<1>);
}

shared_obj<iderived4> iderived4_source()
{
    return shared_obj<iderived4>(new cderived4<1>);
}

void assignment_test()
{
    {
        shared_obj<ifirst> obj;
        obj = ifirst_source();
    }
    BOOST_CHECK(counter< cfirst<1> >::value() == 0);

    {
        shared_obj<iderived1> obj;
        obj = iderived1_source();
    }
    BOOST_CHECK(counter< cderived1<1> >::value() == 0);

    {
        shared_obj<iderived4> obj;
        obj = iderived4_source();
    }
    BOOST_CHECK(counter< cderived4<1> >::value() == 0);
}

test_suite* init_unit_test_suite(int, char* []) 
{
    test_suite* test = BOOST_TEST_SUITE("shared_obj assignment test");
    test->add(BOOST_TEST_CASE(&assignment_test));
    return test;
}
