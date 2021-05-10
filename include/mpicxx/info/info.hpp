/**
 * @file
 * @author Marcel Breyer
 * @date 2020-08-04
 * @copyright This file is distributed under the MIT License.
 *
 * @brief Implements a wrapper class around the [*MPI_Info*](https://www.mpi-forum.org/docs/mpi-3.1/mpi31-report/node229.htm) object.
 * @details The @ref mpicxx::info class interface is inspired by the
 *          [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) and
 *          [`std::map`](https://en.cppreference.com/w/cpp/container/map) interface.
 */

#ifndef MPICXX_INFO_HPP
#define MPICXX_INFO_HPP

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/detail/concepts.hpp>
#include <mpicxx/detail/conversion.hpp>
#include <mpicxx/info/iterator.hpp>
#include <mpicxx/info/proxy.hpp>

#include <fmt/format.h>
#include <mpi.h>

#include <cstddef>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <limits>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>


namespace mpicxx {

/// This class is a wrapper to the MPI_Info object providing a interface inspired by std::unordered_map and std::map.
class info {

 public:
  /**************************************************************************************************************/
  /**                                                member types                                               **/
  /**************************************************************************************************************/
  using key_type = std::string;
  using mapped_type = std::string;
  using value_type = std::pair<const key_type, mapped_type>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = impl::info_iterator<false>;
  using const_iterator = impl::info_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using proxy = impl::info_proxy;


  /**************************************************************************************************************/
  /**                                            static data member                                            **/
  /**************************************************************************************************************/
  static const info env;
  static const info null;


  /**************************************************************************************************************/
  /**                                       constructors and destructor                                        **/
  /**************************************************************************************************************/
  info() : is_freeable_{ true } {
    // initialize an empty info object
    MPI_Info_create(&info_);
  }

  info(const info& other) {
    if (other.info_ == MPI_INFO_NULL) {
      // copy an info object which refers to MPI_INFO_NULL
      info_ = MPI_INFO_NULL;
      is_freeable_ = other.is_freeable_;
    } else {
      // copy normal info object
      MPI_Info_dup(other.info_, &info_);
      is_freeable_ = true;
    }
  }

  info(info&& other) noexcept: info_{ std::exchange(other.info_, MPI_INFO_NULL) },
                               is_freeable_{ std::exchange(other.is_freeable_, false) } { }

  template <std::input_iterator InputIter>
  info(InputIter first, InputIter last) : info{ } {
    // default construct the info object via the default constructor
    // add all (key, value)-pairs
    this->insert_or_assign(first, last);
  }

  info(std::initializer_list<value_type> init) : info{ } {
    // default construct the info object via the default constructor
    // add all (key, value)-pairs
    this->insert_or_assign(init);
  }

  constexpr info(MPI_Info other, const bool is_freeable) noexcept: info_{ other }, is_freeable_{ is_freeable } {
    MPICXX_ASSERT_SANITY(!(other == MPI_INFO_NULL && is_freeable == true), "'MPI_INFO_NULL' shouldn't be marked as freeable!");
    MPICXX_ASSERT_SANITY(!(other == MPI_INFO_ENV && is_freeable == true), "'MPI_INFO_ENV' shouldn't be marked as freeable!");
  }

  ~info() {
    // destroy info object if marked as freeable
    if (is_freeable_) {
      MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_NULL, "Attempt to free a 'MPI_INFO_NULL' object!");
      MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_ENV, "Attempt to free a 'MPI_INFO_ENV' object!");

      MPI_Info_free(&info_);
    }
  }

  /**************************************************************************************************************/
  /**                                           assignment operators                                           **/
  /**************************************************************************************************************/
  info& operator=(const info& rhs) {
    info tmp{ rhs };
    this->swap(tmp);
    return *this;
  }

  info& operator=(info&& rhs) {
    MPICXX_ASSERT_SANITY(!this->identical(rhs), "Attempt to perform a \"self move assignment\"!");

    // delete current MPI_Info object if and only if it is marked as freeable
    if (is_freeable_) {
      MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_NULL, "Attempt to free a 'MPI_INFO_NULL' object!");
      MPICXX_ASSERT_PRECONDITION(info_ != MPI_INFO_ENV, "Attempt to free a 'MPI_INFO_ENV' object!");

      MPI_Info_free(&info_);
    }
    // transfer ownership
    info_ = std::exchange(rhs.info_, MPI_INFO_NULL);
    is_freeable_ = std::exchange(rhs.is_freeable_, false);

    return *this;
  }

  info& operator=(std::initializer_list<value_type> ilist) {
    info tmp{ ilist };
    this->swap(tmp);
    return *this;
  }

  /**************************************************************************************************************/
  /**                                                 iterators                                                **/
  /**************************************************************************************************************/
  [[nodiscard]]
  iterator begin() noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");

    return iterator{ info_, 0 };
  }

  [[nodiscard]]
  iterator end() noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");

    return iterator{ info_, static_cast<difference_type>(this->size()) };
  }

  [[nodiscard]]
  const_iterator begin() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return const_iterator{ info_, 0 };
  }

  [[nodiscard]]
  const_iterator end() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return const_iterator{ info_, static_cast<difference_type>(this->size()) };
  }

  [[nodiscard]]
  const_iterator cbegin() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return const_iterator{ info_, 0 };
  }

  [[nodiscard]]
  const_iterator cend() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return const_iterator{ info_, static_cast<difference_type>(this->size()) };
  }

  [[nodiscard]]
  reverse_iterator rbegin() noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return std::make_reverse_iterator(this->end());
  }

  [[nodiscard]]
  reverse_iterator rend() noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return std::make_reverse_iterator(this->begin());
  }

  [[nodiscard]]
  const_reverse_iterator rbegin() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL!");

    return std::make_reverse_iterator(this->cend());
  }

  [[nodiscard]]
  const_reverse_iterator rend() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return std::make_reverse_iterator(this->cbegin());
  }

  [[nodiscard]]
  const_reverse_iterator crbegin() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return std::make_reverse_iterator(this->cend());
  }

  [[nodiscard]]
  const_reverse_iterator crend() const noexcept {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to create a const_reverse_iterator from an info object referring to 'MPI_INFO_NULL'!");

    return std::make_reverse_iterator(this->cbegin());
  }

  /**************************************************************************************************************/
  /**                                                 capacity                                                 **/
  /**************************************************************************************************************/
  [[nodiscard]]
  bool empty() const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    return this->size() == 0;
  }

  [[nodiscard]]
  size_type size() const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    int nkeys;
    MPI_Info_get_nkeys(info_, &nkeys);
    return static_cast<size_type>(nkeys);
  }

  [[nodiscard]]
  static constexpr size_type max_size() noexcept {
    return std::numeric_limits<difference_type>::max();
  }

  /**************************************************************************************************************/
  /**                                                 modifiers                                                **/
  /**************************************************************************************************************/
  template <detail::is_string T>
  proxy at(T&& key) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)",
                               detail::convert_to_string_size(key), MPI_MAX_INFO_KEY);

    // check whether the key exists
    if (!this->key_exists(key)) {
      // key doesn't exist
      throw std::out_of_range(fmt::format("{} doesn't exist!", std::forward<T>(key)));
    }
    // create proxy object and forward key
    return proxy{ info_, std::forward<decltype(key)>(key) };
  }

  std::string at(const std::string_view key) const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    // get the length of the value associated with key
    int valuelen, flag;
    MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
    // check whether the key exists
    if (!static_cast<bool>(flag)) {
      // key doesn't exist
      throw std::out_of_range{ fmt::format("{} doesn't exist!", std::forward<decltype(key)>(key)) };
    }
    // get the value associated with key
    std::string value(valuelen, ' ');
    MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
    return value;
  }

  template <detail::is_string T>
  proxy operator[](T&& key) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)",
                               detail::convert_to_string_size(key), MPI_MAX_INFO_KEY);

    // create proxy object and forward key
    return proxy{ info_, std::forward<T>(key) };
  }

  std::pair<iterator, bool> insert(const std::string_view key, const std::string_view value) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                               "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

    // check whether the key exists
    const bool key_already_exists = this->key_exists(key);
    if (!key_already_exists) {
      // key doesn't exist -> add new [key, value]-pair
      MPI_Info_set(info_, key.data(), value.data());
    }
    // search position of the key and return an iterator
    return std::make_pair(iterator(info_, this->find_pos(key, this->size())), !key_already_exists);
  }

  template <std::input_iterator InputIt>
  void insert(InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                               "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

    // try to insert every element in the range [first, last)
    for (; first != last; ++first) {
      // retrieve element
      const value_type& pair = *first;

      MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.first, MPI_MAX_INFO_KEY),
                                 "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", pair.first.size(), MPI_MAX_INFO_KEY);
      MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.second, MPI_MAX_INFO_VAL),
                                 "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", pair.second.size(), MPI_MAX_INFO_VAL);

      // check whether the key exists
      if (!this->key_exists(pair.first)) {
        // key doesn't exist -> add new [key, value]-pair
        MPI_Info_set(info_, pair.first.data(), pair.second.data());
      }
    }
  }

  void insert(std::initializer_list<value_type> ilist) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    this->insert(ilist.begin(), ilist.end());
  }

  std::pair<iterator, bool> insert_or_assign(const std::string_view key, const std::string_view value) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(value, MPI_MAX_INFO_VAL),
                               "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", value.size(), MPI_MAX_INFO_VAL);

    // check whether an insertion or assignment will take place
    const bool key_already_exists = this->key_exists(key);
    // updated (i.e. insert or assign) the [key, value]-pair
    MPI_Info_set(info_, key.data(), value.data());
    // search position of the key and return an iterator
    return std::make_pair(iterator(info_, this->find_pos(key, this->size())), !key_already_exists);
  }

  template <std::input_iterator InputIt>
  void insert_or_assign(InputIt first, InputIt last) requires (!detail::is_c_string<InputIt>) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_iterator_range(first, last),
                               "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

    // insert or assign every element in the range [first, last)
    for (; first != last; ++first) {
      // retrieve element
      const value_type& pair = *first;
      MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.first, MPI_MAX_INFO_KEY),
                                 "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", pair.first.size(), MPI_MAX_INFO_KEY);
      MPICXX_ASSERT_PRECONDITION(this->legal_string_size(pair.second, MPI_MAX_INFO_VAL),
                                 "Illegal info value: 0 < {} < {} (MPI_MAX_INFO_VAL)", pair.second.size(), MPI_MAX_INFO_VAL);

      // insert or assign [key, value]-pair
      MPI_Info_set(info_, pair.first.data(), pair.second.data());
    }
  }

  void insert_or_assign(std::initializer_list<value_type> ilist) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    this->insert_or_assign(ilist.begin(), ilist.end());
  }

  void clear() {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    const size_type size = this->size();
    char key[MPI_MAX_INFO_KEY];
    // repeat nkeys times and always remove the first element
    for (size_type i = 0; i < size; ++i) {
      MPI_Info_get_nthkey(info_, 0, key);
      MPI_Info_delete(info_, key);
    }
  }

  iterator erase(const_iterator pos) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(pos), "Attempt to use an info iterator referring to another info object!");
    MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(pos), "Attempt to dereference a {} iterator!", pos.state());

    char key[MPI_MAX_INFO_KEY];
    MPI_Info_get_nthkey(info_, pos.pos_, key);
    MPI_Info_delete(info_, key);
    return iterator(info_, pos.pos_);
  }

  iterator erase(const_iterator first, const_iterator last) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(first),
                               "Attempt to use an info iterator ('first') referring to another info object!");
    MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(last),
                               "Attempt to use an info iterator ('last') referring to another info object!");
    MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(first),
                               "Attempt to dereference a {} iterator ('first')!", first.state());
    MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(last),
                               "Attempt to dereference a {} iterator ('last')!", last.state());
    MPICXX_ASSERT_SANITY(this->legal_iterator_range(first, last),
                         "Attempt to pass an illegal iterator range ('first' must be less or equal than 'last')!");

    const difference_type count = last - first;
    char key[MPI_MAX_INFO_KEY];
    std::vector<std::string> keys_to_delete(count);

    // save all keys in the range [first, last)
    for (difference_type i = 0; i < count; ++i) {
      MPI_Info_get_nthkey(info_, first.pos_ + i, key);
      keys_to_delete[i] = key;
    }

    // delete all saved [key, value]-pairs
    for (const auto& str : keys_to_delete) {
      MPI_Info_delete(info_, str.data());
    }

    return iterator(info_, first.pos_);
  }

  size_type erase(const std::string_view key) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    // check whether the key exists
    if (this->key_exists(key)) {
      // key exists -> delete the [key, value]-pair
      MPI_Info_delete(info_, key.data());
      return 1;
    }
    return 0;
  }

  void swap(info& other) noexcept {
    using std::swap;
    swap(info_, other.info_);
    swap(is_freeable_, other.is_freeable_);
  }

  [[nodiscard]]
  value_type extract(const_iterator pos) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_info_iterator(pos), "Attempt to use an info iterator referring to another info object!");
    MPICXX_ASSERT_PRECONDITION(this->info_iterator_valid(pos), "Attempt to dereference a {} iterator!", pos.state());

    // get [key, value]-pair pointed to by pos
    const value_type& pair = *pos;
    // remove [key, value]-pair from info object
    MPI_Info_delete(info_, pair.first.data());
    // return extracted [key, value]-pair
    return pair;
  }

  [[nodiscard]]
  std::optional<value_type> extract(const std::string_view key) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    // check whether the key exists
    int valuelen, flag;
    MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
    if (static_cast<bool>(flag)) {
      // key exists -> delete the [key, value]-pair and return an iterator
      // get the value associated with the given key
      std::string value(valuelen, ' ');
      MPI_Info_get(info_, key.data(), valuelen, value.data(), &flag);
      // delete the [key, value]-pair from the info object
      MPI_Info_delete(info_, key.data());
      // return the extracted [key, value]-pair
      return std::make_optional<value_type>(std::make_pair(std::string(key), std::move(value)));
    }
    return std::nullopt;
  }

  void merge(info& source) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object ('*this') referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(!source.refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object ('source') referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_SANITY(!this->identical(source), "Attempt to perform a \"self merge\"!");

    // do nothing if a "self merge" is attempted
    if (this == std::addressof(source)) return;

    size_type size = source.size();
    char source_key[MPI_MAX_INFO_KEY];
    std::vector<std::string> keys_to_delete;

    // loop as long as there is at least one [key, value]-pair not visited yet
    for (size_type i = 0; i < size; ++i) {
      // get source_key
      MPI_Info_get_nthkey(source.info_, i, source_key);

      // check if source_key already exists in *this
      if (!this->key_exists(source_key)) {
        // get the value associated with source_key
        int valuelen, flag;
        MPI_Info_get_valuelen(source.info_, source_key, &valuelen, &flag);
        auto source_value = std::make_unique<char[]>(valuelen + 1);
        MPI_Info_get(source.info_, source_key, valuelen, source_value.get(), &flag);
        // remember the source's key
        keys_to_delete.emplace_back(source_key);
        // add [key, value]-pair to *this info object
        MPI_Info_set(info_, source_key, source_value.get());
      }
    }

    // delete all [key, value]-pairs merged into *this info object from source
    for (const auto& str : keys_to_delete) {
      MPI_Info_delete(source.info_, str.data());
    }
  }


  /**************************************************************************************************************/
  /**                                                  lookup                                                  **/
  /**************************************************************************************************************/
  [[nodiscard]]
  size_type count(const std::string_view key) const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    return static_cast<size_type>(this->contains(key));
  }

  [[nodiscard]]
  iterator find(const std::string_view key) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    const size_type size = this->size();
    return iterator(info_, this->find_pos(key, size));
  }

  [[nodiscard]]
  const_iterator find(const std::string_view key) const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    const size_type size = this->size();
    return const_iterator(info_, this->find_pos(key, size));
  }

  [[nodiscard]]
  bool contains(const std::string_view key) const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    const size_type size = this->size();
    return this->find_pos(key, size) != size;
  }

  [[nodiscard]]
  std::pair<iterator, iterator> equal_range(const std::string_view key) {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    const size_type size = this->size();
    const size_type pos = this->find_pos(key, size);
    if (pos != size) {
      // found key in the info object
      return std::make_pair(iterator(info_, pos), iterator(info_, pos + 1));
    } else {
      // the key is not in the info object
      return std::make_pair(iterator(info_, size), iterator(info_, size));
    }
  }

  [[nodiscard]]
  std::pair<const_iterator, const_iterator> equal_range(const std::string_view key) const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_PRECONDITION(this->legal_string_size(key, MPI_MAX_INFO_KEY),
                               "Illegal info key: 0 < {} < {} (MPI_MAX_INFO_KEY)", key.size(), MPI_MAX_INFO_KEY);

    const size_type size = this->size();
    const size_type pos = this->find_pos(key, size);
    if (pos != size) {
      // found key in the info object
      return std::make_pair(const_iterator(info_, pos), const_iterator(info_, pos + 1));
    } else {
      // the key is not in the info object
      return std::make_pair(const_iterator(info_, size), const_iterator(info_, size));
    }
  }


  /**************************************************************************************************************/
  /**                                           non-member functions                                           **/
  /**************************************************************************************************************/
  [[nodiscard]]
  friend bool operator==(const info& lhs, const info& rhs) {
    // if both info object refer to MPI_INFO_NULL they compare equal
    if (lhs.info_ == MPI_INFO_NULL && rhs.info_ == MPI_INFO_NULL) return true;
    // if only one info object refers to MPI_INFO_NULL they don't compare equal
    if (lhs.info_ == MPI_INFO_NULL || rhs.info_ == MPI_INFO_NULL) return false;

    // not the same number of [key, value]-pairs therefore can't compare equal
    const size_type size = lhs.size();
    if (size != rhs.size()) return false;

    // check all [key, value]-pairs for equality
    char key[MPI_MAX_INFO_KEY];
    for (size_type i = 0; i < size; ++i) {
      // retrieve key
      MPI_Info_get_nthkey(lhs.info_, i, key);

      // check if rhs contains the current key
      int valuelen, flag;
      MPI_Info_get_valuelen(rhs.info_, key, &valuelen, &flag);
      if (!static_cast<bool>(flag)) {
        // rhs does not contain the currently inspected lhs key -> info objects can't compare equal
        return false;
      }

      // both info objects contain the same key -> check for the respective values
      int lhs_valuelen;
      MPI_Info_get_valuelen(lhs.info_, key, &lhs_valuelen, &flag);
      if (valuelen != lhs_valuelen) {
        // both values have different lengths -> different values -> info objects can't compare equal
        return false;
      }

      // allocate a buffer for each value with the correct length
      auto lhs_value = std::make_unique<char[]>(valuelen + 1);
      auto rhs_value = std::make_unique<char[]>(valuelen + 1);
      // retrieve values
      MPI_Info_get(lhs.info_, key, valuelen, lhs_value.get(), &flag);
      MPI_Info_get(rhs.info_, key, valuelen, rhs_value.get(), &flag);
      // check if the values are equal
      const bool are_values_equal = std::strcmp(lhs_value.get(), rhs_value.get()) == 0;
      if (!are_values_equal) {
        // values compare inequal -> info objects can't compare equal
        return false;
      }
    }

    // all elements are equal
    return true;
  }

  friend void swap(info& lhs, info& rhs) noexcept { lhs.swap(rhs); }

  template <typename Pred>
  friend void erase_if(info& c, Pred pred) requires std::is_invocable_r_v<bool, Pred, value_type> {
    MPICXX_ASSERT_PRECONDITION(!c.refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object ('c') referring to 'MPI_INFO_NULL'!");

    size_type size = c.size();
    char key[MPI_MAX_INFO_KEY];

    std::vector<std::string> keys_to_delete;

    // loop through all [key, value]-pairs
    for (info::size_type i = 0; i < size; ++i) {
      // get key
      MPI_Info_get_nthkey(c.info_, i, key);
      // get value associated with key
      int valuelen, flag;
      MPI_Info_get_valuelen(c.info_, key, &valuelen, &flag);
      std::string value(valuelen, ' ');
      MPI_Info_get(c.info_, key, valuelen, value.data(), &flag);
      // create [key, value]-pair as a std::pair
      const value_type& pair = std::make_pair(std::string(key), std::move(value));

      // check whether the predicate holds
      if (std::invoke(pred, pair)) {
        // the predicate evaluates to true -> remember key for erasure
        keys_to_delete.emplace_back(std::move(pair.first));
      }
    }

    // delete all [key, value]-pairs for which the predicate returns true
    for (const auto& str : keys_to_delete) {
      MPI_Info_delete(c.info_, str.data());
    }
  }


  /**************************************************************************************************************/
  /**                                           additional functions                                           **/
  /**************************************************************************************************************/
  [[nodiscard]]
  std::vector<key_type> keys() const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    // create vector which will hold all keys
    const size_type size = this->size();
    std::vector<key_type> keys(size);
    char key[MPI_MAX_INFO_KEY];

    for (size_type i = 0; i < size; ++i) {
      // get key and add it to the vector
      MPI_Info_get_nthkey(info_, i, key);
      keys[i] = key;
    }

    return keys;
  }

  [[nodiscard]]
  std::vector<mapped_type> values() const {
    MPICXX_ASSERT_PRECONDITION(!this->refers_to_mpi_info_null(),
                               "Attempt to call a function on an info object referring to 'MPI_INFO_NULL'!");

    // create vector which will hold all values
    const size_type size = this->size();
    std::vector<mapped_type> values(size);
    char key[MPI_MAX_INFO_KEY];

    for (size_type i = 0; i < size; ++i) {
      // get key
      MPI_Info_get_nthkey(info_, i, key);
      // get value associated with key and add it to the vector
      int valuelen, flag;
      MPI_Info_get_valuelen(info_, key, &valuelen, &flag);
      std::string value(valuelen, ' ');
      MPI_Info_get(info_, key, valuelen, value.data(), &flag);
      values[i] = std::move(value);
    }

    return values;
  }

  [[nodiscard]]
  static constexpr size_type max_key_size() { return static_cast<size_type>(MPI_MAX_INFO_KEY); }

  [[nodiscard]]
  static constexpr size_type max_value_size() { return static_cast<size_type>(MPI_MAX_INFO_VAL); }


  /**************************************************************************************************************/
  /**                                                  getter                                                  **/
  /**************************************************************************************************************/
  [[nodiscard]]
  const MPI_Info& get() const noexcept { return info_; }

  [[nodiscard]]
  MPI_Info& get() noexcept { return info_; }

  [[nodiscard]]
  bool freeable() const noexcept { return is_freeable_; }

 private:
  [[nodiscard]]
  size_type find_pos(const std::string_view key, const size_type size) const {
    char info_key[MPI_MAX_INFO_KEY];
    // loop until a matching key is found
    for (size_type i = 0; i < size; ++i) {
      MPI_Info_get_nthkey(info_, i, info_key);
      // found equal key
      if (key.compare(info_key) == 0) {
        return i;
      }
    }
    // no matching key found
    return size;
  }

  [[nodiscard]]
  bool key_exists(const std::string_view key) const {
    int valuelen, flag;
    MPI_Info_get_valuelen(info_, key.data(), &valuelen, &flag);
    return static_cast<bool>(flag);
  }

#if MPICXX_ASSERTION_LEVEL > 0

  [[nodiscard]]
  bool refers_to_mpi_info_null() const {
    return info_ == MPI_INFO_NULL;
  }

  [[nodiscard]]
  bool identical(const info& other) const {
    return this == std::addressof(other);
  }

  [[nodiscard]]
  static bool legal_string_size(const std::string_view val, const int max_size) {
    return !val.empty() && val.size() < static_cast<std::size_t>(max_size);
  }

  template <std::input_iterator InputIt>
  [[nodiscard]]
  bool legal_iterator_range(InputIt first, InputIt last) {
    return std::distance(first, last) >= 0;
  }

  [[nodiscard]]
  bool legal_info_iterator(const_iterator it) const {
    return !it.singular() && it.info_ == std::addressof(info_);
  }

  [[nodiscard]]
  bool info_iterator_valid(const_iterator it) const {
    return this->legal_info_iterator(it) && 0 <= it.pos_ && it.pos_ <= static_cast<const_iterator::difference_type>(this->size());
  }

#endif

  MPI_Info info_;
  bool is_freeable_;
};

// initialize static environment object
inline const info info::env = info(MPI_INFO_ENV, false);

// initialize static null object
inline const info info::null = info(MPI_INFO_NULL, false);

}

#endif // MPICXX_INFO_HPP