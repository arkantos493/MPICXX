#ifndef MPICXX_THREAD_SUPPORT_HPP
#define MPICXX_THREAD_SUPPORT_HPP

#include <string_view>

#include <mpi.h>
#include <fmt/format.h>


namespace mpicxx {

    enum class thread_support {
        SINGLE = MPI_THREAD_SINGLE,
        FUNNELED = MPI_THREAD_FUNNELED,
        SERIALIZED = MPI_THREAD_SERIALIZED,
        MULTIPLE = MPI_THREAD_MULTIPLE
    };

}

template <>
struct fmt::formatter<mpicxx::thread_support> : fmt::formatter<string_view> {
    template <typename FormatContext>
    auto format(const mpicxx::thread_support ts, FormatContext& ctx) {
        string_view name;
        switch (ts) {
            case mpicxx::thread_support::SINGLE:
                name = "MPI_THREAD_SINGLE";
                break;
            case mpicxx::thread_support::FUNNELED:
                name = "MPI_THREAD_FUNNELED";
                break;
            case mpicxx::thread_support::SERIALIZED:
                name = "MPI_THREAD_SERIALIZED";
                break;
            case mpicxx::thread_support::MULTIPLE:
                name = "MPI_THREAD_MULTIPLE";
                break;
        }
        return fmt::formatter<string_view>::format(name, ctx);
    }
};

#endif // MPICXX_THREAD_SUPPORT_HPP
