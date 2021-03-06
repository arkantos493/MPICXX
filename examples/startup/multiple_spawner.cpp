/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-24
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Examples for some functions of the @ref mpicxx::multiple_spawner implementation.
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
//! [spawn without error codes]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// add command line arguments
ms.add_argv_at(0, "--file", "foo", "--size", 42);
ms.add_argv_at(1, "--file", "bar");

// add additional spawn information
mpicxx::info info;
info["wdir"] = "new/working/dir";
ms.set_spawn_info_at(1, info);

// spawn new executables
mpicxx::spawn_result res = ms.spawn();
//! [spawn without error codes]
//! [spawn with error codes]
// create multiple_spawner spawning exactly two new executables
mpicxx::multiple_spawner ms({ { "a.out", 4 }, { "b.out", 2 } });

// add command line arguments
ms.add_argv_at(0, "--file", "foo", "--size", 42);
ms.add_argv_at(1, "--file", "bar");

// add additional spawn information
mpicxx::info info;
info["wdir"] = "new/working/dir";
ms.set_spawn_info_at(1, info);

// spawn new executables
mpicxx::spawn_result_with_errcodes res = ms.spawn_with_errcodes();
//! [spawn with error codes]