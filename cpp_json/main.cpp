//
//  Copyright 2011 Kaoru Yanase. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include <cpp_json/cjson_wrapper.hpp>
#include <string>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/functions.hpp>

struct func
{
    template <typename ValueType>
    void operator()(ValueType const& j) const
    {
    }
};

BOOST_AUTO_TEST_CASE( cpp_json_test )
{
    using namespace cpp_json;
    using namespace boost;

    cjson_document j = parse("{\"code\":0,\"message\":\"OK\",\"list\":[{\"id\":\"1234\"},{\"id\":\"abcd\"}],\"empty-list\":[]}");
    BOOST_CHECK_EQUAL( j["code"].as<int>(), 0);
    BOOST_CHECK_EQUAL( j["message"].as<const char *>(), "OK");

    {
        cjson_wrapper::iterator first = j["empty-list"].begin();
        cjson_wrapper::iterator last = j["empty-list"].end();
        BOOST_CHECK( first == last );
    }

    {
        cjson_wrapper::iterator first = j["list"].begin();
        cjson_wrapper::iterator last = j["list"].end();

        {
            cjson_wrapper const& j = *first++;
            BOOST_CHECK_EQUAL( j["id"].as<const char *>(), "1234");
        }

        {
            cjson_wrapper const& j = *first++;
            BOOST_CHECK_EQUAL( j["id"].as<const char *>(), "abcd");
        }

        BOOST_CHECK( first == last );
    }

    {
        cjson_wrapper::iterator first = j["list"].begin();
        cjson_wrapper::iterator last = j["list"].end();

        boost::for_each(
            make_iterator_range(first, last),
            func()
        );
    }

    {
        boost::for_each(
            j["list"],
            func()
        );
    }
}
