#define BOOST_TEST_MODULE app_test_module

#include "../lib.h"
#include <boost/test/unit_test.hpp>
#include "Bulk.h"

BOOST_AUTO_TEST_SUITE(app_test_suite)

BOOST_AUTO_TEST_CASE(test_version)
{
    BOOST_CHECK(version() > 0);
}
BOOST_AUTO_TEST_SUITE_END()
