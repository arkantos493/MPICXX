/**
 * @file info_modifier_test.cpp
 * @author Marcel Breyer
 * @date 2019-12-05
 *
 * @brief Test cases for the @ref mpicxx::info implementation.
 *
 * This file provides test cases for the modifying methods of a mpicxx::info object.
 */

#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

#include <mpicxx/info/info.hpp>


TEST(InfoTests, Clear) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // clear content
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

    // invoking another clear should be fine
    info.clear();
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 0);

}

TEST(InfoTests, Insert) {

    // check values
    mpicxx::info correct_info = { {"key4", "value4"}, {"key1", "value1"}, {"key2", "value2"} };

    // multiple inserts
    {
        // construct an info object
        mpicxx::info info = { {"key4", "value4"} };
        info.insert("key1", "value1");
        info.insert("key2", "value2");
        info.insert("key1", "value10");     // <- shouldn't get added
        info.insert("key4", "value4");      // <- shouldn't get added

        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);

        EXPECT_EQ(nkeys, 3);
        EXPECT_TRUE(info == correct_info);
    }
    // insert via iterator range
    {
        // construct an info object
        mpicxx::info info = { {"key4", "value4"} };
        std::vector<std::pair<const std::string, std::string>> vec = { {"key1", "value1"},  {"key2", "value2"},
                                                                       {"key1", "value10"}, {"key4", "value4"} };   // <- shouldn't get added

        info.insert(vec.begin(), vec.end());

        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);

        EXPECT_EQ(nkeys, 3);
        EXPECT_TRUE(info == correct_info);
    }
    // insert via initializer list
    {
        // construct an info object
        mpicxx::info info = { {"key4", "value4"} };

        info.insert({ {"key1", "value1"},  {"key2", "value2"},
                      {"key1", "value10"}, {"key4", "value4"} });   // <- shouldn't get added

        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);

        EXPECT_EQ(nkeys, 3);
        EXPECT_TRUE(info == correct_info);
    }

}

TEST(InfoTests, InsertOrAssign) {

    // check values
    mpicxx::info correct_info = { {"key4", "value40"}, {"key1", "value10"}, {"key2", "value2"} };

    // multiple inserts
    {
        // construct an info object
        mpicxx::info info = { {"key4", "value4"} };
        info.insert_or_assign("key1", "value1");
        info.insert_or_assign("key2", "value2");
        info.insert_or_assign("key1", "value10");      // <- should override {"key1", "value1"}
        info.insert_or_assign("key4", "value40");      // <- should override {"key4", "value4"}

        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);

        EXPECT_EQ(nkeys, 3);
        EXPECT_TRUE(info == correct_info);
    }
    // insert via iterator range
    {
        // construct an info object
        mpicxx::info info = { {"key4", "value4"} };
        std::vector<std::pair<const std::string, std::string>> vec = { {"key1", "value1"},  {"key2", "value2"},
                                                                       {"key1", "value10"}, {"key4", "value40"} };   // <- should override

        info.insert_or_assign(vec.begin(), vec.end());

        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);

        EXPECT_EQ(nkeys, 3);
        EXPECT_TRUE(info == correct_info);
    }
    // insert via initializer list
    {
        // construct an info object
        mpicxx::info info = { {"key4", "value4"} };

        info.insert_or_assign({ {"key1", "value1"},  {"key2", "value2"},
                                {"key1", "value10"}, {"key4", "value40"} });   // <- should override

        int nkeys;
        MPI_Info_get_nkeys(info.get(), &nkeys);

        EXPECT_EQ(nkeys, 3);
        EXPECT_TRUE(info == correct_info);
    }

}

TEST(InfoTests, Erase) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);


    // create copy for erasing
    mpicxx::info info_copy(info);

    // erase first and last elements
    info.erase(info.begin());
    info.erase(info.end() - 1);

    // info object should now contain 2 entries
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // check if the correct elements has been deleted
    char key[MPI_MAX_INFO_KEY];
    MPI_Info_get_nthkey(info.get(), 0, key);
    EXPECT_STREQ(key, "key2");
    MPI_Info_get_nthkey(info.get(), 1, key);
    EXPECT_STREQ(key, "key3");


    // restore state
    info =  info_copy;

    // erase first three elements
    info.erase(info.begin(), info.begin() + 3);

    // info object should now contain one entry
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // check if the correct elements has been deleted
    MPI_Info_get_nthkey(info.get(), 0, key);
    EXPECT_STREQ(key, "key4");

    // restore state
    info =  info_copy;

    // erase nothing (first == last)
    info.erase(info.begin(), info.begin());
    // info object should now still contain 4 entries
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);


    // restore state
    info =  info_copy;

    // erase elements by key
    info.erase("key1");
    info.erase("key3");
    info.erase("key4");

    // info object should now contain one entry
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);

    // check if the correct elements has been deleted
    MPI_Info_get_nthkey(info.get(), 0, key);
    EXPECT_STREQ(key, "key2");


    // assertion tests
    info = info_copy;
//    info.erase(info.end());             // past-the-end iterator -> out-of-bounds access
//    info.erase(info.begin() - 1);       // out-of-bounds access
//    info.erase(info_copy.begin());      // iterator pointing to another info object

//    info.erase(info.end(), info.begin());             // `first` past-the-end iterator -> out-of-bounds access
//    info.erase(info.begin() - 1, info.begin());       // `first` out-of-bounds access
//    info.erase(info_copy.begin(), info.begin());      // `first` iterator pointing to another info object
//    info.erase(info.begin(), info.end());             // `last` past-the-end iterator -> out-of-bounds access
//    info.erase(info.begin(), info.begin() - 1);       // `last` out-of-bounds access
//    info.erase(info.begin(), info_copy.begin());      // `last` iterator pointing to another info object
//    info.erase(info.begin() + 1, info.begin());       // `first` must be less or equal than `last`

//    info.erase("vvvvveeeeerrrrryyyyy llllloooonnnngggg kkkkkeeeeeyyyyy");       // key too long

}

TEST(InfoTests, Extract) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };

    // info object should now contain 4 entries
    int nkeys;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);

    // extract [key, value]-pair by iterator
    std::pair<std::string, std::string> key_value_pair = info.extract(info.begin() + 1);
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 3);

    // extracted [key, value]-pair is correct
    EXPECT_STREQ(key_value_pair.first.c_str(), "key2");
    EXPECT_STREQ(key_value_pair.second.c_str(), "value2");

    // change extracted [key, value]-pair and add it again
    key_value_pair.first = "key5";
    MPI_Info_set(info.get(), key_value_pair.first.c_str(), key_value_pair.second.c_str());

    // check if added correctly
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);


    // extract [key, value]-pair by key
    auto opt_pair = info.extract("key1");
    EXPECT_TRUE(opt_pair.has_value());
    if (opt_pair.has_value()) {
        MPI_Info_get_nkeys(info.get(), &nkeys);
        EXPECT_EQ(nkeys, 3);

        // extracted [key, value]-pair is correct
        EXPECT_STREQ(opt_pair.value().first.c_str(), "key1");
        EXPECT_STREQ(opt_pair.value().second.c_str(), "value1");

    }

    // try to extract non-existing key
    auto nullopt_pair = info.extract("key1");
    EXPECT_FALSE(nullopt_pair.has_value());
    EXPECT_TRUE(nullopt_pair == std::nullopt);

}

TEST(InfoTests, Merge) {

    // construct a info object using a std::initializer_list<>
    mpicxx::info info = { {"key1", "value1"},
                          {"key2", "value2"},
                          {"key3", "value3"},
                          {"key4", "value4"} };
    mpicxx::info info_2 = { {"key1", "value10"}, {"key5", "value5"} };

    // check info object sizes
    int nkeys, flag;
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 4);
    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 2);

    // merge info objects
    info.merge(info_2);

    // check new sizes and [key, value]-pairs
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 5);
    char info_value[6 + 1];
    MPI_Info_get(info.get(), "key5", 6, info_value, &flag);
    EXPECT_STREQ(info_value, "value5");

    MPI_Info_get_nkeys(info_2.get(), &nkeys);
    EXPECT_EQ(nkeys, 1);
    char info_2_value[7 + 1];
    MPI_Info_get(info_2.get(), "key1", 7, info_2_value, &flag);
    EXPECT_STREQ(info_2_value, "value10");

    // check self-merge -> should do nothing
    info.merge(info);
    MPI_Info_get_nkeys(info.get(), &nkeys);
    EXPECT_EQ(nkeys, 5);

}