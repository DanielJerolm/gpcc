/*
    General Purpose Class Collection (GPCC)

    This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
    If a copy of the MPL was not distributed with this file,
    You can obtain one at https://mozilla.org/MPL/2.0/.

    Copyright (C) 2024 Daniel Jerolm
*/

#ifndef STRINGCOMPOSER_HPP_202407291957
#define STRINGCOMPOSER_HPP_202407291957

#include <string>
#include <cstddef>

namespace gpcc   {
namespace string {

/**
 * \ingroup GPCC_STRING
 * \brief Leight-weight alternative for `std::ostringstream` for composition of formatted strings.
 *
 * # Rationale
 * It has been observed (EPOS operating system, Cortex-M4, gcc 10.3) that even simplest usage of `std::ostringstream`
 * pulls in up to 190kB of machine code:
 * ~~~{.cpp}
 * std::ostringstream oss;
 * oss << "cnt = " << cnt << std::endl;
 * ~~~
 * The increase of code size is significant compared to the size of the flash memory typically available on small MCUs
 * (< 1MB). Usage of `std::ostringstream` should therefore be avoided on small MCUs.
 *
 * This class offers an alternative to `std::ostringstream` that consumes only 5..24kB of code memory. It can fully
 * replace `std::ostringstream` in most use cases that just incorporate composition of formatted strings from string
 * snippets and from integer and floating-point values.
 *
 * # Usage
 * ## Composing strings
 * Just stream variables and strings into an @ref StringComposer instance and finally fetch the composed string into an
 * `std::string` object. Example:
 * ~~~{.cpp}
 * gpcc::string::StringComposer sc;
 * sc << "cnt = " << cnt;
 * // [...]
 *
 * std::string composedString(sc.Get());
 * ~~~
 *
 * The following basic data types are accepted and will be converted into a string representation that will be appended
 * to the composed string:\n
 * bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long
 *
 * The following basic and complex data types are accepted and will be appended to the composed string without
 * conversion:\n
 * char, unsigned char, null-terminated c-string (char const *), std::string
 *
 * The following floating-point types are accepted and will be converted into a string representation that will be
 * appended to the composed string:\n
 * float, double, long double
 *
 * ## Formatting
 * The output format can be configured by streaming configuration functions into a @ref StringComposer instance.\n
 * Example:
 * ~~~{.cpp}
 * using gpcc::string::StringComposer;
 * StringComposer sc;
 * // Add 'cnt' to the stream using hexadecimal format:
 * sc << "cnt as hex = " << StringComposer::ShowBase << StringComposer::BaseHex << cnt;
 * ~~~
 *
 * The following configuration functions are available:
 * - @ref BoolAlpha()
 * - @ref NoBoolAlpha()
 * - @ref ShowBase()
 * - @ref NoShowBase()
 * - @ref BaseDec()
 * - @ref BaseHex()
 * - @ref BaseOct()
 * - @ref FixedFloat()
 * - @ref ScientificFloat()
 * - @ref HexFloat()
 * - @ref AutoFloat()
 * - @ref AlignLeft()
 * - @ref AlignRight()
 * - @ref AlignRightPadZero()
 * - @ref ShowPos()
 * - @ref NoShowPos()
 * - @ref ShowPoint()
 * - @ref NoShowPoint()
 * - @ref Uppercase()
 * - @ref NoUppercase()
 * - @ref Width()
 * - @ref Precision()
 *
 * ## Default configuration
 * The initial configuration for any @ref StringComposer instance is:
 * - Alphanumeric bool: No
 * - Show base: No
 * - Show plus sign for positive numbers: No
 * - Always show decimal point in floating-point output: No
 * - Uppercase in hex and floating-point output: No
 * - Base for integer output: decimal
 * - Format for floating-point output: automatic
 * - Alignment: Right
 * - Field width: 0
 * - Precision for floating-point output: 6
 *
 * - - -
 *
 * __Thread safety:__\n
 * Not thread safe, but non-modifying concurrent access is safe.
 */
class StringComposer final
{
  // internal forward declarations
  private:
    struct Width_;
    struct Precision_;

  public:
    StringComposer(void);
    StringComposer(size_t const capacity);
    StringComposer(char const * const pStr);
    StringComposer(std::string const & s);
    StringComposer(std::string && s);
    StringComposer(StringComposer const & other);
    StringComposer(StringComposer && other) noexcept;
    ~StringComposer(void) = default;

    StringComposer& operator=(StringComposer const & rhv);
    StringComposer& operator=(StringComposer && rhv) noexcept;

    static StringComposer& BoolAlpha(StringComposer& sc);
    static StringComposer& NoBoolAlpha(StringComposer& sc);
    static StringComposer& ShowBase(StringComposer& sc);
    static StringComposer& NoShowBase(StringComposer& sc);
    static StringComposer& BaseDec(StringComposer& sc);
    static StringComposer& BaseHex(StringComposer& sc);
    static StringComposer& BaseOct(StringComposer& sc);
    static StringComposer& FixedFloat(StringComposer& sc);
    static StringComposer& ScientificFloat(StringComposer& sc);
    static StringComposer& HexFloat(StringComposer& sc);
    static StringComposer& AutoFloat(StringComposer& sc);
    static StringComposer& AlignLeft(StringComposer& sc);
    static StringComposer& AlignRight(StringComposer& sc);
    static StringComposer& AlignRightPadZero(StringComposer& sc);
    static StringComposer& ShowPos(StringComposer& sc);
    static StringComposer& NoShowPos(StringComposer& sc);
    static StringComposer& ShowPoint(StringComposer& sc);
    static StringComposer& NoShowPoint(StringComposer& sc);
    static StringComposer& Uppercase(StringComposer& sc);
    static StringComposer& NoUppercase(StringComposer& sc);
    static StringComposer::Width_ Width(int const w);
    static StringComposer::Precision_ Precision(int const p);

    StringComposer& operator<<(Width_ const w);
    StringComposer& operator<<(Precision_ const p);
    StringComposer& operator<<(StringComposer& (*func)(StringComposer&));

    StringComposer& operator<<(bool const rhv);
    StringComposer& operator<<(short const rhv);
    StringComposer& operator<<(unsigned short const rhv);
    StringComposer& operator<<(int const rhv);
    StringComposer& operator<<(unsigned int const rhv);
    StringComposer& operator<<(long const rhv);
    StringComposer& operator<<(unsigned long const rhv);
    StringComposer& operator<<(long long const rhv);
    StringComposer& operator<<(unsigned long long const rhv);

    StringComposer& operator<<(float const rhv);
    StringComposer& operator<<(double const rhv);
    StringComposer& operator<<(long double const rhv);

    StringComposer& operator<<(char const rhv);
    StringComposer& operator<<(unsigned char const rhv);
    StringComposer& operator<<(char const * const rhv);

    StringComposer& operator<<(std::string const & rhv);

    void Clear(void) noexcept;
    void Set(char const * const pStr);
    void Set(std::string const & s);
    void Set(std::string && s);
    std::string Get(void) const;

  private:
    /// Streams a field-width-value into a @ref StringComposer instance.
    struct Width_ { int w_; };

    /// Streams a precision-value into a @ref StringComposer instance.
    struct Precision_ { int p_; };

    /// Base configuration for integer conversion.
    enum class Base
    {
      decimal,      ///<Decimal (base 10)
      hexadecimal,  ///<Hexadecimal (base 16)
      octal         ///<Octal (base 8)
    };

    /// Alignment for strings and converted data appended to the content of a @ref StringComposer instance.
    enum class Alignment
    {
      left,         ///<Align left, pad on the right with white-spaces if required.
      right,        ///<Align right, pad on the left with white-spaces if required.
      rightPadZero  ///<Align right, pad on the left with zeros (figures) or white-spaces (text) if required.
    };

    /// Formats for conversion of floating-point numbers.
    enum class FloatFormat
    {
      fixed,       ///<Fix-point format.
      scientific,  ///<Scientific format.
      hex,         ///<Hexadecimal format.
      automatic    ///<Selects between fixed and scientific format depending on the actual value and the configured
                   ///<precision.
    };

    /// Data types.
    enum class Type
    {
      s,   ///<short
      us,  ///<unsigned short
      i,   ///<int
      ui,  ///<unsigned int
      l,   ///<long
      ul,  ///<unsigned long
      ll,  ///<long long
      ull, ///<unsigned long long
      f,   ///<float
      d,   ///<double
      ld   ///<long double
    };

    /// Size for buffers used to convert integer values to string.
    static size_t constexpr intConvBufSize = 32U;

    /// Size for buffers used to convert floating-point values to string.
    static size_t constexpr floatConvBufSize = 32U;

    /// Size for buffers containing format strings for snprintf()
    static size_t constexpr maxFmtStrBufSize = 12U;


    /// The string is composed here.
    std::string str_;

    bool boolalpha_;          ///<Configuration: Convert bool to true/false or 1/0.
    bool showBase_;           ///<Configuration: Show base when converting integer values.
    bool showPos_;            ///<Configuration: Always show '+'-sign for positive integer values.
    bool showPoint_;          ///<Configuration: Always show decimal point in floating-point output.
    bool uppercase_;          ///<Configuration: Use uppercase characters for hexadecimal output in integer and floating-point conversions.
    Base base_;               ///<Configuration: Base that shall be used for integer conversion.
    FloatFormat floatFormat_; ///<Configuration: Floating point format.
    Alignment align_;         ///<Configuration: Alignment of any output appended to the composed string.
    int width_;               ///<Configuration: Field with for any output appended to the composed string.
    int prec_;                ///<Configuration: Precision that shall be used in floating-point conversions.

    template<typename T>
    void PrintiToBuffer(char* const buffer, size_t const bufferSize, Type const type, T const value) const;
    template<typename T>
    void PrintToBuffer(char* const buffer, size_t const bufferSize, Type const type, T const value) const;
    bool SetupFormatString(char* pFMT, Type const type) const noexcept;

    static bool IsFloat(Type const type) noexcept;
};


/**
 * \brief Configures a @ref StringComposer to convert boolean values to textual values "true" and "false".
 *
 * Usage:
 * ~~~{.cpp}
 * bool t = true;
 * bool f = false;
 * sc << StringComposer::BoolAlpha << t << ' ' << f;
 * // Result: "true false"
 * ~~~
 *
 * The default setting is conversion to _numeric values "1" and "0"_.
 *
 * \see @ref StringComposer::NoBoolAlpha()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::BoolAlpha(StringComposer& sc)
{
  sc.boolalpha_ = true;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to convert boolean values to numeric values "1" and "0".
 *
 * Usage:
 * ~~~{.cpp}
 * bool t = true;
 * bool f = false;
 * sc << StringComposer::NoBoolAlpha << t << ' ' << f;
 * // Result: "1 0"
 * ~~~
 *
 * The default setting is conversion to _numeric values "1" and "0"_.
 *
 * \see @ref StringComposer::BoolAlpha()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::NoBoolAlpha(StringComposer& sc)
{
  sc.boolalpha_ = false;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print the base in front of hexadecimal integer numbers.
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int value = 2997U;
 * sc << StringComposer::ShowBase << StringComposer::BaseHex << value;
 * // Result: "0xbb5"
 * ~~~
 *
 * The default setting is _not_ to print the base prefix.
 *
 * \see @ref StringComposer::NoShowBase() \n
 *      @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::ShowBase(StringComposer& sc)
{
  sc.showBase_ = true;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer not to print the base in front of hexadecimal integer numbers.
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int value = 2997U;
 * sc << StringComposer::NoShowBase << StringComposer::BaseHex << value;
 * // Result: "bb5"
 * ~~~
 *
 * The default setting is _not_ to print the base prefix.
 *
 * \see @ref StringComposer::ShowBase() \n
 *      @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::NoShowBase(StringComposer& sc)
{
  sc.showBase_ = false;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print integer numbers in decimal format (base 10).
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int value = 25U;
 * sc << StringComposer::BaseDec << value;
 * // Result: "25"
 * ~~~
 *
 * The default setting is _decimal format (base 10)_.
 *
 * \see @ref StringComposer::BaseHex() \n
 *      @ref StringComposer::BaseOct() \n
 *      @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::BaseDec(StringComposer& sc)
{
  sc.base_ = Base::decimal;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print integer numbers in hexadecimal format (base 16).
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int value = 2997U;
 * sc << StringComposer::BaseHex << value;
 * // Result: "bb5"
 * ~~~
 *
 * The default setting is _decimal format (base 10)_.
 *
 * \see @ref StringComposer::BaseDec() \n
 *      @ref StringComposer::BaseOct() \n
 *      @ref StringComposer::ShowBase() and @ref StringComposer::NoShowBase() \n
 *      @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos() \n
 *      @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::BaseHex(StringComposer& sc)
{
  sc.base_ = Base::hexadecimal;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print integer numbers in octal format (base 8).
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int value = 2997U;
 * sc << StringComposer::BaseOct << value;
 * // Result: "5665"
 * ~~~
 *
 * The default setting is _decimal format (base 10)_.
 *
 * \see @ref StringComposer::BaseDec() \n
 *      @ref StringComposer::BaseHex()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::BaseOct(StringComposer& sc)
{
  sc.base_ = Base::octal;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print floating-point numbers in fix-point format.
 *
 * Usage:
 * ~~~{.cpp}
 * float value = 37.5;
 * sc << StringComposer::FixedFloat << value;
 * // Result: "37.500000" (note: default precision is 6)
 * ~~~
 *
 * The default setting is _automatic format_.
 *
 * \see @ref StringComposer::ScientificFloat() \n
 *      @ref StringComposer::HexFloat() \n
 *      @ref StringComposer::AutoFloat() \n
 *      @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos() \n
 *      @ref StringComposer::ShowPoint() and @ref StringComposer::NoShowPoint() \n
 *      @ref StringComposer::Precision()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::FixedFloat(StringComposer& sc)
{
  sc.floatFormat_ = FloatFormat::fixed;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print floating-point numbers in scientific format.
 *
 * Usage:
 * ~~~{.cpp}
 * float value = 37.5;
 * sc << StringComposer::ScientificFloat << value;
 * // Result: "3.750000e+01" (note: default precision is 6)
 * ~~~
 *
 * The default setting is _automatic format_.
 *
 * \see @ref StringComposer::FixedFloat() \n
 *      @ref StringComposer::HexFloat() \n
 *      @ref StringComposer::AutoFloat() \n
 *      @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos() \n
 *      @ref StringComposer::ShowPoint() and @ref StringComposer::NoShowPoint() \n
 *      @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase() \n
 *      @ref StringComposer::Precision()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::ScientificFloat(StringComposer& sc)
{
  sc.floatFormat_ = FloatFormat::scientific;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to print floating-point numbers in hexadecimal format.
 *
 * Usage:
 * ~~~{.cpp}
 * float value = 37.5;
 * sc << StringComposer::HexFloat << value;
 * // Result: "0x1.2cp+5"
 * ~~~
 *
 * The default setting is _automatic format_.
 *
 * \see @ref StringComposer::FixedFloat() \n
 *      @ref StringComposer::ScientificFloat() \n
 *      @ref StringComposer::AutoFloat() \n
 *      @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::HexFloat(StringComposer& sc)
{
  sc.floatFormat_ = FloatFormat::hex;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to automatically choose between fixed and scientific format to print
 *        floating-point numbers.
 *
 * The decision depends on the value that shall be printed and the configured precision. The exact behaviour depends
 * on the underlying C library, but it should be about like this:\n
 * If the exponent is less than -4 or larger than the configured precision, then scientific format will be used.
 *
 * Usage:
 * ~~~{.cpp}
 * float value;
 * sc << StringComposer::AutoFloat << value;
 * ~~~
 *
 * The default setting is _automatic format_.
 *
 * \see @ref StringComposer::FixedFloat() \n
 *      @ref StringComposer::ScientificFloat() \n
 *      @ref StringComposer::HexFloat() \n
 *      @ref StringComposer::ShowPos() and @ref StringComposer::NoShowPos() \n
 *      @ref StringComposer::ShowPoint() and @ref StringComposer::NoShowPoint() \n
 *      @ref StringComposer::Uppercase() and @ref StringComposer::NoUppercase() \n
 *      @ref StringComposer::Precision()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::AutoFloat(StringComposer& sc)
{
  sc.floatFormat_ = FloatFormat::automatic;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to align all output to the left.
 *
 * If the length of the output is less than the configured field-width, then the output will be padded on the right side
 * with white-spaces.
 *
 * Usage:
 * ~~~{.cpp}
 * std::string str = "1234";
 * sc << StringComposer::AlignLeft << StringComposer::Width(8) << str;
 * // Result: "1234    "
 * ~~~
 *
 * The default setting is _right alignment_.
 *
 * \see @ref StringComposer::AlignRight() \n
 *      @ref StringComposer::AlignRightPadZero() \n
 *      @ref StringComposer::Width()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::AlignLeft(StringComposer& sc)
{
  sc.align_ = Alignment::left;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to align all output to the right and pad with white-spaces on the left.
 *
 * If the length of the output is less than the configured field-width, then the output will be padded on the left side
 * with white-spaces.
 *
 * Usage:
 * ~~~{.cpp}
 * std::string str = "1234";
 * sc << StringComposer::AlignRight << StringComposer::Width(8) << str;
 * // Result: "    1234"
 * ~~~
 *
 * The default setting is _right alignment_.
 *
 * \see @ref StringComposer::AlignLeft() \n
 *      @ref StringComposer::AlignRightPadZero() \n
 *      @ref StringComposer::Width()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::AlignRight(StringComposer& sc)
{
  sc.align_ = Alignment::right;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to align all output to the right and pad with zeros or white-spaces on the
 *        left.
 *
 * If the length of the output is less than the configured field-width, then the output will be padded on the left side
 * with zeros (for figures) or white-spaces (for text).
 *
 * If there is a prefix (e.g. 0x), then the padding zeros are inserted behind the prefix.
 *
 * Usage:
 * ~~~{.cpp}
 * std::string str = "1234";
 * sc << StringComposer::AlignRightPadZero << StringComposer::Width(8) << str;
 * // Result: "    1234"
 *
 * sc.Clear();
 * uint32_t v = 12;
 * sc << StringComposer::Width(8) << v;
 * // Result: "00000012";
 * ~~~
 *
 * The default setting is _right alignment_.
 *
 * \see @ref StringComposer::AlignLeft() \n
 *      @ref StringComposer::AlignRight() \n
 *      @ref StringComposer::Width()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::AlignRightPadZero(StringComposer& sc)
{
  sc.align_ = Alignment::rightPadZero;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to prefix positive integer and floating-point numbers with a '+'-sign.
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int v = 12U;
 * sc << StringComposer::ShowPos << v;
 * // Result: +12
 * ~~~
 *
 * The default setting is _no prefix_ for positive integer and floating-point numbers.
 *
 * \see   @ref StringComposer::NoShowPos()
 *
 * \note  This has no effect on unsigned types.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::ShowPos(StringComposer& sc)
{
  sc.showPos_ = true;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer not to prefix positive integer and floating-point numbers with a '+'-sign.
 *
 * Usage:
 * ~~~{.cpp}
 * unsigned int v = 12U;
 * sc << StringComposer::NoShowPos << v;
 * // Result: 12
 * ~~~
 *
 * The default setting is _no prefix_ for positive integer and floating-point numbers.
 *
 * \see @ref StringComposer::NoShowPos()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::NoShowPos(StringComposer& sc)
{
  sc.showPos_ = false;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to always include a decimal point in floating-point numbers.
 *
 * Usage:
 * ~~~{.cpp}
 * float f = 12;
 * sc << StringComposer::ShowPoint << f;
 * // Result: "12."
 * ~~~
 *
 * The default setting is _to include a decimal point only if necessary_ in floating-point numbers.
 *
 * \see @ref StringComposer::NoShowPoint()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::ShowPoint(StringComposer& sc)
{
  sc.showPoint_ = true;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to include a decimal point in floating-point numbers only if necessary.
 *
 * Usage:
 * ~~~{.cpp}
 * float f = 12;
 * sc << StringComposer::NoShowPoint << f;
 * // Result: "12"
 * ~~~
 *
 * The default setting is _to include a decimal point only if necessary_ in floating-point numbers.
 *
 * \see @ref StringComposer::ShowPoint()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::NoShowPoint(StringComposer& sc)
{
  sc.showPoint_ = false;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to use uppercase characters in hexadecimal or floating-point output.
 *
 * Usage:
 * ~~~{.cpp}
 * uint32_t value = 12U;
 * sc << StringComposer::BaseHex << StringComposer::ShowBase << StringComposer::Uppercase << value;
 * // Result: 0XC
 * ~~~
 *
 * The default setting is _lowercase_.
 *
 * \see @ref StringComposer::NoUppercase()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::Uppercase(StringComposer& sc)
{
  sc.uppercase_ = true;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to use lowercase characters in hexadecimal or floating-point output.
 *
 * Usage:
 * ~~~{.cpp}
 * uint32_t value = 12U;
 * sc << StringComposer::BaseHex << StringComposer::ShowBase << StringComposer::NoUppercase << value;
 * // Result: 0xc
 * ~~~
 *
 * The default setting is _lowercase_.
 *
 * \see @ref StringComposer::Uppercase()
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * No-throw guarantee.
 *
 * __Thread cancellation safety:__\n
 * No cancellation point included.
 *
 * - - -
 *
 * \param sc
 * Reference to @ref StringComposer instance.
 *
 * \return
 * Reference to @ref StringComposer @p sc.
 */
inline StringComposer& StringComposer::NoUppercase(StringComposer& sc)
{
  sc.uppercase_ = false;
  return sc;
}

/**
 * \brief Configures a @ref StringComposer to pad the output with white-space characters up to a specific width.
 *
 * If the length of the output resulting from __any__ data streamed into the @ref StringComposer is less than the
 * configured field-width, then the output will be padded on the left or right side with white-spaces. The configured
 * alignment determines on which side the padding occurs.
 *
 * Usage:
 * ~~~{.cpp}
 * std::string str = "1234";
 * sc << StringComposer::AlignLeft << StringComposer::Width(8) << str;
 * // Result: "1234    "
 * ~~~
 *
 * The default setting is _zero_.
 *
 * \see @ref StringComposer::AlignLeft() \n
 *      @ref StringComposer::AlignRight() \n
 *      @ref StringComposer::AlignRightPadZero()
 *
 * \note   The configured width is not sticky. It will be reset to zero each time data is streamed into the
 *         @ref StringComposer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * In conjunction with `StringComposer::operator<<`: Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * In conjunction with `StringComposer::operator<<`: No cancellation point included.
 *
 * - - -
 *
 * \param w
 * Desired field width for __any__ data streamed into the @ref StringComposer.
 *
 * \return
 * A special object that can be streamed into a @ref StringComposer to set the field width.
 */
inline StringComposer::Width_ StringComposer::Width(int const w)
{
  return {w};
}

/**
 * \brief Configures a @ref StringComposer to use a certain precision when converting floating-point values.
 *
 * For fixed float format, this determines the number of decimal places.
 *
 * Usage:
 * ~~~{.cpp}
 * float v = 12;
 * sc << StringComposer::FixedFloat << StringComposer::Precision(3) << v;
 * // Result: "12.000"
 * ~~~
 *
 * The default setting is _6_.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * In conjunction with `StringComposer::operator<<`: Strong guarantee
 *
 * __Thread cancellation safety:__\n
 * In conjunction with `StringComposer::operator<<`: No cancellation point included.
 *
 * - - -
 *
 * \param p
 * Desired precision for conversion of floating-point numbers.
 *
 * \return
 * A special object that can be streamed into a @ref StringComposer to set the precision.
 */
inline StringComposer::Precision_ StringComposer::Precision(int const p)
{
  return {p};
}

/**
 * \brief Invokes a function that manipulates the configuration of a @ref StringComposer.
 *
 * - - -
 *
 * __Thread safety:__\n
 * The state of the @ref StringComposer object is modified. Any concurrent accesses are not safe.
 *
 * __Exception safety:__\n
 * Same as @p func
 *
 * __Thread cancellation safety:__\n
 * Same as @p func
 *
 * - - -
 *
 * \param func
 * Pointer to the function that shall be invoked on the @ref StringComposer instance.
 *
 * \return
 * The return value of @p func is returned. Typically this is a reference to the @ref StringComposer itself.
 */
inline StringComposer& StringComposer::operator<<(StringComposer& (*func)(StringComposer&))
{
  return func(*this);
}

} // namespace string
} // namespace gpcc

#endif // STRINGCOMPOSER_HPP_202407291957
