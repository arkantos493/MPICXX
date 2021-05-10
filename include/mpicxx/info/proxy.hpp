/**
 * Copyright (C) 2021-05-10 - Marcel Breyer - All Rights Reserved
 * Licensed under the MIT License. See LICENSE.md file in the project root for full license information.
 *
 * Implements a proxy class for the mpicxx::info class.
 */

#ifndef MPICXX_INCLUDE_MPICXX_INFO_PROXY_HPP_
#define MPICXX_INCLUDE_MPICXX_INFO_PROXY_HPP_

#include <mpicxx/detail/assert.hpp>

#include <mpi.h>

#include <memory>
#include <string>
#include <string_view>

namespace mpicxx::impl {

/**
 * @brief A proxy class for the mpicxx::info class to distinguish between read and write accesses for the element access functions.
 */
class info_proxy {
  using MPI_Info_ptr = MPI_Info*;
  using MPI_Info_ref = MPI_Info&;
 public:
  /**
   * @brief Construct a new proxy object referring to a specific MPI_Info object and key.
   * @param info the referred to MPI_Info object
   * @param key the provided key
   */
  info_proxy(MPI_Info_ref info, std::string key) : info_{ std::addressof(info) }, key_{ std::move(key) } {
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null(),
                  "Attempt to create a proxy from an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT(this->legal_string_size(key_, MPI_MAX_INFO_KEY),
                  "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key_.size(), MPI_MAX_INFO_KEY);
  }

  /**
   * @brief On write access, add the provided value and saved key to the info object.
   * @details Creates a new (key, value)-pair if the key doesn't already exist, otherwise overwrites the existing value.
   * @param[in] value the value associated with the saved key
   */
  void operator=(const std::string_view value) {
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null(),
                  "Attempt to access a (key, value)-pair of an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                  "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

    MPI_Info_set(*info_, key_.data(), value.data());
  }

  /**
   * @brief On read access, return the value associated with the saved key.
   * @details If the key doesn't exist yet, it will be inserted with a string consisting only of one whitespace as value,
   *          also returning a std::string(" ").
   * @return the value associated with the saved key
   */
  [[nodiscard]] operator std::string() const {
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null(),
                  "Attempt to access a (key, value)-pair of an info object referring to 'MPI_INFO_NULL'!");

    // get the length of the value
    int valuelen, flag;
    MPI_Info_get_valuelen(*info_, key_.data(), &valuelen, &flag);

    if (!static_cast<bool>(flag)) {
      // the key doesn't exist yet
      // -> add a new (key, value)-pair and return a std::string consisting of only one whitespace
      std::string value{ " " };
      MPI_Info_set(*info_, key_.data(), value.data());
      return value;
    } else {
      // key exists -> get the associated value
      std::string value(valuelen, ' ');
      MPI_Info_get(*info_, key_.data(), valuelen, value.data(), &flag);
      return value;
    }
  }

  /**
   * @brief Convenience overload to be able to directly print a proxy object.
   * @details Calls info_proxy::operator std::string() const to get the value that should be printed,
   *          i.e. if the key doesn't exist yet, a new (key, value)-pair will be inserted into the info object
   * @param[inout] out the output stream to write on
   * @param[in] rhs the proxy object
   * @return the output stream
   */
  friend std::ostream& operator<<(std::ostream& out, const info_proxy& rhs) {
    MPICXX_ASSERT(!rhs.info_refers_to_mpi_info_null(),
                  "Attempt to access a (key, value)-pair of an info object referring to 'MPI_INFO_NULL'!");

    return out << static_cast<std::string>(rhs);
  }

 private:
#ifdef MPICXX_ENABLE_ASSERTIONS
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

#endif // MPICXX_INCLUDE_MPICXX_INFO_PROXY_HPP_
