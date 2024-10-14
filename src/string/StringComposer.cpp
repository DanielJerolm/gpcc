/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#include <gpcc/string/StringComposer.hpp>
#include <gpcc/osal/Panic.hpp>
#include <stdexcept>
#include <cstdio>
#include <cstring>

namespace gpcc   {
namespace string {

/**
 * \brief Constructor. Creates an empty @ref StringComposer with unspecified capacity.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
StringComposer::StringComposer(void)
: str_()
, boolalpha_(false)
, showBase_(false)
, showPos_(false)
, showPoint_(false)
, uppercase_(false)
, base_(Base::decimal)
, floatFormat_(FloatFormat::automatic)
, align_(Alignment::right)
, width_(0)
, prec_(6)
{
}

/**
 * \brief Constructor. Creates an empty @ref StringComposer with given capacity.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param capacity
 * Initial capacity.
 */
StringComposer::StringComposer(size_t const capacity)
: StringComposer()
{
  str_.reserve(capacity);
}

/**
 * \brief Constructor. Creates a @ref StringComposer with initial content copied from a null-terminated C string.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pStr
 * Pointer to a null-terminated C string.\n
 * `nullptr` is not allowed.
 */
StringComposer::StringComposer(char const * const pStr)
: StringComposer()
{
  str_ = pStr;
}

/**
 * \brief Constructor. Creates a @ref StringComposer with initial content copied from an `std::string`.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to an `std::string` whose content shall be copied into the new @ref StringComposer instance.
 */
StringComposer::StringComposer(std::string const & s)
: StringComposer()
{
  str_ = s;
}

/**
 * \brief Constructor. Creates a @ref StringComposer with initial content moved from an `std::string`.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Universal reference to an `std::string` whose content shall be moved into the new @ref StringComposer instance.
 */
StringComposer::StringComposer(std::string && s)
: StringComposer()
{
  str_ = std::move(s);
}

/**
 * \brief Copy-constructs a @ref StringComposer.
 *
 * \note  The capacity of @p other is not adopted.
 *
 * - - -
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * Unmodifiable reference to another @ref StringComposer instance that shall be used to copy-construct the new one.
 */
StringComposer::StringComposer(StringComposer const & other)
: str_(other.str_)
, boolalpha_(other.boolalpha_)
, showBase_(other.showBase_)
, showPos_(other.showPos_)
, showPoint_(other.showPoint_)
, uppercase_(other.uppercase_)
, base_(other.base_)
, floatFormat_(other.floatFormat_)
, align_(other.align_)
, width_(other.width_)
, prec_(other.prec_)
{
}

/**
 * \brief Move-constructs a @ref StringComposer.
 *
 * - - -
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param other
 * Universal reference to another @ref StringComposer instance that shall be used to move-construct the new one.\n
 * The referenced instance is left in an undefined, but valid state.
 */
StringComposer::StringComposer(StringComposer && other) noexcept
: str_(std::move(other.str_))
, boolalpha_(other.boolalpha_)
, showBase_(other.showBase_)
, showPos_(other.showPos_)
, showPoint_(other.showPoint_)
, uppercase_(other.uppercase_)
, base_(other.base_)
, floatFormat_(other.floatFormat_)
, align_(other.align_)
, width_(other.width_)
, prec_(other.prec_)
{
}

/**
 * \brief Copy-assigns the content and configuration of another @ref StringComposer instance to this instance.
 *
 * \post  The capacity of this @ref StringComposer instance is unspecified.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Unmodifiable reference to another @ref StringComposer instance whose contents and configuration shall be copy-
 * assigned to this instance.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator=(StringComposer const & rhv)
{
  if (&rhv != this)
  {
    str_ = rhv.str_;
    boolalpha_ = rhv.boolalpha_;
    showBase_ = rhv.showBase_;
    showPos_ = rhv.showPos_;
    showPoint_ = rhv.showPoint_;
    uppercase_ = rhv.uppercase_;
    base_ = rhv.base_;
    floatFormat_ = rhv.floatFormat_;
    align_ = rhv.align_;
    width_ = rhv.width_;
    prec_ = rhv.prec_;
  }

  return *this;
}

/**
 * \brief Move-assigns the content and configuration of another @ref StringComposer instance to this instance.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Universal reference to another @ref StringComposer instance whose contents and configuration shall be move-
 * assigned to this instance.\n
 * The referenced instance is left in an undefined, but valid state.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator=(StringComposer && rhv) noexcept
{
  if (&rhv != this)
  {
    str_ = std::move(rhv.str_);
    boolalpha_ =  rhv.boolalpha_;
    showBase_ = rhv.showBase_;
    showPos_ = rhv.showPos_;
    showPoint_ = rhv.showPoint_;
    uppercase_ = rhv.uppercase_;
    base_ = rhv.base_;
    floatFormat_ = rhv.floatFormat_;
    align_ = rhv.align_;
    width_ = rhv.width_;
    prec_ = rhv.prec_;
  }

  return *this;
}

/**
 * \brief Consumes a `Width_` instance produced by @ref StringComposer::Width().
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param w
 * `Width_` instance returned by @ref StringComposer::Width().
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(Width_ const w)
{
  if (w.w_ < 0)
    throw std::invalid_argument("Invalid args");

  width_ = w.w_;
  return *this;
}

/**
 * \brief Consumes a `Precision_` instance produced by @ref StringComposer::Precision().
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param p
 * `Precision_` instance returned by @ref StringComposer::Precision().
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(Precision_ const p)
{
  if (p.p_ < 0)
    throw std::invalid_argument("Invalid args");

  prec_ = p.p_;
  return *this;
}

/**
 * \brief Formats and inserts data of type `bool` into the @ref StringComposer.
 *
 * \post  The configured field-width is reset to zero.
 *
 * \see   @ref StringComposer::BoolAlpha() and @ref StringComposer::NoBoolAlpha() \n
 *        @ref StringComposer::AlignLeft() and @ref StringComposer::AlignRight() \n
 *        @ref StringComposer::Width()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Right hand value of operator <<.\n
 * The value will be converted into a string and appended to the @ref StringComposer.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(bool const rhv)
{
  char const * pStr;
  if (boolalpha_)
    pStr = rhv ? "true" : "false";
  else
    pStr = rhv ? "1" : "0";

  return operator<<(pStr);
}

/**
 * \brief Formats and inserts integer data into the @ref StringComposer.
 *
 * \post  The configured field-width is reset to zero.
 *
 * \see   @ref StringComposer::ShowBase() and  @ref StringComposer::NoShowBase() \n
 *        @ref StringComposer::BaseDec() \n
 *        @ref StringComposer::BaseHex() \n
 *        @ref StringComposer::BaseOct() \n
 *        @ref StringComposer::AlignLeft() and @ref StringComposer::AlignRight() \n
 *        @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos() \n
 *        @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase() \n
 *        @ref StringComposer::Width()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Right hand value of operator <<.\n
 * The value will be converted into a string and appended to the @ref StringComposer.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(short const rhv)
{
  if ((rhv < 0) && (base_ != Base::decimal))
    return operator<<(static_cast<unsigned short>(rhv));

  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::s, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(unsigned short const rhv)
{
  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::us, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(int const rhv)
{
  if ((rhv < 0) && (base_ != Base::decimal))
    return operator<<(static_cast<unsigned int>(rhv));

  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::i, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(unsigned int const rhv)
{
  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::ui, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(long const rhv)
{
  if ((rhv < 0) && (base_ != Base::decimal))
    return operator<<(static_cast<unsigned long>(rhv));

  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::l, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(unsigned long const rhv)
{
  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::ul, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(long long const rhv)
{
  if ((rhv < 0) && (base_ != Base::decimal))
    return operator<<(static_cast<unsigned long long>(rhv));

  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::ll, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(short const) */
StringComposer& StringComposer::operator<<(unsigned long long const rhv)
{
  char buffer[intConvBufSize];
  PrintiToBuffer(buffer, sizeof(buffer), Type::ull, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/**
 * \brief Formats and inserts floating point data into the @ref StringComposer.
 *
 * \post  The configured field-width is reset to zero.
 *
 * \see   @ref StringComposer::FixedFloat() \n
 *        @ref StringComposer::ScientificFloat() \n
 *        @ref StringComposer::HexFloat() \n
 *        @ref StringComposer::AutoFloat() \n
 *        @ref StringComposer::AlignLeft() and @ref StringComposer::AlignRight() \n
 *        @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos() \n
 *        @ref StringComposer::ShowPoint() and @ref StringComposer::NoShowPoint() \n
 *        @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase() \n
 *        @ref StringComposer::Width() \n
 *        @ref StringComposer::Precision()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Right hand value of operator <<.\n
 * The value will be converted into a string and appended to the @ref StringComposer.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(float const rhv)
{
  char buffer[floatConvBufSize];
  PrintToBuffer(buffer, sizeof(buffer), Type::f, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(float const) */
StringComposer& StringComposer::operator<<(double const rhv)
{
  char buffer[floatConvBufSize];
  PrintToBuffer(buffer, sizeof(buffer), Type::d, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/** \copydoc StringComposer::operator<<(float const) */
StringComposer& StringComposer::operator<<(long double const rhv)
{
  char buffer[floatConvBufSize];
  PrintToBuffer(buffer, sizeof(buffer), Type::ld, rhv);

  str_ += buffer;
  width_ = 0;
  return *this;
}

/**
 * \brief Inserts a single character into the @ref StringComposer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Right hand value of operator <<.\n
 * The character will be appended to the @ref StringComposer.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(char const rhv)
{
  return operator<<(static_cast<unsigned char>(rhv));
}

/** \copydoc StringComposer::operator<<(char const) */
StringComposer& StringComposer::operator<<(unsigned char const rhv)
{
  if (width_ <= 1)
  {
    str_ += rhv;
  }
  else
  {
    str_.reserve(str_.size() + width_);

    // For the next step we cannot provide the strong guarantee.
    // But we have reserved capacity, so no error is expected.
    try
    {
      if (align_ == Alignment::left)
      {
        str_ += rhv;
        str_.append(width_ - 1, ' ');
      }
      else
      {
        str_.append(width_ - 1, ' ');
        str_ += rhv;
      }
    }
    catch (...)
    {
      PANIC();
    }
  }

  width_ = 0;
  return *this;
}

/**
 * \brief Inserts a copy of a null-terminated C string into the @ref StringComposer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Right hand value of operator <<.\n
 * The C string will be appended to the @ref StringComposer.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(char const * const rhv)
{
  if (rhv == nullptr)
    throw std::invalid_argument("Inv. args");

  if (width_ == 0)
  {
    str_ += rhv;
  }
  else
  {
    size_t const len = strlen(rhv);
    if (len >= static_cast<size_t>(width_))
    {
      str_.append(rhv, len);
    }
    else
    {
      str_.reserve(str_.size() + width_);

      // For the next step we cannot provide the strong guarantee.
      // But we have reserved capacity, so no error is expected.
      try
      {
        if (align_ == Alignment::left)
        {
          str_.append(rhv, len);
          str_.append(width_ - len, ' ');
        }
        else
        {
          str_.append(width_ - len, ' ');
          str_.append(rhv, len);
        }
      }
      catch (...)
      {
        PANIC();
      }
    }
  }

  width_ = 0;
  return *this;
}

/**
 * \brief Inserts a copy of an `std::string` into the @ref StringComposer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param rhv
 * Right hand value of operator <<.\n
 * The string's content will be appended to the @ref StringComposer.
 *
 * \return
 * Reference to self.
 */
StringComposer& StringComposer::operator<<(std::string const & rhv)
{
  if (width_ == 0)
  {
    str_ += rhv;
  }
  else
  {
    size_t const len = rhv.size();
    if (len >= static_cast<size_t>(width_))
    {
      str_ += rhv;
    }
    else
    {
      str_.reserve(str_.size() + width_);

      // For the next step we cannot provide the strong guarantee.
      // But we have reserved capacity, so no error is expected.
      try
      {
        if (align_ == Alignment::left)
        {
          str_ += rhv;
          str_.append(width_ - len, ' ');
        }
        else
        {
          str_.append(width_ - len, ' ');
          str_ += rhv;
        }
      }
      catch (...)
      {
        PANIC();
      }
    }
  }

  width_ = 0;
  return *this;
}

/**
 * \brief Clears the content of the @ref StringComposer.
 *
 * \post  The capacity of the @ref StringComposer is not changed.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 */
void StringComposer::Clear(void) noexcept
{
  // Note:
  // std::string::resize() does not change the capacity.
  // For std::string::clear() the standard allows reduction of the capacity, though all implementations of
  // std::string seem to keep the capacity.
  str_.resize(0U);
}

/**
 * \brief Replaces the content of the @ref StringComposer with a copy of a given null-terminated C string.
 *
 * \post  The capacity of the @ref StringComposer may have been increased.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pStr
 * Pointer to a null-terminated C string that shall be copied and used to replace the current content of the
 * @ref StringComposer.
 */
void StringComposer::Set(char const * const pStr)
{
  str_ = pStr;
}

/**
 * \brief Replaces the content of the @ref StringComposer with a copy of a given `std::string`.
 *
 * \post  The capacity of the @ref StringComposer may have been increased.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Unmodifiable reference to an `std::string` that shall be copied and used to replace the current content of the
 * @ref StringComposer.
 */
void StringComposer::Set(std::string const & s)
{
  str_ = s;
}

/**
 * \brief Replaces the content of the @ref StringComposer with a given `std::string` using move-semantics.
 *
 * \post  The capacity of the @ref StringComposer is unspecified.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param s
 * Universal reference to an `std::string` that shall be used to replace the current content of the @ref StringComposer
 * using move semantics. The referenced object is left in an undefined, but valid state.
 */
void StringComposer::Set(std::string && s)
{
  str_ = std::move(s);
}

/**
 * \brief Retrieves a copy of the content of the @ref StringComposer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \return
 * A `std::string` object containing a copy of the current content of the @ref StringComposer.
 */
std::string StringComposer::Get(void) const
{
  return str_;
}

/**
 * \brief Prints a single integer value into a buffer.
 *
 * This makes use of `sniprintf()` if it is available.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam T
 * Type of value.
 *
 * \param buffer
 * The converted value is written into the referenced buffer.
 *
 * \param bufferSize
 * Size of the buffer referenced by @p buffer.
 *
 * \param type
 * Type of @p value. Must match @p T.
 *
 * \param value
 * Value that shall be converted into a string.
 */
#if defined(_NEWLIB_VERSION)
template<typename T>
void StringComposer::PrintiToBuffer(char* const buffer, size_t const bufferSize, Type const type, T const value) const
{
  char fmt[maxFmtStrBufSize];
  int status;
  if (!SetupFormatString(fmt, type))
    status = sniprintf(buffer, bufferSize, fmt, width_, value);
  else
    status = sniprintf(buffer, bufferSize, fmt, width_, prec_, value);

  if (status < 0)
    throw std::logic_error("sniprintf failed");
  else if (static_cast<size_t>(status) >= bufferSize)
    throw std::logic_error("buffer too small");
}
#else
template<typename T>
void StringComposer::PrintiToBuffer(char* const buffer, size_t const bufferSize, Type const type, T const value) const
{
  PrintToBuffer(buffer, bufferSize, type, value);
}
#endif

/**
 * \brief Prints a single integer or floating-point value into a buffer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * Strong guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \tparam T
 * Type of value.
 *
 * \param buffer
 * The converted value is written into the referenced buffer.
 *
 * \param bufferSize
 * Size of the buffer referenced by @p buffer.
 *
 * \param type
 * Type of @p value. Must match @p T.
 *
 * \param value
 * Value that shall be converted into a string.
 */
template<typename T>
void StringComposer::PrintToBuffer(char* const buffer, size_t const bufferSize, Type const type, T const value) const
{
  char fmt[maxFmtStrBufSize];
  int status;
  if (!SetupFormatString(fmt, type))
    status = snprintf(buffer, bufferSize, fmt, width_, value);
  else
    status = snprintf(buffer, bufferSize, fmt, width_, prec_, value);

  if (status < 0)
    throw std::logic_error("snprintf failed");
  else if (static_cast<size_t>(status) >= bufferSize)
    throw std::logic_error("buffer too small");
}

/**
 * \brief Creates a format-string for `snprintf()`.
 *
 * Depending on the return value, `snprintf()` must be invoked as follows:
 * ~~~{cpp}
 * char fmt[maxFmtStrBufSize];
 * int status;
 * if (!SetupFormatString(fmt, type))
 *   status = snprintf(buffer, bufferSize, fmt, width_, value);
 * else
 *   status = snprintf(buffer, bufferSize, fmt, width_, prec_, value);
 * ~~~
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the object is not modified. Concurrent accesses are safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param pFMT
 * The format string is constructed into the referenced buffer.\n
 * The buffer must have a capacity of @ref maxFmtStrBufSize.
 *
 * \param type
 * Type that shall be converted by `snprintf()`.
 *
 * \return
 * Flag indicating if `prec_` must be passed to `snprintf()` or not. See example above.
 */
bool StringComposer::SetupFormatString(char* pFMT, Type const type) const noexcept
{
  static_assert(maxFmtStrBufSize >= 12U);

  bool providePrec = false;
  bool const fpType = IsFloat(type);

  // Format string expected by snprintf() and friends:
  // %[pos][flags][width][.prec][size]type

  *pFMT++ = '%';

  // [pos]
  // n.A., arguments are consumed in order.

  // [flags]

  if (align_ == Alignment::left)
    *pFMT++ = '-';

  if (showPos_)
    *pFMT++ = '+';

  if (align_ == Alignment::rightPadZero)
    *pFMT++ = '0';

  if (fpType)
  {
    if (showPoint_)
      *pFMT++ = '#';
  }
  else
  {
    if (showBase_)
      *pFMT++ = '#';
  }

  // [width]

  *pFMT++ = '*';

  // [.prec][size]type

  auto AddPrecision = [&]()
  {
    // The reference implementation (std::ostringstream) ignores the field-width for hexadecimal floating-point format,
    // so we do too.
    if (floatFormat_ != FloatFormat::hex)
    {
      *pFMT++ = '.';
      *pFMT++ = '*';
      providePrec = true;
    }
  };

  auto AddTypeSignedInt = [&]()
  {
    switch (base_)
    {
      case Base::decimal:
        *pFMT++ = 'd';
        break;

      case Base::hexadecimal:
        *pFMT++ = uppercase_ ? 'X' : 'x';
        break;

      case Base::octal:
        *pFMT++ = 'o';
        break;
    }
  };

  auto AddTypeUnsignedInt = [&]()
  {
    switch (base_)
    {
      case Base::decimal:
        *pFMT++ = 'u';
        break;

      case Base::hexadecimal:
        *pFMT++ = uppercase_ ? 'X' : 'x';
        break;

      case Base::octal:
        *pFMT++ = 'o';
        break;
    }
  };

  auto AddFloatType = [&]()
  {
    switch (floatFormat_)
    {
      case FloatFormat::fixed:
        *pFMT++ = uppercase_ ? 'F' : 'f';
        break;

      case FloatFormat::scientific:
        *pFMT++ = uppercase_ ? 'E' : 'e';
        break;

      case FloatFormat::hex:
        *pFMT++ = uppercase_ ? 'A' : 'a';
        break;

      case FloatFormat::automatic:
        *pFMT++ = uppercase_ ? 'G' : 'g';
        break;
    }
  };

  switch (type)
  {
    case Type::s:
    {
      *pFMT++ = 'h';
      AddTypeSignedInt();
      break;
    }

    case Type::us:
    {
      *pFMT++ = 'h';
      AddTypeUnsignedInt();
      break;
    }

    case Type::i:
    {
      AddTypeSignedInt();
      break;
    }

    case Type::ui:
    {
      AddTypeUnsignedInt();
      break;
    }

    case Type::l:
    {
      *pFMT++ = 'l';
      AddTypeSignedInt();
      break;
    }

    case Type::ul:
    {
      *pFMT++ = 'l';
      AddTypeUnsignedInt();
      break;
    }

    case Type::ll:
    {
      *pFMT++ = 'l';
      *pFMT++ = 'l';
      AddTypeSignedInt();
      break;
    }

    case Type::ull:
    {
      *pFMT++ = 'l';
      *pFMT++ = 'l';
      AddTypeUnsignedInt();
      break;
    }

    case Type::f:
    {
      AddPrecision();
      AddFloatType();
      break;
    }

    case Type::d:
    {
      AddPrecision();
      *pFMT++ = 'l';
      AddFloatType();
      break;
    }

    case Type::ld:
    {
      AddPrecision();
      *pFMT++ = 'L';
      AddFloatType();
      break;
    }
  }

  // add trailing null-char
  *pFMT = 0;
  return providePrec;
}

/**
 * \brief Queries if a value from the @ref Type enumeration is a floating-point type or not.
 *
 * - - -
 *
 * __Thread safety:__\n
 * This is thread-safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param type
 * Type to test.
 *
 * \retval true   @p type refers to a floating-point type.
 * \retval false  @p type refers to an integer type.
 */
bool StringComposer::IsFloat(Type const type) noexcept
{
  return (   (type == Type::f)
          || (type == Type::d)
          || (type == Type::ld));
}

} // namespace string
} // namespace gpcc
