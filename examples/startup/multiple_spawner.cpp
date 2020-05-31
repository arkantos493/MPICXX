/**
 * @file examples/startup/multiple_spawner.cpp
 * @author Marcel Breyer
 * @date 2020-05-31
 *
 * @brief Examples for the mpicxx::multiple_spawner::add_argv(...) and mpicxx::multiple_spawner::add_argv_add(...) implementations.
 */

//! [add_argv version with iterator range]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// create a two dimensional vector holding all command line arguments
std::vector<std::vector<std::string>> argvs(2);
argvs[0] = { "-foo", "bar", "--baz" };  // command line arguments for the first executable
argvs[1] = { "-bar", "1", "-qux" };     // command line arguments for the second executable

// add all command line arguments to the spawner
ms.add_argv(argvs.begin(), argvs.end());
//! [add_argv version with iterator range]
//! [add_argv version with initializer_list]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// add all command line arguments to the spawner
ms.add_argv({ { "-foo", "bar", "--baz" }, { "-bar", "1", "-qux" } });
//! [add_argv version with initializer_list]
//! [add_argv version with parameter pack]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// create command line arguments
std::string c_arr[] = { "-foo", "bar", "--baz" };   // command line arguments for the first executable
std::array<int, 3> arr = { { 1, 2, 3 } };           // command line arguments for the second executable

// add all command line arguments to the spawner
ms.add_argv(c_arr, arr);
//! [add_argv version with parameter pack]
//! [add_argv_at version with iterator range]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// create a two dimensional vector holding all command line arguments
std::vector<std::string> argvs_1 = { "-foo", "bar", "--baz" };  // command line arguments for the first executable
std::vector<double> argvs_2 = { 1.4, 2.5, 3.6 };                // command line arguments for the second executable

// add all command line arguments to the spawner
ms.add_argv_at(0, argvs_1.begin(), argvs_1.end());
ms.add_argv_at(1, argvs_2.begin(), argvs_2.end());
//! [add_argv_at version with iterator range]
//! [add_argv_at version with initializer_list]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// add all command line arguments to the spawner
ms.add_argv_at(0, { "-foo", "bar", "--baz" });
ms.add_argv_at(1, { 1, 2, 3 });
//! [add_argv_at version with initializer_list]
//! [add_argv_at version with parameter pack]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// add all command line arguments to the spawner
ms.add_argv_at(0, "-foo", "bar", "--baz");
ms.add_argv_at(1, 1, 2, 3);
//! [add_argv_at version with parameter pack]