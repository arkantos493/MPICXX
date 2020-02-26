/**
 * @file examples/startup/init_and_finalize.cpp
 * @author Marcel Breyer
 * @date 2020-02-26
 *
 * @brief Examples for the @ref mpicxx::init() and @ref mpicxx::finalize() implementation.
 */

//! [normal version without args and thread support]
#include <mpicxx/startup/init.hpp>
#include <mpicxx/startup/finalize.hpp>

int main() {
    mpicxx::init();         // don't forget the initialization call (exactly once)!
    {                       // brackets needed or mpicxx objects would get destroyed after the finalization call which is not permitted

        // user code

    }
    mpicxx::finalize();     // don't forget the finalization call (exactly once)!
    return 0;
}
//! [normal version without args and thread support]
//! [normal version with args and without thread support]
#include <mpicxx/startup/init.hpp>
#include <mpicxx/startup/finalize.hpp>

int main(int argc, char** argv) {
    mpicxx::init(argc, argv);   // don't forget the initialization call (exactly once)!
    {                           // brackets needed or mpicxx objects would get destroyed after the finalization call which is not permitted

        // user code

    }
    mpicxx::finalize();         // don't forget the finalization call (exactly once)!
    return 0;
}
//! [normal version with args and without thread support]
//! [normal version without args and with thread support]
#include <mpicxx/startup/init.hpp>
#include <mpicxx/startup/finalize.hpp>

int main() {
    try {
        mpicxx::init(mpicxx::thread_support::MULTIPLE);     // don't forget the initialization call (exactly once)!

        // user code

    } catch(const mpicxx::thread_support_not_satisfied& e) {
        std::cout << e.what() << std::endl;
    }
    mpicxx::finalize();                                     // don't forget the finalization call even in case of an exception (exactly once)!
    return 0;
}
//! [normal version without args and with thread support]
//! [normal version with args and thread support]
#include <mpicxx/startup/init.hpp>
#include <mpicxx/startup/finalize.hpp>

int main(int argc, char** argv) {
    try {
        mpicxx::init(argc, argv, mpicxx::thread_support::MULTIPLE);     // don't forget the initialization call (exactly once)!

        // user code

    } catch(const mpicxx::thread_support_not_satisfied& e) {
        std::cout << e.what() << std::endl;
    }
    mpicxx::finalize();                                                 // don't forget the finalization call even in case of an exception (exactly once)!
    return 0;
}
//! [normal version with args and thread support]