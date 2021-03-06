//  :copyright: (c) 2017 Alex Huszagh.
//  :license: MIT, see LICENSE.md for more details.
/*
 *  \addtogroup Tests
 *  \brief JSON SAX unittests.
 */

#include <pycpp/filesystem.h>
#include <pycpp/json.h>
#include <pycpp/stl/fstream.h>
#include <pycpp/stl/sstream.h>
#include <gtest/gtest.h>

PYCPP_USING_NAMESPACE

// HELPERS
// -------


template <typename SaxReader, typename OpenArg>
static void test_json_reader(SaxReader& reader, OpenArg& arg, bool move = false)
{
    json_document_t document;
    json_dom_handler handler(document);
    SaxReader r2;
    if (move) {
        SaxReader r2;
        r2.set_handler(handler);
        r2.swap(reader);
    } else {
        reader.set_handler(handler);
    }
    reader.open(arg);

    ASSERT_TRUE(document.has_object());
    auto &object = document.get_object();
    EXPECT_EQ(object.size(), 7);
    EXPECT_EQ(object["hello"].get_string(), "world");
    EXPECT_EQ(object["t"].get_boolean(), true);
    EXPECT_EQ(object["pi"].get_number(), 3.1416);
    EXPECT_EQ(object["a"].get_array().size(), 4);
    EXPECT_EQ(object["a"].get_array().front().get_number(), 1.);

    auto str = document.dumps(' ', 0);
    // only check the first character, since the order isn't defined
    EXPECT_EQ(str.substr(0, 1), "{");

    str = document.dumps(' ', 4);
    // only check the first two to ensure newlines are added
    EXPECT_EQ(str.substr(0, 2), "{\n");
}

// TESTS
// -----


TEST(json, json_stream_reader)
{
    // don't worry about compliance testing:
    // the backends are robustly tested
    json_string_t str(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");
    {
        json_istringstream_t sstream(str);
        json_stream_reader reader;
        test_json_reader(reader, sstream);
    }
    {
        json_istringstream_t sstream(str);
        json_stream_reader reader;
        test_json_reader(reader, sstream, true);
    }
}


TEST(json, json_file_reader)
{
    // don't worry about compliance testing:
    // the backends are robustly tested
    json_string_t str(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");
    string path("test.json");
    {
        ofstream ostream(path);
        ostream << str << endl;
    }
    {
        json_file_reader reader;
        test_json_reader(reader, path);
    }
    {
        json_file_reader reader;
        test_json_reader(reader, path, true);
    }
    EXPECT_TRUE(remove_file(path));
}


TEST(json, json_string_reader)
{
    // don't worry about compliance testing:
    // the backends are robustly tested
    json_string_t str(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");
    {
        json_string_reader reader;
        test_json_reader(reader, str);
    }
    {
        json_string_reader reader;
        test_json_reader(reader, str, true);
    }
}
