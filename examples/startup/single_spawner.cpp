/**
 * @file
 * @author Marcel Breyer
 * @date 2020-07-24
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Examples for some functions of the @ref mpicxx::single_spawner implementation.
 */

//! [spawn without error codes]
// create single_spawner
mpicxx::single_spawner ss("a.out", 4);

// add command line arguments
ss.add_argv("--file", "foo", "--size", 42);

// add additional spawn information
mpicxx::info info;
info["wdir"] = "new/working/dir";
ss.set_spawn_info(info);

// spawn new executables
mpicxx::spawn_result res = ss.spawn();
//! [spawn without error codes]
//! [spawn with error codes]
// create single_spawner
mpicxx::single_spawner ss("a.out", 4);

// add command line arguments
ss.add_argv("--file", "foo", "--size", 42);

// add additional spawn information
mpicxx::info info;
info["wdir"] = "new/working/dir";
ss.set_spawn_info(info);

// spawn new executables
mpicxx::spawn_result_with_errcodes res = ss.spawn_with_errcodes();
//! [spawn with error codes]