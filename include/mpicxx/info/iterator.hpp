/**
 * Copyright (C) 2021-05-10 - Marcel Breyer - All Rights Reserved
 * Licensed under the MIT License. See LICENSE.md file in the project root for full license information.
 *
 * Implements iterators for the mpicxx::info class.
 */

#ifndef MPICXX_INCLUDE_MPICXX_INFO_ITERATOR_HPP_
#define MPICXX_INCLUDE_MPICXX_INFO_ITERATOR_HPP_

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/info/proxy.hpp>

#include <compare>      // std::partial_ordering
#include <cstddef>      // std::ptrdiff_t
#include <iterator>     // std::random_access_iterator_tag
#include <memory>       // std::addressof
#include <string>       // std::string
#include <string_view>  // std::string_view
#include <type_traits>  // std::conditional_t
#include <utility>      // std::move, std::pair, std::make_pair

namespace mpicxx::impl {

/**
 * @brief Provides random access iterator and const_iterator for an info object.
 * @tparam is_const if true a const_iterator is instantiated, otherwise a non-const iterator
 */
template <bool is_const>
class info_iterator {
  // befriend all info_iterator specializations
  template <bool>
  friend
  class info_iterator;

  // info class can now directly access the pos member
  friend class mpicxx::info;

  // helper class returned by operator->
  template <typename T>
  class pointer_impl {
   public:
    explicit pointer_impl(T&& val) : val_{ std::move(val) } { }
    T* operator->() { return &val_; }

   private:
    T val_;
  };

  // pointer type to the referred to info object (pointer to const if is_const is true)
  using MPI_Info_ptr = std::conditional_t<is_const, const MPI_Info*, MPI_Info*>;
  // reference type to the referred to info object (reference to const if is_const is true)
  using MPI_Info_ref = std::conditional_t<is_const, const MPI_Info&, MPI_Info&>;

 public:
  /**************************************************************************************************************/
  /**                                        iterator_traits definitions                                       **/
  /**************************************************************************************************************/
  /**
   * @brief std::iterator_traits difference type to identify the distance between two iterators.
   */
  using difference_type = std::ptrdiff_t;
  /**
   * @brief std::iterator_traits value type that can be obtained by dereferencing the iterator.
   * @details n case of a non-const iterator, the value will be returned by a mpicxx::info::proxy object to allow changing its value.
   */
  using value_type = std::conditional_t<is_const,
                                        std::pair<const std::string, const std::string>,
                                        std::pair<const std::string, info_proxy>>;
  /**
   * @brief std::iterator_traits pointer type to the (key, value)-pair iterated over.
   */
  using pointer = pointer_impl<value_type>;
  /**
   * @brief std::iterator_traits reference type (not meaningful because mpicxx::impl::info_iterator::operator*() and
   *        mpicxx::impl::info_iterator::operator->() has to return by-value (using a proxy for write access)).
   */
  using reference = value_type;
  /**
   * @brief std::iterator_traits iterator category.
   */
  using iterator_category = std::random_access_iterator_tag;
  /**
   * @brief std::iterator_traits iterator concept (for C++20 concepts).
   */
  using iterator_concept = std::random_access_iterator_tag;


  /**************************************************************************************************************/
  /**                                               constructors                                               **/
  /**************************************************************************************************************/
  /**
   * @brief Default construct a singular iterator.
   */
  info_iterator() noexcept: info_{ nullptr }, pos_{ 0 } { }

  /**
   * @brief Construct a new iterator referring to the info object info at position pos.
   * @param[inout] info the referred to MPI_Info object
   * @param[in] pos the iterator's current position
   */
  info_iterator(MPI_Info_ref info, const difference_type pos) noexcept: info_{ std::addressof(info) }, pos_{ pos } {
    MPICXX_ASSERT(!this->singular(),
                  "Attempt to explicitly create a singular iterator!");
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null(),
                  "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT(pos_ >= 0 && pos <= this->info_size(),
                  "Attempt to create an iterator referring to {}, which falls outside its valid range!", pos);
  }

  /**
   * @brief Copy constructor. Constructs the info object with a copy of the contents of other.
   * @param[in] other iterator to use as data source
   */
  info_iterator(const info_iterator& other) noexcept: info_{ other.info_ }, pos_{ other.pos_ } {
    MPICXX_ASSERT(!other.singular() && !other.info_refers_to_mpi_info_null(),
                  "Attempt to create an iterator from a {} iterator{}!",
                  other.state(), other.info_state());
  }

  /**
   * @brief Special copy constructor. Convert a non-const iterator to a const_iterator.
   * @tparam is_const_
   * @param[in] other iterator to use as data source
   */
  template <bool is_const_ = is_const>
  requires is_const_
  info_iterator(const info_iterator<false>& other) noexcept : info_{ other.info_ }, pos_{ other.pos_ } {
    MPICXX_ASSERT(!other.singular() && !other.info_refers_to_mpi_info_null(),
                  "Attempt to create an iterator from a {} iterator{}!",
                  other.state(), other.info_state());
  }


  /**************************************************************************************************************/
  /**                                           assignment operators                                           **/
  /**************************************************************************************************************/
  /**
   * @brief Copy assignment operator. Replace contents with a copy of the contents of other.
   * @param[in] rhs iterator to use as data source
   * @return *this
   */
  info_iterator& operator=(const info_iterator& rhs) noexcept {
    MPICXX_ASSERT(!rhs.singular() && !rhs.info_refers_to_mpi_info_null(),
                  "Attempt to assign a {} iterator{} to a {} iterator{}!",
                  rhs.state(), rhs.info_state(), this->state(), this->info_state());

    info_ = rhs.info_;
    pos_ = rhs.pos_;
    return *this;
  }

  /**
   * @brief Special copy assignment operator. Assign a non-const iterator to a const_iterator.
   * @tparam is_const_
   * @param rhs  iterator to use as data source
   * @return *this
   */
  template <bool is_const_ = is_const>
  requires is_const_
  info_iterator& operator=(const info_iterator<false>& rhs) noexcept {
    MPICXX_ASSERT(!rhs.singular() && !rhs.info_refers_to_mpi_info_null(),
                  "Attempt to assign a {} iterator{} to a {} iterator{}!",
                  rhs.state(), rhs.info_state(), this->state(), this->info_state());

    info_ = rhs.info_;
    pos_ = rhs.pos_;
    return *this;
  }


  /**************************************************************************************************************/
  /**                                           relational operators                                           **/
  /**************************************************************************************************************/
  /**
   * @brief Compares *this and rhs for equality.
   * @tparam rhs_const
   * @param rhs the other iterator
   * @return true if both iterators are equal, otherwise false
   */
  template <bool rhs_const>
  [[nodiscard]] bool operator==(const info_iterator<rhs_const>& rhs) const {
    MPICXX_ASSERT(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!", this->state(), rhs.state());
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                  "Attempt to compare a {} iterator{} to a {} iterator{}!",
                  this->state(), this->info_state(), rhs.state(), rhs.info_state());
    MPICXX_ASSERT(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

    return info_ == rhs.info_ && pos_ == rhs.pos_;
  }

  /**
   * @brief Three-way comparison operator for *this and rhs.
   * @tparam rhs_const
   * @param rhs the other iterator
   * @return the std::partial_ordering result
   */
  template <bool rhs_const>
  [[nodiscard]] std::partial_ordering operator<=>(const info_iterator<rhs_const>& rhs) const {
    MPICXX_ASSERT(!this->singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!", this->state(), rhs.state());
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                  "Attempt to compare a {} iterator{} to a {} iterator{}!",
                  this->state(), this->info_state(), rhs.state(), rhs.info_state());
    MPICXX_ASSERT(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

    if (auto cmp = info_ <=> rhs.info_; cmp != 0) { return std::partial_ordering::unordered; }
    return pos_ <=> rhs.pos_;
  }


  /**************************************************************************************************************/
  /**                                                 modifiers                                                **/
  /**************************************************************************************************************/
  /**
   * @brief Move the iterator one position forward.
   * @return modified iterator pointing to the new position
   */
  info_iterator& operator++() {
    MPICXX_ASSERT(this->incrementable(), "Attempt to increment a {} iterator{}!", this->state(), this->info_state());

    ++pos_;
    return *this;
  }
  /**
   * @brief Move the iterator one position forward and return the old iterator.
   * @return iterator pointing to the old position
   */
  info_iterator operator++(int) {
    MPICXX_ASSERT(this->incrementable(), "Attempt to increment a {} iterator{}!", this->state(), this->info_state());

    info_iterator tmp{ *this };
    this->operator++();
    return tmp;
  }
  /**
   * @brief Move this iterator inc steps forward.
   * @param[in] inc number of steps (may be negative)
   * @return modified iterator pointing to the new position
   */
  info_iterator& operator+=(const difference_type inc) {
    MPICXX_ASSERT(this->advanceable(inc),
                  "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                  this->state(), this->info_state(), inc);

    pos_ += inc;
    return *this;
  }
  /**
   * @brief Move the iterator it inc steps forward.
   * @param[in] it the iterator to increment
   * @param[in] inc number of steps (may be negative)
   * @return new iterator pointing to the new position
   */
  [[nodiscard("Did you mean 'operator+='?")]]
  friend info_iterator operator+(info_iterator it, const difference_type inc) {
    MPICXX_ASSERT(it.advanceable(inc),
                  "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                  it.state(), it.info_state(), inc);

    it.pos_ += inc;
    return it;
  }
  /**
   * @brief Move the iterator it inc steps forward.
   * @param[in] inc number of steps (may be negative)
   * @param[in] it the iterator to increment
   * @return new iterator pointing to the new position
   */
  [[nodiscard("Did you mean 'operator+='?")]]
  friend info_iterator operator+(const difference_type inc, info_iterator it) {
    MPICXX_ASSERT(it.advanceable(inc),
                  "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                  it.state(), it.info_state(), inc);

    return it + inc;
  }

  /**
   * @brief Move the iterator one position backward.
   * @return modified iterator pointing to the new position.
   */
  info_iterator& operator--() {
    MPICXX_ASSERT(this->decrementable(), "Attempt to decrement a {} iterator{}!", this->state(), this->info_state());

    --pos_;
    return *this;
  }
  /**
   * @brief Move the iterator one position backward and return the old iterator
   * @return iterator pointing to the old position
   */
  info_iterator operator--(int) {
    MPICXX_ASSERT(this->decrementable(), "Attempt to decrement a {} iterator{}!", this->state(), this->info_state());

    info_iterator tmp{ *this };
    this->operator--();
    return tmp;
  }
  /**
   * @brief Move this iterator inc steps backward.
   * @param[in] inc number of steps (may be negative)
   * @return modified iterator pointing to the new position
   */
  info_iterator& operator-=(const difference_type inc) {
    MPICXX_ASSERT(this->advanceable(-inc),
                  "Attempt to retreat a {} iterator{} {} steps, which falls outside its valid range!",
                  this->state(), this->info_state(), inc);

    pos_ -= inc;
    return *this;
  }
  /**
   * @brief Move the iterator it inc steps backward.
   * @param[in] it the iterator to decrement
   * @param[in] inc number of steps (may be negative)
   * @return new iterator pointing to the new position
   */
  [[nodiscard("Did you mean 'operator-='?")]]
  friend info_iterator operator-(info_iterator it, const difference_type inc) {
    MPICXX_ASSERT(it.advanceable(-inc),
                  "Attempt to retreat a {} iterator{} {} steps, which falls outside its valid range!",
                  it.state(), it.info_state(), inc);

    it.pos_ -= inc;
    return it;
  }


  /**************************************************************************************************************/
  /**                                           distance calculation                                           **/
  /**************************************************************************************************************/
  /**
   * @brief Calculate the distance between this iterator and the given rhs iterator.
   * @tparam rhs_const
   * @param[in] rhs the end iterator
   * @return number of (key, value)-pairs between this iterator and rhs
   */
  template <bool rhs_const>
  [[nodiscard]] difference_type operator-(const info_iterator<rhs_const>& rhs) const {
    MPICXX_ASSERT(!this->singular() && !rhs.singular(),
                  "Attempt to compare a {} iterator to a {} iterator!",
                  this->state(), rhs.state());
    MPICXX_ASSERT(!this->info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                  "Attempt to compare a {} iterator{} to a {} iterator{}!",
                  this->state(), this->info_state(), rhs.state(), rhs.info_state());
    MPICXX_ASSERT(this->comparable(rhs), "Attempt to compare iterators from different sequences!");

    return pos_ - rhs.pos_;
  }


  /**************************************************************************************************************/
  /**                                              dereferencing                                               **/
  /**************************************************************************************************************/
  /**
   * @brief Get the (key, value)-pair at the current iterator position + n.
   * @details If the iterator is a non-const iterator, the value will be returned as a mpicxx::info::proxy object.
   * @param[in] n the requested offset of the iterator (may be negative)
   * @return the (key, value)-pair
   */
  [[nodiscard]] reference operator[](const difference_type n) const {
    MPICXX_ASSERT(!this->singular() && !this->info_refers_to_mpi_info_null(),
                  "Attempt to subscript a {} iterator{}!",
                  this->state(), this->info_state());
    MPICXX_ASSERT(this->advanceable(n) && this->advanceable(n + 1),
                  "Attempt to subscript a {} iterator {} step from its current position, which falls outside its dereferenceable range.",
                  this->state(), n);

    // get the key (with an offset of n)
    char key[MPI_MAX_INFO_KEY];
    MPI_Info_get_nthkey(*info_, pos_ + n, key);

    if constexpr (is_const) {
      // this is currently a const_iterator
      // -> retrieve the value associated with the key

      // get the length of the value associated with the key
      int valuelen, flag;
      MPI_Info_get_valuelen(*info_, key, &valuelen, &flag);

      // get the value associated with the key
      std::string value(valuelen, ' ');
      MPI_Info_get(*info_, key, valuelen, value.data(), &flag);

      return std::make_pair(std::string{ key }, std::move(value));
    } else {
      // this is currently a non-const iterator
      // -> create a proxy object and return it as value instead of a std::string
      // (allows changing the value in the info object)
      return std::make_pair(std::string{ key }, info_proxy{ *info_, key });
    }
  }

  /**
   * @brief Get the (key, value)-pair at the current iterator position.
   * @details If the iterator is a non-const iterator, the value will be returned as a mpicxx::info::proxy object.
   * @return the (key, value)-pair
   */
  [[nodiscard]] reference operator*() const {
    MPICXX_ASSERT(!this->singular() && !this->info_refers_to_mpi_info_null() && this->dereferenceable(),
                  "Attempt to dereference a {} iterator{}!",
                  this->state(), this->info_state());

    return this->operator[](0);
  }

  /**
   * @copydoc operator*()
   */
  [[nodiscard]] pointer operator->() const {
    MPICXX_ASSERT(!this->singular() && !this->info_refers_to_mpi_info_null() && this->dereferenceable(),
                  "Attempt to dereference a {} iterator{}!",
                  this->state(), this->info_state());

    return pointer{ this->operator[](0) };
  }

 private:
#ifdef MPICXX_ENABLE_ASSERTIONS

  [[nodiscard]] difference_type info_size() const {
    if (this->singular() || this->info_refers_to_mpi_info_null()) {
      return 0;
    }
    int nkeys = 0;
    MPI_Info_get_nkeys(*info_, &nkeys);
    return static_cast<difference_type>(nkeys);
  }

  [[nodiscard]] bool singular() const {
    return info_ == nullptr;
  }

  [[nodiscard]] bool info_refers_to_mpi_info_null() const {
    return info_ != nullptr && *info_ == MPI_INFO_NULL;
  }

  template <bool rhs_const>
  [[nodiscard]] bool comparable(const info_iterator<rhs_const>& rhs) const {
    return !this->singular() && !rhs.singular() && info_ == rhs.info_;
  }

  [[nodiscard]] bool past_the_end() const {
    return pos_ >= this->info_size();
  }

  [[nodiscard]] bool start_of_sequence() const {
    return pos_ == 0;
  }

  [[nodiscard]] bool incrementable() const {
    return !this->singular() && !this->info_refers_to_mpi_info_null() && !this->past_the_end();
  }

  [[nodiscard]] bool decrementable() const {
    return !this->singular() && !this->info_refers_to_mpi_info_null() && !this->start_of_sequence();
  }

  [[nodiscard]] bool advanceable(const difference_type n) const {
    if (this->singular() || this->info_refers_to_mpi_info_null()) {
      return false;
    } else if (n > 0) {
      return pos_ + n <= this->info_size();
    } else {
      return pos_ + n >= 0;
    }
  }

  [[nodiscard]] bool dereferenceable() const {
    return !this->singular() && !this->info_refers_to_mpi_info_null() && !this->past_the_end() && pos_ >= 0;
  }

  [[nodiscard]] std::string_view state() const {
    using namespace std::string_view_literals;
    if (this->singular()) {
      return "singular"sv;
    } else if (this->past_the_end()) {
      return "past-the-end"sv;
    } else if (pos_ < 0) {
      return "before-begin"sv;
    } else if (this->start_of_sequence()) {
      return "dereferenceable (start-of-sequence)"sv;
    } else {
      return "dereferenceable"sv;
    }
  }

  [[nodiscard]] std::string_view info_state() const {
    using namespace std::string_view_literals;
    if (this->info_refers_to_mpi_info_null()) {
      return " (referring to an info object refering to 'MPI_INFO_NULL')"sv;
    } else {
      return std::string_view{};
    }
  }

#endif

  MPI_Info_ptr info_;
  difference_type pos_;
};

}

#endif // MPICXX_INCLUDE_MPICXX_INFO_ITERATOR_HPP_
