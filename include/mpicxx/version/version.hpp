/**
 * @file include/mpicxx/version/version.hpp
 * @author Marcel Breyer
 * @date 2020-06-22
 *
 * @brief Implements functions to query the current mpicxx and MPI version.
 */

#ifndef MPICXX_VERSION_HPP
#define MPICXX_VERSION_HPP

#include <string>
#include <string_view>

#include <mpi.h>
#include <fmt/format.h>


namespace mpicxx::version {

    /// @name version details specific to the mpicxx library
    ///@{
    /**
     * @brief The name of the mpicxx library.
     * @details The value gets automatically set during the [`CMake`](https://cmake.org/) configuration step.
     */
    inline constexpr std::string_view name = "mpicxx";
    /**
     * @brief The current version of the mpicxx library.
     * @details The value gets automatically set during the [`CMake`](https://cmake.org/) configuration step.
     *
     *          It's of the form: "version_major.version_minor.version_patch".
     */
    inline constexpr std::string_view version = "0.3.1";
    /**
     * @brief The current major version of the mpicxx library.
     * @details The value gets automatically set during the [`CMake`](https://cmake.org/) configuration step.
     */
    inline constexpr int version_major = 0;
    /**
     * @brief The current minor version of the mpicxx library.
     * @details The value gets automatically set during the [`CMake`](https://cmake.org/) configuration step.
     */
    inline constexpr int version_minor = 3;
    /**
     * @brief The current patch version of the mpicxx library.
     * @details The value gets automatically set during the [`CMake`](https://cmake.org/) configuration step.
     */
    inline constexpr int version_patch = 1;
    ///@}

    /// @name version details specific to the used MPI standard
    ///@{
    namespace detail {
        /*
         * @brief The current version of the used MPI standard.
         * @return a pair containing the major and minor MPI standard version
         *
         * @calls{ int MPI_Get_version(int *version, int *subversion);      // exactly once }
         */
        inline std::pair<int, int> get_mpi_version() {
            int version, subversion;
            MPI_Get_version(&version, &subversion);
            return std::make_pair(version, subversion);
        }
    }
    /**
     * @brief The current version of the used MPI standard in the form "mpi_version_major.mpi_version_minor" (e.g. `"3.1"`).
     * @details This function can be called before @ref mpicxx::init() and after @ref mpicxx::finalize() and is thread safe as required by
     *          the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return the MPI standard version (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Get_version(int *version, int *subversion);      // exactly twice }
     */
    [[nodiscard]]
    inline std::string mpi_version() {
        return fmt::format("{}.{}", detail::get_mpi_version().first, detail::get_mpi_version().second);
    }
    /**
     * @brief The current major version of the used MPI standard.
     * @details This function can be called before @ref mpicxx::init() and after @ref mpicxx::finalize() and is thread safe as required by
     *          the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return the MPI standard major version (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Get_version(int *version, int *subversion);      // exactly once }
     */
    [[nodiscard]]
    inline int mpi_version_major() {
        return detail::get_mpi_version().first;
    }
    /**
     * @brief The current minor version (subversion) of the used MPI standard.
     * @details This function can be called before @ref mpicxx::init() and after @ref mpicxx::finalize() and is thread safe as required by
     *          the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return the MPI standard minor version (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Get_version(int *version, int *subversion);      // exactly once }
     */
    [[nodiscard]]
    inline int mpi_version_minor() {
        return detail::get_mpi_version().second;
    }
    ///@}

    /// @name version details specific to the used MPI library
    ///@{
    /**
     * @brief The current version of the used MPI library (library specific implementation defined).
     * @details This function can be called before @ref mpicxx::init() and after @ref mpicxx::finalize() and is thread safe as required by
     *          the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return a library specific version string (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Get_library_version(char *version, int *resultlen);      // exactly once }
     */
    [[nodiscard]]
    inline std::string mpi_library_version() {
        char library_version[MPI_MAX_LIBRARY_VERSION_STRING];
        int resultlen;
        MPI_Get_library_version(library_version, &resultlen);
        return std::string(library_version, resultlen);
    }
    /**
     * @brief The name of the used MPI library.
     * @details The value is one of: `"Open MPI"`, `"MPICH"`, or `"unknown"`.
     *
     *          This function can be called before @ref mpicxx::init() and after @ref mpicxx::finalize() and is thread safe as required by
     *          the [MPI standard 3.1](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report.pdf).
     * @return the name of the used MPI library (`[[nodiscard]]`)
     *
     * @calls{ int MPI_Get_library_version(char *version, int *resultlen);      // exactly once }
     */
    [[nodiscard]]
    inline std::string mpi_library_name() {
        using namespace std::string_literals;
        std::string library_version = mpi_library_version();
        if (library_version.find("Open MPI"s) != std::string::npos) {
            return "Open MPI"s;
        } else if (library_version.find("MPICH"s) != std::string::npos) {
            return "MPICH"s;
        } else {
            return "unknown"s;
        }
    }
    ///@}

}


#endif // MPICXX_VERSION_HPP
