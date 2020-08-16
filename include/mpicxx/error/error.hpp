/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-16
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Defines error codes and error classes including standard once.
 */

#ifndef MPICXX_ERROR_HPP
#define MPICXX_ERROR_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <mpi.h>

#include <functional>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>

namespace mpicxx {

    // forward declaration of the error_category class
    class error_category;

    /**
     * @nosubgrouping
     * @brief This class represents an error code returned by calls to various MPI functions.
     */
    class error_code {
    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                             static data member                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name predefined MPI error codes
        ///@{
        /// No Error.
        static const error_code success;
        /// Invalid buffer pointer.
        static const error_code buffer;
        /// Invalid count argument.
        static const error_code count;
        /// Invalid datatype argument.
        static const error_code type;
        /// Invalid tag argument.
        static const error_code tag;
        /// Invalid communicator.
        static const error_code comm;
        /// Invalid rank.
        static const error_code rank;
        /// Invalid request (handle).
        static const error_code request;
        /// Invalid root.
        static const error_code root;
        /// Invalid group.
        static const error_code group;
        /// Invalid operation.
        static const error_code op;
        /// Invalid topology.
        static const error_code topology;
        /// Invalid dimension argument.
        static const error_code dims;
        /// Invalid argument of some other kind.
        static const error_code arg;
        /// Unknown error.
        static const error_code unknown;
        /// Message truncated on receive.
        static const error_code truncate;
        /// Known error not in this list.
        static const error_code other;
        /// Internal MPI (implementation) error.
        static const error_code intern;
        /// Error code is in status.
        static const error_code in_status;
        /// Pending request.
        static const error_code pending;
        /// Invalid keyval has been passed.
        static const error_code keyval;
        /// [*MPI_ALLOC_MEM*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node216.htm) failed because memory is exhausted.
        static const error_code no_mem;
        /// Invalid base passed to [*MPI_FREE_MEM*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node216.htm).
        static const error_code base;
        /// Key longer than [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
        static const error_code info_key;
        /// Value longer than [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
        static const error_code info_value;
        /// Invalid key passed to [*MPI_INFO_DELETE*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
        static const error_code info_nokey;
        /// Error in spawning processes.
        static const error_code spawn;
        /// Invalid port name passed to [*MPI_COMM_CONNECT*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node245.htm).
        static const error_code port;
        /// Invalid service name passed to [*MPI_UNPUBLISH_NAME*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node246.htm).
        static const error_code service;
        /// Invalid service name passed to [*MPI_LOOKUP_NAME*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node246.htm).
        static const error_code name;
        /// Invalid `win` argument.
        static const error_code win;
        /// Invalid `size` argument.
        static const error_code size;
        /// Invalid `disp` argument.
        static const error_code disp;
        /// Invalid `info` argument.
        static const error_code info;
        /// Invalid `locktype` argument.
        static const error_code locktype;
        /// Invalid `assert` argument.
        static const error_code assert;
        /// Conflicting accesses to window.
        static const error_code rma_conflict;
        /// Wrong synchronization of RMA (Remote Memory Access) calls.
        static const error_code rma_sync;
        /// Target memory is not part of the window (in the case of a window created with
        /// [*MPI_WIN_CREATE_DYNAMIC*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node264.htm), target memory is not attached).
        static const error_code rma_range;
        /// Memory cannot be attached (e.g., because of resource exhaustion).
        static const error_code rma_attach;
        /// Memory cannot be shared (e.g., some process in the group of the specified communicator cannot expose shared memory).
        static const error_code rma_shared;
        /// Passed window has the wrong flavor for the called function
        static const error_code rma_flavor;
        /// Invalid file handle.
        static const error_code file;
        /// Collective argument not identical on all processes, or collective routines called in a different order by different processes.
        static const error_code not_same;
        /// Error related to the amode passed to [*MPI_FILE_OPEN*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node308.htm).
        static const error_code amode;
        /// Unsupported datarep passed to [*MPI_FILE_SET_VIEW*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node317.htm).
        static const error_code unsupported_datarep;
        /// Unsupported operation, such as seeking on a file which supports sequential access only.
        static const error_code unsupported_operation;
        /// File does not exist.
        static const error_code no_such_file;
        /// File exists.
        static const error_code file_exits;
        /// Invalid file name (e.g., path name too long).
        static const error_code bad_file;
        /// Permission denied.
        static const error_code access;
        /// Not enough space.
        static const error_code no_space;
        /// Quota exceeded.
        static const error_code quota;
        /// Read-only file or file system.
        static const error_code read_only;
        /// File operation could not be completed, as the file is currently open by some process.
        static const error_code file_in_use;
        /// Conversion functions could not be registered because a data representation identifier that was already defined was passed
        /// to [*MPI_REGISTER_DATAREP*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node334.htm).
        static const error_code dup_datarep;
        /// An error occurred in a user supplied data conversion function.
        static const error_code conversion;
        /// Other I/O error.
        static const error_code io;
        /// Last error code.
        static const error_code lastcode;
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                constructor                                                 //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Constructs a new error code with the value given by @p code.
         * @param[in] code the error code value (default: [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm))
         *
         * @pre @p code **must** not be less than 0 or greater than the last used error code (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_sanity{ If @p code isn't a valid error code value. }
         */
        error_code(const int code = MPI_SUCCESS) noexcept : code_(code) {
#if MPICXX_ASSERTION_LEVEL > 0
            int initialized;
            MPI_Initialized(&initialized);
            if (static_cast<bool>(initialized)) {
                // if the MPI environment has been initialized use the (dynamic) MPI_LASTUSEDCODE attribute
                MPICXX_ASSERT_SANITY(this->valid_error_code(code),
                        "Attempt to create an error code with invalid value ({})! "
                        "Valid error code values must be in the interval [{}, {}].",
                        code, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));
            } else {
                // if the MPI environment hasn't been initialized use the (stat) MPI_ERR_LASTCODE
                MPICXX_ASSERT_SANITY(this->valid_error_code(code, MPI_ERR_LASTCODE),
                        "Attempt to create an error code with invalid value ({})! "
                        "Valid error code values must be in the interval [{}, {}].",
                        code, MPI_SUCCESS, MPI_ERR_LASTCODE);
            }
#endif
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 modifiers                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify error code
        ///@{
        /**
         * @brief Assign the new error code value @p code to the current one.
         * @param[in] code the new error code value
         *
         * @pre @p code **must** not be less than 0 or greater than the last used error code (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_sanity{ If @p code isn't a valid error code value. }
         */
        void assign(const int code) noexcept {
            MPICXX_ASSERT_SANITY(this->valid_error_code(code),
                    "Attempt to assign an error code with invalid value ({})! "
                    "Valid error code values must be in the interval [{}, {}].",
                    code, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));

            code_ = code;
        }
        /**
         * @brief Replaces the error code with the default value
         *        [*MPI_SUCCESS*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node222.htm).
         */
        void clear() noexcept { code_ = MPI_SUCCESS; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  observer                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name observe error code
        ///@{
        /**
         * @brief Returns the value of the error code.
         * @return the error code value
         * @nodiscard
         */
        [[nodiscard]]
        constexpr int value() const noexcept { return code_; }
        /**
         * @brief Returns the value of the last used error code.
         * @details The returned value will **not** change unless a function to add an error class or an error category is called.
         * @return the last error code value
         * @nodiscard
         *
         * @note One can **not** assume that **all** values below the returned value are valid.
         *
         * @calls{ int MPI_Comm_get_attr(MPI_Comm comm, int comm_keyval, void *attribute_val, int *flag);    // exactly once }
         */
        [[nodiscard]]
        static const std::optional<int> last_used_value() {
            void* ptr;
            int flag;
            MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_LASTUSEDCODE, &ptr, &flag);
            if (static_cast<bool>(flag)) {
                return std::make_optional(*reinterpret_cast<int*>(ptr));
            } else {
                return std::nullopt;
            }
        }
        /**
         * @brief Returns the @ref mpicxx::error_category of the error code value.
         * @return the @ref mpicxx::error_category
         * @nodiscard
         *
         * @pre The current error code value **must** not be less than 0 or greater than the last used error code
         *      (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_precondition{ If the current error code value isn't a valid value. }
         *
         * @calls{ int MPI_Error_class(int errorcode, int *errorclass);    // exactly once }
         */
        [[nodiscard]]
        error_category category() const;
        /**
         * @brief Returns the error string associated with the error code value.
         * @return the error string
         * @nodiscard
         * 
         * @pre The current error code value **must** not be less than 0 or greater than the last used error code
         *      (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_precondition{ If the current error code value isn't a valid value. }
         *
         * @calls{ int MPI_Error_string(int errorcode, char *string, int *resultlen);    // exactly once }
         */
        [[nodiscard]]
        std::string message() const {
            MPICXX_ASSERT_PRECONDITION(this->valid_error_code(code_),
                    "Attempt to retrieve the error string of an error code with invalid value ({})! "
                    "Valid error code values must be in the interval [{}, {}].",
                    code_, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));

            char error_string[MPI_MAX_ERROR_STRING];
            int resultlen;
            MPI_Error_string(code_, error_string, &resultlen);
            return std::string(error_string, resultlen);
        }
        /**
         * @brief Returns the maximum possible error string size.
         * @return the maximum error string size
         *         (= [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm))
         * @nodiscard
         */
        [[nodiscard]]
        static constexpr std::size_t max_message_size() { return static_cast<std::size_t>(MPI_MAX_ERROR_STRING); }
        /**
         * @brief Check if the error code value is valid, i.e. non-zero.
         * @return `false` if `value() == MPI_SUCCESS`, `true` otherwise
         * @nodiscard
         */
        [[nodiscard]]
        operator bool() const noexcept { return code_ != MPI_SUCCESS; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name non-member functions
        ///@{
        /**
         * @brief [Three-way comparison operator](https://en.cppreference.com/w/cpp/language/default_comparisons) for two
         *        @ref mpicxx::error_code @p lhs and @p rhs.
         * @details Automatically generates `%mpicxx::error_code::operator<`, `%mpicxx::error_code::operator<=`,
         *          `%mpicxx::error_code::operator>` and `%mpicxx::error_code::operator>=`.
         * @param[in] lhs an @ref mpicxx::error_code
         * @param[in] rhs an @ref mpicxx::error_code
         * @return the [`std::strong_ordering`](https://en.cppreference.com/w/cpp/utility/compare/strong_ordering) result
         * @nodiscard
         */
        [[nodiscard]]
        friend std::strong_ordering operator<=>(const error_code lhs, const error_code rhs) = default;
        /**
         * @brief Stream-insertion operator overload for the @ref mpicxx::error_code class.
         * @details Outputs the error code value **and** the associated error string.
         * @param[inout] out an output stream
         * @param[in] ec the @ref mpicxx::error_code
         * @return the output stream
         *
         * @pre The current error code value **must** not be less than 0 or greater than the last used error code
         *      (@ref mpicxx::error_code::last_used_value()).
         *
         * @assert_precondition{ If the current error code value isn't a valid value. }
         *
         * @calls{ int MPI_Error_string(int errorcode, char *string, int *resultlen);    // exactly once }
         */
        friend std::ostream& operator<<(std::ostream& out, const error_code ec) {
            return out << ec.value() << ": " << ec.message();
        }
        ///@}


    private:
#if MPICXX_ASSERTION_LEVEL > 0
        /*
         * @brief Checks whether @p code is a valid error code value, i.e. @p code is not less than 0 and not greater than the last used
         *        error code value (mpicxx::error_code::last_used_value()).
         * @param[in] code the error code value to check
         * @return `true` if @p code is a valid error code value, otherwise `false`
         */
        bool valid_error_code(const int code) const {
            return MPI_SUCCESS <= code && code <= error_code::last_used_value().value_or(std::numeric_limits<int>::max());
        }
        /*
         * @brief Checks whether @p code is a valid error code value, i.e. @p code is not less than 0 and not greater than the last used
         *        error code value @p last_code.
         * @param[in] code the error code value to check
         * @param[in] last_code the last used error code value
         * @return `true` if @p code is a valid error code value, otherwise `false`
         */
        bool valid_error_code(const int code, const int last_code) const {
            return MPI_SUCCESS <= code && code <= last_code;
        }
#endif

        int code_;
    };

    // initialize predefined MPI error codes
    inline const error_code error_code::success = error_code(MPI_SUCCESS);
    inline const error_code error_code::buffer = error_code(MPI_ERR_BUFFER);
    inline const error_code error_code::count = error_code(MPI_ERR_COUNT);
    inline const error_code error_code::type = error_code(MPI_ERR_TYPE);
    inline const error_code error_code::tag = error_code(MPI_ERR_TAG);
    inline const error_code error_code::comm = error_code(MPI_ERR_COMM);
    inline const error_code error_code::rank = error_code(MPI_ERR_RANK);
    inline const error_code error_code::request = error_code(MPI_ERR_REQUEST);
    inline const error_code error_code::root = error_code(MPI_ERR_ROOT);
    inline const error_code error_code::group = error_code(MPI_ERR_GROUP);
    inline const error_code error_code::op = error_code(MPI_ERR_OP);
    inline const error_code error_code::topology = error_code(MPI_ERR_TOPOLOGY);
    inline const error_code error_code::dims = error_code(MPI_ERR_DIMS);
    inline const error_code error_code::arg = error_code(MPI_ERR_ARG);
    inline const error_code error_code::unknown = error_code(MPI_ERR_UNKNOWN);
    inline const error_code error_code::truncate = error_code(MPI_ERR_TRUNCATE);
    inline const error_code error_code::other = error_code(MPI_ERR_OTHER);
    inline const error_code error_code::intern = error_code(MPI_ERR_INTERN);
    inline const error_code error_code::in_status = error_code(MPI_ERR_IN_STATUS);
    inline const error_code error_code::pending = error_code(MPI_ERR_PENDING);
    inline const error_code error_code::keyval = error_code(MPI_ERR_KEYVAL);
    inline const error_code error_code::no_mem = error_code(MPI_ERR_NO_MEM);
    inline const error_code error_code::base = error_code(MPI_ERR_BASE);
    inline const error_code error_code::info_key = error_code(MPI_ERR_INFO_KEY);
    inline const error_code error_code::info_value = error_code(MPI_ERR_INFO_VALUE);
    inline const error_code error_code::info_nokey = error_code(MPI_ERR_INFO_NOKEY);
    inline const error_code error_code::spawn = error_code(MPI_ERR_SPAWN);
    inline const error_code error_code::port = error_code(MPI_ERR_PORT);
    inline const error_code error_code::service = error_code(MPI_ERR_SERVICE);
    inline const error_code error_code::name = error_code(MPI_ERR_NAME);
    inline const error_code error_code::win = error_code(MPI_ERR_WIN);
    inline const error_code error_code::size = error_code(MPI_ERR_SIZE);
    inline const error_code error_code::disp = error_code(MPI_ERR_DISP);
    inline const error_code error_code::info = error_code(MPI_ERR_INFO);
    inline const error_code error_code::locktype = error_code(MPI_ERR_LOCKTYPE);
    inline const error_code error_code::assert = error_code(MPI_ERR_ASSERT);
    inline const error_code error_code::rma_conflict = error_code(MPI_ERR_RMA_CONFLICT);
    inline const error_code error_code::rma_sync = error_code(MPI_ERR_RMA_CONFLICT);
    inline const error_code error_code::rma_range = error_code(MPI_ERR_RMA_RANGE);
    inline const error_code error_code::rma_attach = error_code(MPI_ERR_RMA_ATTACH);
    inline const error_code error_code::rma_shared = error_code(MPI_ERR_RMA_SHARED);
    inline const error_code error_code::rma_flavor = error_code(MPI_ERR_RMA_FLAVOR);
    inline const error_code error_code::file = error_code(MPI_ERR_FILE);
    inline const error_code error_code::not_same = error_code(MPI_ERR_NOT_SAME);
    inline const error_code error_code::amode = error_code(MPI_ERR_AMODE);
    inline const error_code error_code::unsupported_datarep = error_code(MPI_ERR_UNSUPPORTED_DATAREP);
    inline const error_code error_code::unsupported_operation = error_code(MPI_ERR_UNSUPPORTED_OPERATION);
    inline const error_code error_code::no_such_file = error_code(MPI_ERR_NO_SUCH_FILE);
    inline const error_code error_code::file_exits = error_code(MPI_ERR_FILE_EXISTS);
    inline const error_code error_code::bad_file = error_code(MPI_ERR_BAD_FILE);
    inline const error_code error_code::access = error_code(MPI_ERR_ACCESS);
    inline const error_code error_code::no_space = error_code(MPI_ERR_NO_SPACE);
    inline const error_code error_code::quota = error_code(MPI_ERR_QUOTA);
    inline const error_code error_code::read_only = error_code(MPI_ERR_READ_ONLY);
    inline const error_code error_code::file_in_use = error_code(MPI_ERR_FILE_IN_USE);
    inline const error_code error_code::dup_datarep = error_code(MPI_ERR_DUP_DATAREP);
    inline const error_code error_code::conversion = error_code(MPI_ERR_CONVERSION);
    inline const error_code error_code::io = error_code(MPI_ERR_IO);
    inline const error_code error_code::lastcode = error_code(MPI_ERR_LASTCODE);

    
    /**
     * @nosubgrouping
     * @brief This class represents an error category containing possibly multiple @ref error_code.
     * @details This class is marked `[[nodiscard]]` since the default constructor creates a new error category.
     *          Discarding this newly constructed @ref mpicxx::error_category wouldn't allow adding of new @ref mpicxx::error_code which
     *          would render the respective @ref mpicxx::error_category useless.
     */
    class [[nodiscard]] error_category {
        // Befriend error_code class to be able to create an error_category in mpicxx::error_code::category
        friend class error_code;

        /*
         * @brief Constructs an error category with the value given by @p category.
         * @default **Doesn't** create a new error category, but refers to an already existing one.
         *
         *    This constructor is `private` and is only used in the `mpicxx::error_code::category()` function.
         * @param[in] category the value of the error category
         */
        explicit constexpr error_category(const int category) noexcept : category_(category) { }

    public:
        // ---------------------------------------------------------------------------------------------------------- //
        //                                             static data member                                             //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name predefined MPI error categories
        ///@{
        /// No Error.
        static const error_category success;
        /// Invalid buffer pointer.
        static const error_category buffer;
        /// Invalid count argument.
        static const error_category count;
        /// Invalid datatype argument.
        static const error_category type;
        /// Invalid tag argument.
        static const error_category tag;
        /// Invalid communicator.
        static const error_category comm;
        /// Invalid rank.
        static const error_category rank;
        /// Invalid request (handle).
        static const error_category request;
        /// Invalid root.
        static const error_category root;
        /// Invalid group.
        static const error_category group;
        /// Invalid operation.
        static const error_category op;
        /// Invalid topology.
        static const error_category topology;
        /// Invalid dimension argument.
        static const error_category dims;
        /// Invalid argument of some other kind.
        static const error_category arg;
        /// Unknown error.
        static const error_category unknown;
        /// Message truncated on receive.
        static const error_category truncate;
        /// Known error not in this list.
        static const error_category other;
        /// Internal MPI (implementation) error.
        static const error_category intern;
        /// Error code is in status.
        static const error_category in_status;
        /// Pending request.
        static const error_category pending;
        /// Invalid keyval has been passed.
        static const error_category keyval;
        /// [*MPI_ALLOC_MEM*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node216.htm) failed because memory is exhausted.
        static const error_category no_mem;
        /// Invalid base passed to [*MPI_FREE_MEM*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node216.htm).
        static const error_category base;
        /// Key longer than [*MPI_MAX_INFO_KEY*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
        static const error_category info_key;
        /// Value longer than [*MPI_MAX_INFO_VAL*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
        static const error_category info_value;
        /// Invalid key passed to [*MPI_INFO_DELETE*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm).
        static const error_category info_nokey;
        /// Error in spawning processes.
        static const error_category spawn;
        /// Invalid port name passed to [*MPI_COMM_CONNECT*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node245.htm).
        static const error_category port;
        /// Invalid service name passed to [*MPI_UNPUBLISH_NAME*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node246.htm).
        static const error_category service;
        /// Invalid service name passed to [*MPI_LOOKUP_NAME*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node246.htm).
        static const error_category name;
        /// Invalid `win` argument.
        static const error_category win;
        /// Invalid `size` argument.
        static const error_category size;
        /// Invalid `disp` argument.
        static const error_category disp;
        /// Invalid `info` argument.
        static const error_category info;
        /// Invalid `locktype` argument.
        static const error_category locktype;
        /// Invalid `assert` argument.
        static const error_category assert;
        /// Conflicting accesses to window.
        static const error_category rma_conflict;
        /// Wrong synchronization of RMA (Remote Memory Access) calls.
        static const error_category rma_sync;
        /// Target memory is not part of the window (in the case of a window created with
        /// [*MPI_WIN_CREATE_DYNAMIC*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node264.htm), target memory is not attached).
        static const error_category rma_range;
        /// Memory cannot be attached (e.g., because of resource exhaustion).
        static const error_category rma_attach;
        /// Memory cannot be shared (e.g., some process in the group of the specified communicator cannot expose shared memory).
        static const error_category rma_shared;
        /// Passed window has the wrong flavor for the called function
        static const error_category rma_flavor;
        /// Invalid file handle.
        static const error_category file;
        /// Collective argument not identical on all processes, or collective routines called in a different order by different processes.
        static const error_category not_same;
        /// Error related to the amode passed to [*MPI_FILE_OPEN*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node308.htm).
        static const error_category amode;
        /// Unsupported datarep passed to [*MPI_FILE_SET_VIEW*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node317.htm).
        static const error_category unsupported_datarep;
        /// Unsupported operation, such as seeking on a file which supports sequential access only.
        static const error_category unsupported_operation;
        /// File does not exist.
        static const error_category no_such_file;
        /// File exists.
        static const error_category file_exits;
        /// Invalid file name (e.g., path name too long).
        static const error_category bad_file;
        /// Permission denied.
        static const error_category access;
        /// Not enough space.
        static const error_category no_space;
        /// Quota exceeded.
        static const error_category quota;
        /// Read-only file or file system.
        static const error_category read_only;
        /// File operation could not be completed, as the file is currently open by some process.
        static const error_category file_in_use;
        /// Conversion functions could not be registered because a data representation identifier that was already defined was passed
        /// to [*MPI_REGISTER_DATAREP*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node334.htm).
        static const error_category dup_datarep;
        /// An error occurred in a user supplied data conversion function.
        static const error_category conversion;
        /// Other I/O error.
        static const error_category io;
        /// Last error code.
        static const error_category lastcode;
        ///@}
        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 constructor                                                //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name constructor
        ///@{
        /**
         * @brief Constructs a new error category.
         *
         * @calls{ int MPI_Add_error_class(int *errorclass);    // exactly once }
         */
        error_category() noexcept {
            MPI_Add_error_class(&category_);
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                 modifiers                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name modify error category
        ///@{
        /**
         * @brief Constructs a new @ref mpicxx::error_code with @p error_string as error description associated with this
         *        @ref mpicxx::error_category.
         * @param[in] error_string the description of the new error code
         * @return the newly created @ref mpicxx::error_code
         *
         * @pre The current error category value **must** not be less than 0.
         * @pre @p error_string **must** include the null-terminator.
         * @pre The length of @p error_string **must** be less than
         *      [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm)
         *
         * @assert_precondition{ If the current error category value isn't a valid value..\n
         *                       If @p error_string exceeds its size limit. }
         *
         * @calls{
         * int MPI_Add_error_code(int errorclass, int *errorcode);         // exactly once
         * int MPI_Add_error_string(int errorcode, const char *string);    // exactly once
         * }
         */
        error_code add_error_code(const std::string_view error_string) const {
            MPICXX_ASSERT_PRECONDITION(this->valid_error_category(category_),
                    "Attempt to use an error category with invalid value ({})! "
                    "Valid error category values must be greater or equal than {}.",
                    category_, MPI_SUCCESS);
            MPICXX_ASSERT_PRECONDITION(this->legal_error_string(error_string),
                    "Illegal error string: {} < {} (MPI_MAX_ERROR_STRING)",
                    error_string.size(), MPI_MAX_ERROR_STRING);

            int new_error_code;
            MPI_Add_error_code(category_, &new_error_code);
            MPI_Add_error_string(new_error_code, error_string.data());
            return error_code(new_error_code);
        }
        /**
         * @brief Constructs [`std::distance(first, last)`](https://en.cppreference.com/w/cpp/iterator/distance) new @ref mpicxx::error_code
         *        with the respective error description in the range [@p first, @p last) associated with this @ref mpicxx::error_category.
         * @tparam InputIt must meet the [LegacyInputIterator](https://en.cppreference.com/w/cpp/named_req/InputIterator) requirements
         * @param[in] first iterator to the first error description in the range
         * @param[in] last iterator one-past the last error description in the range
         * @return all newly created @ref mpicxx::error_code
         *
         * @pre The current error category value **must** not be less than 0.
         * @pre @p first and @p last **must** refer to the same container.
         * @pre @p first and @p last **must** form a valid range, i.e. @p first must be less or equal than @p last.
         * @pre All error strings **must** include the null-terminator.
         * @pre The length of all error strings **must** be less than
         *      [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm)
         *
         * @assert_precondition{ If the current error category value isn't a valid value. \n
         *                       If @p first and @p last don't denote a valid range. \n
         *                       If any error string exceeds its size limit. }
         *
         * @calls{
         * int MPI_Add_error_code(int errorclass, int *errorcode);         // exactly 'last - first' times
         * int MPI_Add_error_string(int errorcode, const char *string);    // exactly 'last - first' times
         * }
         */
        template <std::input_iterator InputIt>
        std::vector<error_code> add_error_code(InputIt first, InputIt last) const requires (!detail::is_c_string<InputIt>) {
            MPICXX_ASSERT_PRECONDITION(this->valid_error_category(category_),
                    "Attempt to use an error category with invalid value ({})! "
                    "Valid error category values must be greater or equal than {}.",
                    category_, MPI_SUCCESS);
            MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                    "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");
            
            std::vector<error_code> new_error_codes;
            new_error_codes.reserve(std::distance(first, last));
            for (; first != last; ++first) {
                new_error_codes.emplace_back(this->add_error_code(*first));
            }
            return new_error_codes;
        }
        /**
         * @brief Constructs `ilist.begin()` new @ref mpicxx::error_code with the respective error description in the
         *        [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) @p ilist associated with this
         *        @ref mpicxx::error_category.
         * @param ilist [`std::initializer_list`](https://en.cppreference.com/w/cpp/utility/initializer_list) containing the error
         *              descriptions
         * @return all newly created @ref mpicxx::error_code
         *
         * @pre The current error category value **must** not be less than 0.
         * @pre All error strings **must** include the null-terminator.
         * @pre The length of all error strings **must** be less than
         *      [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm)
         *
         * @assert_precondition{ If the current error category value isn't a valid value..\n
         *                       If any error string exceeds its size limit. }
         *
         * @calls{
         * int MPI_Add_error_code(int errorclass, int *errorcode);         // exactly 'ilist.size()' times
         * int MPI_Add_error_string(int errorcode, const char *string);    // exactly 'ilist.size()' times
         * }
         */
        template <detail::is_string T>
        std::vector<error_code> add_error_code(std::initializer_list<T> ilist) const {
            MPICXX_ASSERT_PRECONDITION(this->valid_error_category(category_),
                    "Attempt to use an error category with invalid value ({})! "
                    "Valid error category values must be greater or equal than {}.",
                    category_, MPI_SUCCESS);

            return this->add_error_code(ilist.begin(), ilist.end());
        }
        /**
         * @brief Constructs `sizeof...(T)` new @ref mpicxx::error_code with the respective error description in the
         *        parameter pack @p args associated with this @ref mpicxx::error_category.
         * @tparam T must meed the @ref mpicxx::detail::is_string requirements and must not greater than 1
         * @param args an arbitrary number (but at least 2) of error descriptions
         * @return all newly created @ref mpicxx::error_code
         *
         * @pre The current error category value **must** not be less than 0.
         * @pre All error strings **must** include the null-terminator.
         * @pre The length of all error strings **must** be less than
         *      [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm)
         *
         * @assert_precondition{ If the current error category value isn't a valid value..\n
         *                       If any error string exceeds its size limit. }
         *
         * @calls{
         * int MPI_Add_error_code(int errorclass, int *errorcode);         // exactly 'sizeof...(T)' times
         * int MPI_Add_error_string(int errorcode, const char *string);    // exactly 'sizeof...(T)' times
         * }
         */
        template <detail::is_string... T>
        std::vector<error_code> add_error_code(T&&... args) const requires (sizeof...(T) > 1) {
            MPICXX_ASSERT_PRECONDITION(this->valid_error_category(category_),
                    "Attempt to use an error category with invalid value ({})! "
                    "Valid error category values must be greater or equal than {}.",
                    category_, MPI_SUCCESS);

            std::vector<error_code> new_error_codes;
            new_error_codes.reserve(sizeof...(T));
            ([&](auto&& arg) {
                new_error_codes.emplace_back(this->add_error_code(std::forward<decltype(arg)>(arg)));
            }(std::forward<T>(args)), ...);
            return new_error_codes;
        }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                                  observer                                                  //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name observe error category
        ///@{
        /**
         * @brief Returns the value of the error category.
         * @return the error category value
         * @nodiscard
         */
        [[nodiscard]]
        constexpr int value() const noexcept { return category_; }
        ///@}


        // ---------------------------------------------------------------------------------------------------------- //
        //                                            non-member functions                                            //
        // ---------------------------------------------------------------------------------------------------------- //
        /// @name non-member functions
        ///@{
        /**
         * @brief [Three-way comparison operator](https://en.cppreference.com/w/cpp/language/default_comparisons) for two
         *        @ref mpicxx::error_category @p lhs and @p rhs.
         * @details Automatically generates `%mpicxx::error_category::operator<`, `%mpicxx::error_category::operator<=`,
         *          `%mpicxx::error_category::operator>` and `%mpicxx::error_category::operator>=`.
         * @param[in] lhs an @ref mpicxx::error_category
         * @param[in] rhs an @ref mpicxx::error_category
         * @return the [`std::strong_ordering`](https://en.cppreference.com/w/cpp/utility/compare/strong_ordering) result
         * @nodiscard
         */
        friend std::strong_ordering operator<=>(const error_category lhs, const error_category rhs) = default;
        /**
         * @brief Stream-insertion operator overload for the @ref mpicxx::error_category class.
         * @details Outputs the error category value.
         * @param[inout] out an output stream
         * @param[in] ec the @ref mpicxx::error_category
         * @return the output stream
         */
        friend std::ostream& operator<<(std::ostream& out, const error_category ec) {
            return out << ec.value();
        }
        ///@}


    private:
#if MPICXX_ASSERTION_LEVEL > 0
        /*
         * @brief Checks whether @p category is a valid error category value, i.e. @p category is not less than 0.
         * @param[in] category the error category value to check
         * @return `true` if @p code is a valid error code value, otherwise `false`
         */
        bool valid_error_category(const int category) const {
            return MPI_SUCCESS <= category;
        }
        /*
         * @brief Checks whether the error string @p str is legal, i.e. its size is less than
         *        [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm).
         * @param[in] str the error string to check the size
         * @return `true` if the size of @p str less than
         *         [*MPI_MAX_ERROR_STRING*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node221.htm), otherwise `false`
         */
        bool legal_error_string(const std::string_view str) const {
            return str.size() < MPI_MAX_ERROR_STRING;
        }
        /*
         * @brief Check whether @p first and @p last denote a valid range, i.e. @p first is less or equal than @p last.
         * @details Checks whether the distance bewteen @p first and @p last is not negative.
         * @param[in] first iterator to the first element
         * @param[in] last past-the-end iterator
         * @return `true` if @p first and @p last denote a valid iterator range, otherwise `false`
         */
        template <std::input_iterator InputIt>
        bool legal_iterator_range(InputIt first, InputIt last) const {
            return std::distance(first, last) >= 0;
        }
#endif

        int category_;
    };

    // initialize predefined MPI error codes
    inline const error_category error_category::success = error_category(MPI_SUCCESS);
    inline const error_category error_category::buffer = error_category(MPI_ERR_BUFFER);
    inline const error_category error_category::count = error_category(MPI_ERR_COUNT);
    inline const error_category error_category::type = error_category(MPI_ERR_TYPE);
    inline const error_category error_category::tag = error_category(MPI_ERR_TAG);
    inline const error_category error_category::comm = error_category(MPI_ERR_COMM);
    inline const error_category error_category::rank = error_category(MPI_ERR_RANK);
    inline const error_category error_category::request = error_category(MPI_ERR_REQUEST);
    inline const error_category error_category::root = error_category(MPI_ERR_ROOT);
    inline const error_category error_category::group = error_category(MPI_ERR_GROUP);
    inline const error_category error_category::op = error_category(MPI_ERR_OP);
    inline const error_category error_category::topology = error_category(MPI_ERR_TOPOLOGY);
    inline const error_category error_category::dims = error_category(MPI_ERR_DIMS);
    inline const error_category error_category::arg = error_category(MPI_ERR_ARG);
    inline const error_category error_category::unknown = error_category(MPI_ERR_UNKNOWN);
    inline const error_category error_category::truncate = error_category(MPI_ERR_TRUNCATE);
    inline const error_category error_category::other = error_category(MPI_ERR_OTHER);
    inline const error_category error_category::intern = error_category(MPI_ERR_INTERN);
    inline const error_category error_category::in_status = error_category(MPI_ERR_IN_STATUS);
    inline const error_category error_category::pending = error_category(MPI_ERR_PENDING);
    inline const error_category error_category::keyval = error_category(MPI_ERR_KEYVAL);
    inline const error_category error_category::no_mem = error_category(MPI_ERR_NO_MEM);
    inline const error_category error_category::base = error_category(MPI_ERR_BASE);
    inline const error_category error_category::info_key = error_category(MPI_ERR_INFO_KEY);
    inline const error_category error_category::info_value = error_category(MPI_ERR_INFO_VALUE);
    inline const error_category error_category::info_nokey = error_category(MPI_ERR_INFO_NOKEY);
    inline const error_category error_category::spawn = error_category(MPI_ERR_SPAWN);
    inline const error_category error_category::port = error_category(MPI_ERR_PORT);
    inline const error_category error_category::service = error_category(MPI_ERR_SERVICE);
    inline const error_category error_category::name = error_category(MPI_ERR_NAME);
    inline const error_category error_category::win = error_category(MPI_ERR_WIN);
    inline const error_category error_category::size = error_category(MPI_ERR_SIZE);
    inline const error_category error_category::disp = error_category(MPI_ERR_DISP);
    inline const error_category error_category::info = error_category(MPI_ERR_INFO);
    inline const error_category error_category::locktype = error_category(MPI_ERR_LOCKTYPE);
    inline const error_category error_category::assert = error_category(MPI_ERR_ASSERT);
    inline const error_category error_category::rma_conflict = error_category(MPI_ERR_RMA_CONFLICT);
    inline const error_category error_category::rma_sync = error_category(MPI_ERR_RMA_CONFLICT);
    inline const error_category error_category::rma_range = error_category(MPI_ERR_RMA_RANGE);
    inline const error_category error_category::rma_attach = error_category(MPI_ERR_RMA_ATTACH);
    inline const error_category error_category::rma_shared = error_category(MPI_ERR_RMA_SHARED);
    inline const error_category error_category::rma_flavor = error_category(MPI_ERR_RMA_FLAVOR);
    inline const error_category error_category::file = error_category(MPI_ERR_FILE);
    inline const error_category error_category::not_same = error_category(MPI_ERR_NOT_SAME);
    inline const error_category error_category::amode = error_category(MPI_ERR_AMODE);
    inline const error_category error_category::unsupported_datarep = error_category(MPI_ERR_UNSUPPORTED_DATAREP);
    inline const error_category error_category::unsupported_operation = error_category(MPI_ERR_UNSUPPORTED_OPERATION);
    inline const error_category error_category::no_such_file = error_category(MPI_ERR_NO_SUCH_FILE);
    inline const error_category error_category::file_exits = error_category(MPI_ERR_FILE_EXISTS);
    inline const error_category error_category::bad_file = error_category(MPI_ERR_BAD_FILE);
    inline const error_category error_category::access = error_category(MPI_ERR_ACCESS);
    inline const error_category error_category::no_space = error_category(MPI_ERR_NO_SPACE);
    inline const error_category error_category::quota = error_category(MPI_ERR_QUOTA);
    inline const error_category error_category::read_only = error_category(MPI_ERR_READ_ONLY);
    inline const error_category error_category::file_in_use = error_category(MPI_ERR_FILE_IN_USE);
    inline const error_category error_category::dup_datarep = error_category(MPI_ERR_DUP_DATAREP);
    inline const error_category error_category::conversion = error_category(MPI_ERR_CONVERSION);
    inline const error_category error_category::io = error_category(MPI_ERR_IO);
    inline const error_category error_category::lastcode = error_category(MPI_ERR_LASTCODE);


    // Implement mpicxx::error_code::category() function.
    [[nodiscard]]
    inline error_category error_code::category() const {
        MPICXX_ASSERT_PRECONDITION(this->valid_error_code(code_),
                "Attempt to retrieve the error string of an error code with invalid value ({})! "
                "Valid error code values must be in the interval [{}, {}].",
                code_, MPI_SUCCESS, error_code::last_used_value().value_or(std::numeric_limits<int>::max()));

        int category;
        MPI_Error_class(code_, &category);
        return error_category(category);
    }

}

namespace std {

    /**
     * @brief Specializes the [`std::hash`](https://en.cppreference.com/w/cpp/utility/hash) template to be able to hash a
     *        @ref mpicxx::error_code and therefore to be able to directly use a @ref mpicxx::error_code in a e.g. a
     *        [`std::unordered_set`](https://en.cppreference.com/w/cpp/container/unordered_set) or
     *        [std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map).
     * @details It hashes the error code value using the default [`std::hash<int>`](https://en.cppreference.com/w/cpp/utility/hash).
     * @return the hash value
     */
    template <>
    struct hash<mpicxx::error_code> {
        std::size_t operator()(mpicxx::error_code er) const noexcept {
            return std::hash<int>{}(er.value());
        }
    };

}

#endif // MPICXX_ERROR_HPP