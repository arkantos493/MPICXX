/**
 * @file 
 * @author Marcel Breyer
 * @date 2020-07-16
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Examples for the @ref mpicxx::main() implementation.
 */

//! [mpicxx_main version without args and thread support]
#include <mpicxx/startup/mpicxx_main.hpp>

int mpicxx_main() {
    // no need to call a initialization function!

    // user code

    // no need to call a finalization function!
    return 0;
}

int main() {
    return mpicxx::main(&mpicxx_main);    // can't make any mistake related to initialization or finalization!
}
//! [mpicxx_main version without args and thread support]
//! [mpicxx_main version with args and without thread support]
#include <mpicxx/startup/mpicxx_main.hpp>

int mpicxx_main(int argc, char** argv) {
    // no need to call a initialization function!

    // user code

    // no need to call a finalization function!
    return 0;
}

int main(int argc, char** argv) {
    return mpicxx::main(&mpicxx_main, argc, argv);    // can't make any mistake related to initialization or finalization!
}
//! [mpicxx_main version with args and without thread support]
//! [mpicxx_main version without args and with thread support]
#include <mpicxx/startup/mpicxx_main.hpp>

int mpicxx_main() {
    // no need to call a initialization function!
    // no need to catch an exception; if this function gets called, the required level of thread support could be satisfied

    // user code

    // no need to call a finalization function!
    return 0;
}

int main() {
    // can't make any mistake related to initialization or finalization!
    return mpicxx::main(&mpicxx_main, mpicxx::thread_support::multiple);
}
//! [mpicxx_main version without args and with thread support]
//! [mpicxx_main version with args and thread support]
#include <mpicxx/startup/mpicxx_main.hpp>

int mpicxx_main(int argc, char** argv) {
    // no need to call a initialization function!
    // no need to catch an exception; if this function gets called, the required level of thread support could be satisfied

    // user code

    // no need to call a finalization function!
    return 0;
}

int main(int argc, char** argv) {
    // can't make any mistake related to initialization or finalization!
    return mpicxx::main(&mpicxx_main, argc, argv, mpicxx::thread_support::multiple);
}
//! [mpicxx_main version with args and thread support]