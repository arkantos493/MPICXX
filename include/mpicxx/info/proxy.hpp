/**
 * 2021-05-09: Marcel Breyer
 * This file is distributed under the MIT License.
 *
 * Implements a proxy class for the mpicxx::info class.
 */

#ifndef MPICXX_INCLUDE_MPICXX_INFO_PROXY_HPP_
#define MPICXX_INCLUDE_MPICXX_INFO_PROXY_HPP_

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>

#include <mpi.h>

#include <memory>
#include <string_view>
#include <string>

namespace mpicxx::impl {

/**************************************************************************************************************/
/**                                   proxy class for read/write accesses                                    **/
/**************************************************************************************************************/
class info_proxy {
  using MPI_Info_ptr = MPI_Info*;
  using MPI_Info_ref = MPI_Info&;
 public:
  template <detail::is_string T>
  info_proxy(MPI_Info_ref info, T&& key) : info_{ std::addressof(info) }, key_{ std::forward<T>(key) } {
    MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null(),
                         "Attempt to create a proxy from an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_SANITY(this->legal_string_size(key_, MPI_MAX_INFO_KEY),
                         "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key_.size(), MPI_MAX_INFO_KEY);
  }

  void operator=(const std::string_view value) {
    MPICXX_ASSERT_PRECONDITION(!this->info_refers_to_mpi_info_null(),
                               "Attempt to access a [key, value]-pair of an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                               "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

    MPI_Info_set(*info_, key_.data(), value.data());
  }

  [[nodiscard]]
  operator std::string() const {
    MPICXX_ASSERT_PRECONDITION(!this->info_refers_to_mpi_info_null(),
                               "Attempt to access a [key, value]-pair of an info object referring to 'MPI_INFO_NULL'!");

    // get the length of the value
    int valuelen, flag;
    MPI_Info_get_valuelen(*info_, key_.data(), &valuelen, &flag);

    if (!static_cast<bool>(flag)) {
      // the key doesn't exist yet
      // -> add a new [key, value]-pair and return a std::string consisting of only one whitespace
      std::string value{ " " };
      MPI_Info_set(*info_, key_.data(), value.data());
      return value;
    }

    // key exists -> get the associated value
    std::string value(valuelen, ' ');
    MPI_Info_get(*info_, key_.data(), valuelen, value.data(), &flag);
    return value;
  }

  friend std::ostream& operator<<(std::ostream& out, const info_proxy& rhs) {
    MPICXX_ASSERT_PRECONDITION(!rhs.info_refers_to_mpi_info_null(),
                               "Attempt to access a [key, value]-pair of an info object referring to 'MPI_INFO_NULL'!");

    out << static_cast<std::string>(rhs);
    return out;
  }

 private:
#if MPICXX_ASSERTION_LEVEL > 0

  [[nodiscard]] bool info_refers_to_mpi_info_null() const {
    return *info_ == MPI_INFO_NULL;
  }

  [[nodiscard]] static bool legal_string_size(const std::string_view val, const int max_size) {
    return !val.empty() && static_cast<int>(val.size()) < max_size;
  }

#endif

  MPI_Info_ptr info_;
  const std::string key_;
};

}

#endif //MPICXX_INCLUDE_MPICXX_INFO_PROXY_HPP_
