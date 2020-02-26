/**
 * @file examples/info/constructor.cpp
 * @author Marcel Breyer
 * @date 2020-02-16
 *
 * @brief Code snippets for the @ref mpicxx::info constructor implementations.
 */

//! [constructor iterator range]
std::vector<std::pair<const std::string, std::string>> key_value_pairs;
key_value_pairs.emplace_back("key1", "value1");
key_value_pairs.emplace_back("key2", "value2");
key_value_pairs.emplace_back("key1", "value1_override");
key_value_pairs.emplace_back("key3", "value3");

mpicxx::info obj(key_value_pairs.begin(), key_value_pairs.end());
//! [constructor iterator range]

//! [constructor initializer list]
mpicxx::info obj = { {"key1", "value1"},
                     {"key2", "value2"},
                     {"key1", "value1_override"},
                     {"key3", "value3"} };
//! [constructor initializer list]

//! [constructor MPI_Info]
MPI_Info mpi_info;
MPI_Info_create(&mpi_info);

mpicxx::info info (mpi_info, true);

mpi_info = MPI_INFO_NULL;    // <- does not change the value of 'info'!
//! [constructor MPI_Info]