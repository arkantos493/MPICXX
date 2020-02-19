#ifndef MPICXX_MPICXX_MAIN_HPP
#define MPICXX_MPICXX_MAIN_HPP

#include <mpicxx/startup/initialization.hpp>
#include <mpicxx/startup/finalization.hpp>
#include <mpicxx/detail/concepts.hpp>

#include <functional>
#include <iostream>


namespace mpicxx {

    template <detail::main_pointer FuncPtr>
    int init(FuncPtr ptr) {
        initialize();

        int ret = std::invoke(ptr);

        finalize();
        return ret;
    }

    template <detail::main_args_pointer FuncPtr>
    int init(FuncPtr ptr, int& argc, char** argv) {
        initialize(argc, argv);

        int ret = std::invoke(ptr, argc, argv);

        finalize();
        return ret;
    }

    template <detail::main_pointer FuncPtr>
    int init(FuncPtr ptr, const thread_support ts) {
        int ret = -1;
        try {
            initialize(ts);
            ret = std::invoke(ptr);
        } catch (const mpicxx::thread_support_not_satisfied& e) {
            std::cerr << e.what() << std::endl;
        }

        finalize();
        return ret;
    }

    template <detail::main_args_pointer FuncPtr>
    int init(FuncPtr ptr, int& argc, char** argv, const thread_support ts) {
        int ret = -1;
        try {
            initialize(argc, argv, ts);
            ret = std::invoke(ptr, argc, argv);
        } catch (const mpicxx::thread_support_not_satisfied& e) {
            std::cerr << e.what() << std::endl;
        }

        finalize();
        return ret;
    }

}


#endif // MPICXX_MPICXX_MAIN_HPP
