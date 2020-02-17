/**
 * @file examples/info/info.cpp
 * @author Marcel Breyer
 * @date 2020-02-16
 *
 * @brief Examples for the @ref mpicxx::info implementation.
 */

#include <iostream>

#include <mpicxx/info/info.hpp>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    {
        // create new info object
        mpicxx::info info = {{ "key1", "value1" }, { "key2", "value2" }};

        // change value of a specific key
        info["key1"] = "value1_override";
        // print value of a specific key
        std::cout << info["key2"] << std::endl;

        // check info object if it contains a specific key
        if (info.contains("key1")) {
            std::cout << "key found!" << std::endl;
        }

        // copy info object and compare them
        mpicxx::info info_copy(info);
        if (info == info_copy) {
            std::cout << "info objects are equal" << std::endl;
        }

        // remove first element from a info object
        auto it = info.erase(info.begin());

        // print all elements of the MPI_INFO_ENV object
        for (const auto& [key, value] : mpicxx::info::env) {
            std::cout << key << " : " << value << std::endl;
        }
    }
    MPI_Finalize();
    return 0;
}