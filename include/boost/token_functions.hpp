// Boost token_functions.hpp  ------------------------------------------------//

// Copyright John R. Bandela 2001.

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/tokenizer/ for documentation.

// Revision History:
// 01 Oct 2004   Joaquin M Lopez Munoz
//      Workaround for a problem with string::assign in msvc-stlport
// 06 Apr 2004   John Bandela
//      Fixed a bug involving using char_delimiter with a true input iterator
// 28 Nov 2003   Robert Zeh and John Bandela
//      Converted into "fast" functions that avoid using += when
//      the supplied iterator isn't an input_iterator; based on
//      some work done at Archelon and a version that was checked into
//      the boost CVS for a short period of time.
// 20 Feb 2002   John Maddock
//      Removed using namespace std declarations and added
//      workaround for BOOST_NO_STDC_NAMESPACE (the library
//      can be safely mixed with regex).
// 06 Feb 2002   Jeremy Siek
//      Added char_separator.
// 02 Feb 2002   Jeremy Siek
//      Removed tabs and a little cleanup.


#ifndef BOOST_TOKEN_FUNCTIONS_JRB120303_HPP_
#define BOOST_TOKEN_FUNCTIONS_JRB120303_HPP_

#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/core/addressof.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/mpl/if.hpp>
#include <boost/range/distance.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility/string_view.hpp>

#include <cctype>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#if !defined(BOOST_NO_CXX11_NOEXCEPT)
# define BOOST_TOKENIZER_NOEXCEPT BOOST_NOEXCEPT
# define BOOST_TOKENIZER_NOEXCEPT_EXPR(expr) BOOST_NOEXCEPT_EXPR(expr)
#else
# define BOOST_TOKENIZER_NOEXCEPT BOOST_NOEXCEPT_OR_NOTHROW
# define BOOST_TOKENIZER_NOEXCEPT_EXPR(expr) BOOST_NOEXCEPT_OR_NOTHROW
#endif

#if !defined(BOOST_NO_CWCTYPE)
# include <cwctype>
#endif

#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
# include <string_view> 
#endif

//
// the following must not be macros if we are to prefix them
// with std:: (they shouldn't be macros anyway...)
//
#ifdef ispunct
#  undef ispunct
#endif
#ifdef iswpunct
#  undef iswpunct
#endif
#ifdef isspace
#  undef isspace
#endif
#ifdef iswspace
#  undef iswspace
#endif
//
// fix namespace problems:
//
#ifdef BOOST_NO_STDC_NAMESPACE
namespace std{
 using ::ispunct;
 using ::isspace;
#if !defined(BOOST_NO_CWCTYPE)
 using ::iswpunct;
 using ::iswspace;
#endif
}
#endif

namespace boost{
  //===========================================================================
  // The escaped_list_separator class. Which is a model of TokenizerFunction
  // An escaped list is a super-set of what is commonly known as a comma
  // separated value (csv) list.It is separated into fields by a comma or
  // other character. If the delimiting character is inside quotes, then it is
  // counted as a regular character.To allow for embedded quotes in a field,
  // there can be escape sequences using the \ much like C.
  // The role of the comma, the quotation mark, and the escape
  // character (backslash \), can be assigned to other characters.

  struct escaped_list_error : public std::runtime_error{
    explicit escaped_list_error(const char* what_arg) BOOST_TOKENIZER_NOEXCEPT
      : std::runtime_error(what_arg) { }
  };

  namespace tokenizer_detail {
  //===========================================================================
  // Tokenizer was broken for wide character separators, at least on Windows, since
  // CRT functions isspace etc only expect values in [0, 0xFF]. Debug build asserts
  // if higher values are passed in. The traits extension class should take care of this.
  // Assuming that the conditional will always get optimized out in the function
  // implementations, argument types are not a problem since both forms of character classifiers
  // expect an int.

#if !defined(BOOST_NO_CWCTYPE)
  template<typename traits, int N>
  struct traits_extension_details : public traits {
    typedef typename traits::char_type char_type;
    static bool isspace(char_type c) BOOST_TOKENIZER_NOEXCEPT
    {
       return std::iswspace(c) != 0;
    }
    static bool ispunct(char_type c) BOOST_TOKENIZER_NOEXCEPT
    {
       return std::iswpunct(c) != 0;
    }
  };

  template<typename traits>
  struct traits_extension_details<traits, 1> : public traits {
    typedef typename traits::char_type char_type;
    static bool isspace(char_type c) BOOST_TOKENIZER_NOEXCEPT
    {
       return std::isspace(c) != 0;
    }
    static bool ispunct(char_type c) BOOST_TOKENIZER_NOEXCEPT
    {
       return std::ispunct(c) != 0;
    }
  };
#endif

  // In case there is no cwctype header, we implement the checks manually.
  // We make use of the fact that the tested categories should fit in ASCII.
  template<typename traits>
  struct traits_extension : public traits {
    typedef typename traits::char_type char_type;
    static bool isspace(char_type c) BOOST_TOKENIZER_NOEXCEPT
    {
#if !defined(BOOST_NO_CWCTYPE)
      return traits_extension_details<traits, sizeof(char_type)>::isspace(c);
#else
      return static_cast< unsigned >(c) <= 255 && std::isspace(c) != 0;
#endif
    }

    static bool ispunct(char_type c) BOOST_TOKENIZER_NOEXCEPT
    {
#if !defined(BOOST_NO_CWCTYPE)
      return traits_extension_details<traits, sizeof(char_type)>::ispunct(c);
#else
      return static_cast< unsigned >(c) <= 255 && std::ispunct(c) != 0;
#endif
    }
  };

  template <typename CharT, typename Traits, typename Allocator, typename Iterator, typename Sentinel>
  void assign(std::basic_string<CharT, Traits, Allocator>& str, Iterator first, Sentinel last) {
    str.assign(first, last);
  }

  template <typename CharT, typename Traits, typename Allocator>
  void append(std::basic_string<CharT, Traits, Allocator>& str, CharT ch) {
    str.push_back(ch);
  }

  template <typename CharT, typename Traits, typename Allocator>
  void clear(std::basic_string<CharT, Traits, Allocator>& str) BOOST_TOKENIZER_NOEXCEPT {
    str.clear();
  }

  template <typename CharT, typename Traits, typename Iterator, typename Sentinel>
  void assign(boost::basic_string_view<CharT, Traits>& sv, Iterator first, Sentinel last) BOOST_TOKENIZER_NOEXCEPT {
    boost::basic_string_view<CharT, Traits>(
      boost::addressof(*first), boost::distance(first, last)
    ).swap(sv);
  }

  template <typename CharT, typename Traits>
  void clear(boost::basic_string_view<CharT, Traits>& sv) BOOST_TOKENIZER_NOEXCEPT {
    sv.clear();
  }

#if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
  template <typename CharT, typename Traits, typename Iterator, typename Sentinel>
  void assign(std::basic_string_view<CharT, Traits>& sv, Iterator first, Sentinel last) BOOST_TOKENIZER_NOEXCEPT {
    sv = std::basic_string_view<CharT, Traits>(
      std::addressof(*first), std::distance(first, last)
    );
  }

  template <typename CharT, typename Traits>
  void clear(std::basic_string_view<CharT, Traits>& sv) BOOST_TOKENIZER_NOEXCEPT {
    sv = std::basic_string_view<CharT, Traits>();
  }
#endif
  } // namespace tokenizer_detail

  // The out of the box GCC 2.95 on cygwin does not have a char_traits class.
  // MSVC does not like the following typename
  template <class Char,
    class Traits = BOOST_DEDUCED_TYPENAME std::basic_string<Char>::traits_type >
  class escaped_list_separator {
  private:
    typedef std::basic_string<Char,Traits> string_type;

    string_type  escape_;
    string_type  c_;
    string_type  quote_;
    bool last_;

    bool is_escape(Char e) const BOOST_TOKENIZER_NOEXCEPT {
      return escape_.find(e) != string_type::npos;
    }
    bool is_c(Char e) const BOOST_TOKENIZER_NOEXCEPT {
      return c_.find(e) != string_type::npos;
    }
    bool is_quote(Char e) const BOOST_TOKENIZER_NOEXCEPT {
      return quote_.find(e) != string_type::npos;
    }
    template <typename iterator, typename Token>
    void do_escape(iterator& next,iterator end,Token& tok) {
      using namespace tokenizer_detail;

      if (++next == end)
        BOOST_THROW_EXCEPTION(escaped_list_error("cannot end with escape"));
      if (Traits::eq(*next,'n')) {
        append(tok, '\n');
        return;
      }
      else if (is_quote(*next)) {
        append(tok, *next);
        return;
      }
      else if (is_c(*next)) {
        append(tok, *next);
        return;
      }
      else if (is_escape(*next)) {
        append(tok, *next);
        return;
      }
      else
        BOOST_THROW_EXCEPTION(escaped_list_error("unknown escape sequence"));
    }

    public:

    explicit escaped_list_separator(Char  e = '\\',
                                    Char c = ',',Char  q = '\"')
      : escape_(1,e), c_(1,c), quote_(1,q), last_(false) { }

    escaped_list_separator(string_type e, string_type c, string_type q)
      : escape_(boost::move(e)), c_(boost::move(c)), quote_(boost::move(q)), last_(false) { }

    void reset() BOOST_TOKENIZER_NOEXCEPT { last_ = false; }

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next,InputIterator end,Token& tok) {
      using namespace tokenizer_detail;

      bool bInQuote = false;
      clear(tok);

      if (next == end) {
        if (last_) {
          last_ = false;
          return true;
        }
        else
          return false;
      }
      last_ = false;
      for (;next != end;++next) {
        if (is_escape(*next)) {
          do_escape(next, end, tok);
        }
        else if (is_c(*next)) {
          if (!bInQuote) {
            // If we are not in quote, then we are done
            ++next;
            // The last character was a c, that means there is
            // 1 more blank field
            last_ = true;
            return true;
          }
          else {
            append(tok, *next);
          }
        }
        else if (is_quote(*next)) {
          bInQuote=!bInQuote;
        }
        else {
          append(tok, *next);
        }
      }
      return true;
    }
  };

  //===========================================================================
  // The offset_separator class, which is a model of TokenizerFunction.
  // Offset breaks a string into tokens based on a range of offsets

  class offset_separator {
  private:

    std::vector<int> offsets_;
    unsigned int current_offset_;
    bool wrap_offsets_;
    bool return_partial_last_;

  public:
    template <typename Iter>
    offset_separator(Iter begin, Iter end, bool wrap_offsets = true,
                     bool return_partial_last = true)
      : offsets_(begin,end), current_offset_(0),
        wrap_offsets_(wrap_offsets),
        return_partial_last_(return_partial_last) { }

    offset_separator()
      : offsets_(1,1), current_offset_(0),
        wrap_offsets_(true), return_partial_last_(true) { }

    void reset() BOOST_TOKENIZER_NOEXCEPT {
      current_offset_ = 0;
    }

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next, InputIterator end, Token& tok)
    {
      using namespace tokenizer_detail;

      BOOST_ASSERT(!offsets_.empty());

      clear(tok);
      InputIterator start(next);

      if (next == end)
        return false;

      if (current_offset_ == offsets_.size())
      {
        if (wrap_offsets_)
          current_offset_=0;
        else
          return false;
      }

      int c = offsets_[current_offset_];
      int i = 0;
      for (; i < c; ++i) {
        if (next == end)break;
        ++next;
      }
      assign(tok, start, next);

      if (!return_partial_last_)
        if (i < (c-1) )
          return false;

      ++current_offset_;
      return true;
    }
  };


  //===========================================================================
  // The char_separator class breaks a sequence of characters into
  // tokens based on the character delimiters (very much like bad old
  // strtok). A delimiter character can either be kept or dropped. A
  // kept delimiter shows up as an output token, whereas a dropped
  // delimiter does not.

  // This class replaces the char_delimiters_separator class. The
  // constructor for the char_delimiters_separator class was too
  // confusing and needed to be deprecated. However, because of the
  // default arguments to the constructor, adding the new constructor
  // would cause ambiguity, so instead I deprecated the whole class.
  // The implementation of the class was also simplified considerably.

  enum empty_token_policy { drop_empty_tokens, keep_empty_tokens };

  // The out of the box GCC 2.95 on cygwin does not have a char_traits class.
  template <typename Char,
    typename Tr = BOOST_DEDUCED_TYPENAME std::basic_string<Char>::traits_type >
  class char_separator
  {
    typedef tokenizer_detail::traits_extension<Tr> Traits;
    typedef std::basic_string<Char,Tr> string_type;
  public:
    explicit
    char_separator(const Char* dropped_delims,
                   const Char* kept_delims = 0,
                   empty_token_policy empty_tokens = drop_empty_tokens)
      : m_kept_delims(kept_delims, (kept_delims ? Traits::length(kept_delims) : 0)),
        m_dropped_delims(dropped_delims),
        m_use_ispunct(false),
        m_use_isspace(false),
        m_empty_tokens(empty_tokens),
        m_output_done(false)
    { }

                // use ispunct() for kept delimiters and isspace for dropped.
    explicit
    char_separator()
      : m_use_ispunct(true),
        m_use_isspace(true),
        m_empty_tokens(drop_empty_tokens),
        m_output_done(false) { }

    void reset() BOOST_TOKENIZER_NOEXCEPT { }

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next, InputIterator end, Token& tok)
    {
      using namespace tokenizer_detail;

      clear(tok);

      // skip past all dropped_delims
      if (m_empty_tokens == drop_empty_tokens)
        for (; next != end  && is_dropped(*next); ++next)
          { }

      InputIterator start(next);

      if (m_empty_tokens == drop_empty_tokens) {
        if (next == end) {
          return false;
        }

        // if we are on a kept_delims move past it and stop
        if (is_kept(*next)) {
          ++next;
        } else {
          // append all the non delim characters
          while (next != end && !is_dropped(*next) && !is_kept(*next)) {
            ++next;
          }
        }
      }
      else { // m_empty_tokens == keep_empty_tokens
        // Handle empty token at the end
        if (next == end) {
          if (m_output_done == false) {
            m_output_done = true;
            assign(tok, start, next);
            return true;
          }
          else {
            return false;
          }
        }

        if (is_kept(*next)) {
          if (m_output_done == false) {
            m_output_done = true;
          } else {
            ++next;
            m_output_done = false;
          }
        }
        else if (m_output_done == false && is_dropped(*next)) {
          m_output_done = true;
        }
        else {
          if (is_dropped(*next)) {
            start = ++next;
          }
          while (next != end && !is_dropped(*next) && !is_kept(*next)) {
            ++next;
          }
          m_output_done = true;
        }
      }
      assign(tok, start, next);
      return true;
    }

  private:
    string_type m_kept_delims;
    string_type m_dropped_delims;
    bool m_use_ispunct;
    bool m_use_isspace;
    empty_token_policy m_empty_tokens;
    bool m_output_done;

    bool is_kept(Char E) const BOOST_TOKENIZER_NOEXCEPT
    {
      if (m_kept_delims.length())
        return m_kept_delims.find(E) != string_type::npos;
      else if (m_use_ispunct) {
        return Traits::ispunct(E) != 0;
      } else
        return false;
    }
    bool is_dropped(Char E) const BOOST_TOKENIZER_NOEXCEPT
    {
      if (m_dropped_delims.length())
        return m_dropped_delims.find(E) != string_type::npos;
      else if (m_use_isspace) {
        return Traits::isspace(E) != 0;
      } else
        return false;
    }
  };

  //===========================================================================
  // The following class is DEPRECATED, use class char_separators instead.
  //
  // The char_delimiters_separator class, which is a model of
  // TokenizerFunction.  char_delimiters_separator breaks a string
  // into tokens based on character delimiters. There are 2 types of
  // delimiters. returnable delimiters can be returned as
  // tokens. These are often punctuation. nonreturnable delimiters
  // cannot be returned as tokens. These are often whitespace

  // The out of the box GCC 2.95 on cygwin does not have a char_traits class.
  template <class Char,
    class Tr = BOOST_DEDUCED_TYPENAME std::basic_string<Char>::traits_type >
  class char_delimiters_separator {
  private:

    typedef tokenizer_detail::traits_extension<Tr> Traits;
    typedef std::basic_string<Char,Tr> string_type;
    string_type returnable_;
    string_type nonreturnable_;
    bool return_delims_;
    bool no_ispunct_;
    bool no_isspace_;

    bool is_ret(Char E) const BOOST_TOKENIZER_NOEXCEPT
    {
      if (returnable_.length())
        return  returnable_.find(E) != string_type::npos;
      else{
        if (no_ispunct_) {return false;}
        else{
          int r = Traits::ispunct(E);
          return r != 0;
        }
      }
    }
    bool is_nonret(Char E) const BOOST_TOKENIZER_NOEXCEPT
    {
      if (nonreturnable_.length())
        return  nonreturnable_.find(E) != string_type::npos;
      else{
        if (no_isspace_) {return false;}
        else{
          int r = Traits::isspace(E);
          return r != 0;
        }
      }
    }

  public:
    explicit char_delimiters_separator(bool return_delims = false,
                                       const Char* returnable = 0,
                                       const Char* nonreturnable = 0)
      : returnable_(returnable, (returnable ? Traits::length(returnable) : 0)),
        nonreturnable_(nonreturnable, (returnable ? Traits::length(nonreturnable) : 0)),
        return_delims_(return_delims), no_ispunct_(returnable!=0),
        no_isspace_(nonreturnable!=0) { }

    void reset() BOOST_TOKENIZER_NOEXCEPT { }

  public:

    template <typename InputIterator, typename Token>
    bool operator()(InputIterator& next, InputIterator end,Token& tok) {
      using namespace tokenizer_detail;

      clear(tok);

      // skip past all nonreturnable delims
      // skip past the returnable only if we are not returning delims
      for (;next!=end && ( is_nonret(*next) || (is_ret(*next)
         && !return_delims_ ) );++next) { }

      if (next == end) {
        return false;
      }

      const InputIterator start(next);
      // if we are to return delims and we are one a returnable one
      // move past it and stop
      if (is_ret(*next)) {
        ++next;
      }
      else {
        // append all the non delim characters
        while (next != end && !is_nonret(*next) && !is_ret(*next)) {
          ++next;
        }
      }
      assign(tok, start, next);

      return true;
    }
  };
} //namespace boost

#endif