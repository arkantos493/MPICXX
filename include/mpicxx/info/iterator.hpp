/**
 * 2021-05-09: Marcel Breyer
 * This file is distributed under the MIT License.
 *
 * Implements iterators for the mpicxx::info class.
 */

#ifndef MPICXX_INCLUDE_MPICXX_INFO_ITERATOR_HPP_
#define MPICXX_INCLUDE_MPICXX_INFO_ITERATOR_HPP_

#include <mpicxx/detail/assert.hpp>
#include <mpicxx/info/proxy.hpp>

#include <type_traits>
#include <utility>
#include <string>

namespace mpicxx::impl {

template <bool is_const>
class info_iterator_impl {
  template <bool>
  friend class info_iterator_impl;

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

  // pointer type to the referred to info object (pointer to const if `is_const` is `true`)
  using MPI_Info_ptr = std::conditional_t<is_const, const MPI_Info*, MPI_Info*>;
  // reference type to the referred to info object (reference to const if `is_const` is `true`)
  using MPI_Info_ref = std::conditional_t<is_const, const MPI_Info&, MPI_Info&>;

  using proxy = info_proxy;
 public:
  /**************************************************************************************************************/
  /**                                        iterator_traits definitions                                       **/
  /**************************************************************************************************************/
  using difference_type = std::ptrdiff_t;
  using value_type = std::conditional_t<is_const, std::pair<const std::string, const std::string>, std::pair<const std::string, proxy>>;
  using pointer = pointer_impl<value_type>;
  using reference = value_type;
  using iterator_category = std::random_access_iterator_tag;
  using iterator_concept = std::random_access_iterator_tag;


  /**************************************************************************************************************/
  /**                                               constructors                                               **/
  /**************************************************************************************************************/
  info_iterator_impl() noexcept: info_{ nullptr }, pos_{ 0 } { }

  info_iterator_impl(MPI_Info_ref info, const difference_type pos) noexcept: info_{ std::addressof(info) }, pos_{ pos } {
    MPICXX_ASSERT_SANITY(!this->singular(),
                         "Attempt to explicitly create a singular iterator!");
    MPICXX_ASSERT_SANITY(!this->info_refers_to_mpi_info_null(),
                         "Attempt to create an iterator from an info object referring to 'MPI_INFO_NULL'!");
    MPICXX_ASSERT_SANITY(pos_ >= 0 && pos <= this->info_size(),
                         "Attempt to create an iterator referring to {}, which falls outside its valid range!!", pos);
  }

  info_iterator_impl(const info_iterator_impl& other) noexcept: info_{ other.info_ }, pos_{ other.pos_ } {
    MPICXX_ASSERT_SANITY(!other.singular() && !other.info_refers_to_mpi_info_null(),
                         "Attempt to create an iterator from a {} iterator{}!",
                         other.state(), other.info_state());
  }

  template <bool is_const_ = is_const>
  requires is_const_
  info_iterator_impl(const info_iterator_impl<false>& other) noexcept: info_(other.info_), pos_(other.pos_) {
    MPICXX_ASSERT_SANITY(!other.singular() && !other.info_refers_to_mpi_info_null(),
                         "Attempt to create an iterator from a {} iterator{}!",
                         other.state(), other.info_state());
  }


  /**************************************************************************************************************/
  /**                                           assignment operators                                           **/
  /**************************************************************************************************************/
  info_iterator_impl& operator=(const info_iterator_impl& rhs) noexcept {
    MPICXX_ASSERT_SANITY(!rhs.singular() && !rhs.info_refers_to_mpi_info_null(),
                         "Attempt to assign a {} iterator{} to a {} iterator{}!",
                         rhs.state(), rhs.info_state(), this->state(), this->info_state());

    info_ = rhs.info_;
    pos_ = rhs.pos_;
    return *this;
  }

  template <bool is_const_ = is_const>
  requires is_const_
  info_iterator_impl& operator=(const info_iterator_impl<false>& rhs) noexcept {
    MPICXX_ASSERT_SANITY(!rhs.singular() && !rhs.info_refers_to_mpi_info_null(),
                         "Attempt to assign a {} iterator{} to a {} iterator{}!",
                         rhs.state(), rhs.info_state(), this->state(), this->info_state());

    info_ = rhs.info_;
    pos_ = rhs.pos_;
    return *this;
  }


  /**************************************************************************************************************/
  /**                                           relational operators                                           **/
  /**************************************************************************************************************/
  template <bool rhs_const>
  [[nodiscard]]
  bool operator==(const info_iterator_impl<rhs_const>& rhs) const {
    MPICXX_ASSERT_SANITY(!singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                         state(), rhs.state());
    MPICXX_ASSERT_SANITY(!info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                         "Attempt to compare a {} iterator{} to a {} iterator{}!",
                         state(), info_state(), rhs.state(), rhs.info_state());
    MPICXX_ASSERT_SANITY(comparable(rhs), "Attempt to compare iterators from different sequences!");

    return info_ == rhs.info_ && pos_ == rhs.pos_;
  }

  template <bool rhs_const>
  [[nodiscard]]
  std::partial_ordering operator<=>(const info_iterator_impl<rhs_const>& rhs) const {
    MPICXX_ASSERT_SANITY(!singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                         state(), rhs.state());
    MPICXX_ASSERT_SANITY(!info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                         "Attempt to compare a {} iterator{} to a {} iterator{}!",
                         state(), info_state(), rhs.state(), rhs.info_state());
    MPICXX_ASSERT_SANITY(comparable(rhs), "Attempt to compare iterators from different sequences!");

    if (auto cmp = info_ <=> rhs.info_; cmp != 0) return std::partial_ordering::unordered;
    return pos_ <=> rhs.pos_;
  }


  /**************************************************************************************************************/
  /**                                                 modifiers                                                **/
  /**************************************************************************************************************/
  info_iterator_impl& operator++() {
    MPICXX_ASSERT_SANITY(this->incrementable(), "Attempt to increment a {} iterator{}!", this->state(), this->info_state());

    ++pos_;
    return *this;
  }

  info_iterator_impl operator++(int) {
    MPICXX_ASSERT_SANITY(this->incrementable(), "Attempt to increment a {} iterator{}!", this->state(), this->info_state());

    info_iterator_impl tmp{ *this };
    this->operator++();
    return tmp;
  }

  info_iterator_impl& operator+=(const difference_type inc) {
    MPICXX_ASSERT_SANITY(this->advanceable(inc),
                         "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                         this->state(), this->info_state(), inc);

    pos_ += inc;
    return *this;
  }

  [[nodiscard("Did you mean 'operator+='?")]]
  friend info_iterator_impl operator+(info_iterator_impl it, const difference_type inc) {
    MPICXX_ASSERT_SANITY(it.advanceable(inc),
                         "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                         it.state(), it.info_state(), inc);

    it.pos_ += inc;
    return it;
  }

  [[nodiscard("Did you mean 'operator+='?")]]
  friend info_iterator_impl operator+(const difference_type inc, info_iterator_impl it) {
    MPICXX_ASSERT_SANITY(it.advanceable(inc),
                         "Attempt to advance a {} iterator{} {} steps, which falls outside its valid range!",
                         it.state(), it.info_state(), inc);

    return it + inc;
  }

  info_iterator_impl& operator--() {
    MPICXX_ASSERT_SANITY(this->decrementable(), "Attempt to decrement a {} iterator{}!", this->state(), this->info_state());

    --pos_;
    return *this;
  }

  info_iterator_impl operator--(int) {
    MPICXX_ASSERT_SANITY(this->decrementable(), "Attempt to decrement a {} iterator{}!", this->state(), this->info_state());

    info_iterator_impl tmp{ *this };
    this->operator--();
    return tmp;
  }

  info_iterator_impl& operator-=(const difference_type inc) {
    MPICXX_ASSERT_SANITY(this->advanceable(-inc),
                         "Attempt to retreat a {} iterator{} {} steps, which falls outside its valid range!",
                         this->state(), this->info_state(), inc);

    pos_ -= inc;
    return *this;
  }

  [[nodiscard("Did you mean 'operator-='?")]]
  friend info_iterator_impl operator-(info_iterator_impl it, const difference_type inc) {
    MPICXX_ASSERT_SANITY(
        it.advanceable(-inc),
        "Attempt to retreat a {} iterator{} {} steps, which falls outside its valid range!",
        it.state(), it.info_state(), inc);

    it.pos_ -= inc;
    return it;
  }


  /**************************************************************************************************************/
  /**                                           distance calculation                                           **/
  /**************************************************************************************************************/
  template <bool rhs_const>
  [[nodiscard]]
  difference_type operator-(const info_iterator_impl<rhs_const>& rhs) const {
    MPICXX_ASSERT_SANITY(!singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
                         state(), rhs.state());
    MPICXX_ASSERT_SANITY(!info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
                         "Attempt to compare a {} iterator{} to a {} iterator{}!",
                         state(), info_state(), rhs.state(), rhs.info_state());
    MPICXX_ASSERT_SANITY(comparable(rhs), "Attempt to compare iterators from different sequences!");

    return pos_ - rhs.pos_;
  }


  /**************************************************************************************************************/
  /**                                              dereferencing                                               **/
  /**************************************************************************************************************/
  [[nodiscard]]
  reference operator[](const difference_type n) const {
    MPICXX_ASSERT_PRECONDITION(!this->singular() && !this->info_refers_to_mpi_info_null(),
                               "Attempt to subscript a {} iterator{}!",
                               this->state(), this->info_state());
    MPICXX_ASSERT_PRECONDITION(this->advanceable(n) && this->advanceable(n + 1),
                               "Attempt to subscript a {} iterator {} step from its current position, which falls outside its dereferenceable range.",
                               this->state(),
                               n);

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
      return std::make_pair(std::string{ key }, proxy(*info_, key));
    }
  }

  [[nodiscard]]
  reference operator*() const {
    MPICXX_ASSERT_PRECONDITION(!this->singular() && !this->info_refers_to_mpi_info_null() && this->dereferenceable(),
                               "Attempt to dereference a {} iterator{}!", this->state(), this->info_state());

    return this->operator[](0);
  }

  [[nodiscard]]
  pointer operator->() const {
    MPICXX_ASSERT_PRECONDITION(!this->singular() && !this->info_refers_to_mpi_info_null() && this->dereferenceable(),
                               "Attempt to dereference a {} iterator{}!", this->state(), this->info_state());

    return pointer(this->operator[](0));
  }

 private:
#if MPICXX_ASSERTION_LEVEL > 0

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
  [[nodiscard]] bool comparable(const info_iterator_impl<rhs_const>& rhs) const {
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
      return std::string_view{ };
    }
  }

#endif

  MPI_Info_ptr info_;
  difference_type pos_;
};


/**************************************************************************************************************/
/**                                           relational operators                                           **/
/**************************************************************************************************************/
//template <bool lhs_const, bool rhs_const>
//[[nodiscard]]
//friend bool operator==(const info_iterator_impl<lhs_const>& lhs, const info_iterator_impl<rhs_const>& rhs) {
//  MPICXX_ASSERT_SANITY(!lhs.singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
//                       lhs.state(), rhs.state());
//  MPICXX_ASSERT_SANITY(!lhs.info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
//                       "Attempt to compare a {} iterator{} to a {} iterator{}!",
//                       lhs.state(), lhs.info_state(), rhs.state(), rhs.info_state());
//  MPICXX_ASSERT_SANITY(lhs.comparable(rhs), "Attempt to compare iterators from different sequences!");
//
//  return lhs.info_ == rhs.info_ && lhs.pos_ == rhs.pos_;
//}

//template <bool lhs_const, bool rhs_const>
//[[nodiscard]]
//friend std::partial_ordering operator<=>(const info_iterator_impl<lhs_const>& lhs, const info_iterator_impl<rhs_const>& rhs) {
//  MPICXX_ASSERT_SANITY(!lhs.singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
//                       lhs.state(), rhs.state());
//  MPICXX_ASSERT_SANITY(!lhs.info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
//                       "Attempt to compare a {} iterator{} to a {} iterator{}!",
//                       lhs.state(), lhs.info_state(), rhs.state(), rhs.info_state());
//  MPICXX_ASSERT_SANITY(lhs.comparable(rhs), "Attempt to compare iterators from different sequences!");
//
//  if (auto cmp = lhs.info_ <=> rhs.info_; cmp != 0) return std::partial_ordering::unordered;
//  return lhs.pos_ <=> rhs.pos_;
//}


/**************************************************************************************************************/
/**                                           distance calculation                                           **/
/**************************************************************************************************************/
//template <bool lsh_const, bool rhs_const>
//[[nodiscard]]
//friend auto operator-(const info_iterator_impl<lsh_const>& lhs, const info_iterator_impl<rhs_const>& rhs) {
//  MPICXX_ASSERT_SANITY(!lhs.singular() && !rhs.singular(), "Attempt to compare a {} iterator to a {} iterator!",
//                       lhs.state(), rhs.state());
//  MPICXX_ASSERT_SANITY(!lhs.info_refers_to_mpi_info_null() && !rhs.info_refers_to_mpi_info_null(),
//                       "Attempt to compare a {} iterator{} to a {} iterator{}!",
//                       lhs.state(), lhs.info_state(), rhs.state(), rhs.info_state());
//  MPICXX_ASSERT_SANITY(lhs.comparable(rhs), "Attempt to compare iterators from different sequences!");
//
//  return static_cast<typename info_iterator_impl<true>::difference_type>(lhs.pos_ - rhs.pos_);
//}

}

#endif // MPICXX_INCLUDE_MPICXX_INFO_ITERATOR_HPP_
