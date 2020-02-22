/**
 * @file include/mpicxx/version/version.hpp
 * @author Marcel Breyer
 * @date 2020-02-21
 *
 * @brief Implements functions to query the current mpicxx and MPI version.
 */

#ifndef MPICXX_VERSION_HPP
#define MPICXX_VERSION_HPP

#include <string_view>

#include <mpi.h>


#define STRINGIFY_STR(X) #X
#define STRINGIFY(X) STRINGIFY_STR(X)


namespace mpicxx::version {

    /// @name version details specific to the mpicxx library
    ///@{
    /**
     * @brief The name of the mpicxx library.
     * @details The value gets automatically configured with the library name specified in the main cmake file.
     */
    constexpr std::string_view name = "mpicxx";
    /**
     * @brief The current version of the mpicxx library.
     * @details The value gets automatically configured with the library version specified in the main cmake file. \n
     * It has the form: "version_major.version_minor.version_patch".
     */
    constexpr std::string_view version = "0.3.0";
    /**
     * @brief The current major version of the mpicxx library.
     * @details The value gets automatically configured with the library major version specified in the main cmake file.
     */
    constexpr int version_major = 0;
    /**
     * @brief The current minor version of the mpicxx library.
     * @details The value gets automatically configured with the library minor version specified in the main cmake file.
     */
    constexpr int version_minor = 3;
    /**
     * @brief The current patch version of the mpicxx library.
     * @details The value gets automatically configured with the library patch version specified in the main cmake file.
     */
    constexpr int version_patch = 0;
    ///@}

    /// @name version details specific to the used MPI standard
    ///@{
    /**
     * @brief The current version of the used MPI standard.
     * @details The value gets automatically set via the used MPI library. \n
     * It has the form: "mpi_version_major.mpi_version_minor".
     */
    constexpr std::string_view mpi_version = STRINGIFY(MPI_VERSION) "." STRINGIFY(MPI_SUBVERSION);
    /**
     * @brief The current major version of the used MPI standard.
     * @details The value gets automatically set via the used MPI library.
     */
    constexpr int mpi_version_major = MPI_VERSION;
    /**
     * @brief The current minor version of the used MPI standard.
     * @details The value gets automatically set via the used MPI library.
     */
    constexpr int mpi_version_minor = MPI_SUBVERSION;
    ///@}

    /// @name version details specific to the used MPI library
    ///@{
    /**
     * @brief The current version of the used MPI library.
     * @details The value gets automatically set via the used MPI library.
     * @returns a library specific version string
     */
    std::string mpi_library_version() {
        char library_version[MPI_MAX_LIBRARY_VERSION_STRING];
        int length;
        MPI_Get_library_version(library_version, &length);
        return std::string(library_version, length);
    }
    /**
     * @brief The name of the used MPI library.
     * @details The value is one of: "Open MPI", "MPICH", or "unknown.
     * @return the name of the used MPI library
     */
    std::string mpi_library_name() {
        std::string library_version = mpi_library_version();
        if (library_version.find("Open MPI") != std::string::npos) {
            return "Open MPI";
        } else if (library_version.find("MPICH") != std::string::npos) {
            return "MPICH";
        } else {
            return "unknown";
        }
    }
    ///@}

}


#endif // MPICXX_VERSION_HPP