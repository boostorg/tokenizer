// Boost tokenizer examples  -------------------------------------------------//

// (c) Copyright John R. Bandela 2001. 

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org for updates, documentation, and revision history.

#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#if defined(BOOST_ENABLE_ASSERT_HANDLER)
# error "already defined BOOST_ENABLE_ASSERT_HANDLER somewhere"
#else
# define BOOST_ENABLE_ASSERT_HANDLER
#endif

#include <boost/array.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/tokenizer.hpp>
#include <boost/utility/string_view.hpp>

#include <boost/test/minimal.hpp>

#define PP_REQUIRE_EXCEPTION(statement, exception_type, error_message) \
  do {                                                                 \
    bool caught_exception__ = false;                                   \
    try {                                                              \
      { statement }                                                    \
    } catch (exception_type& e) {                                      \
      BOOST_REQUIRE(std::strcmp(e.what(), error_message) == 0);        \
      caught_exception__ = true;                                       \
    }                                                                  \
    BOOST_REQUIRE(caught_exception__);                                 \
  } while (false)

namespace boost {
  void assertion_failed(char const* expr, char const *function, char const* file, long line) {
    BOOST_THROW_EXCEPTION(std::runtime_error("assertion failed"));
  }
} // namespace boost

int test_main( int /*argc*/, char* /*argv*/[] )
{
  using namespace boost;

  // Use tokenizer
  {
    const std::string test_string = ";;Hello|world||-foo--bar;yow;baz|";
    std::string answer[] = { "Hello", "world",  "foo", "bar", "yow",  "baz" };
    typedef tokenizer<char_separator<char> > Tok;
    char_separator<char> sep("-;|");
    Tok t(test_string, sep);
    BOOST_REQUIRE(std::equal(t.begin(),t.end(),answer));
  }
  {
    const std::string test_string = ";;Hello|world||-foo--bar;yow;baz|";
    std::string answer[] = { "", "", "Hello", "|", "world", "|", "", "|", "",
                                            "foo", "", "bar", "yow", "baz", "|", "" };
    typedef tokenizer<char_separator<char> > Tok;
    char_separator<char> sep("-;", "|", boost::keep_empty_tokens);
    Tok t(test_string, sep);
    BOOST_REQUIRE(std::equal(t.begin(), t.end(), answer));
  }

  {
    const std::string test_string = "Field 1,\"embedded,comma\",quote \\\", escape \\\\";
    std::string answer[] = {"Field 1","embedded,comma","quote \""," escape \\"};
    typedef tokenizer<escaped_list_separator<char> > Tok;
    Tok t(test_string);
    BOOST_REQUIRE(std::equal(t.begin(),t.end(),answer));
  }

  {
    const std::string test_string = ",1,;2\\\";3\\;,4,5^\\,\'6,7\';";
    std::string answer[] = {"","1","","2\"","3;","4","5\\","6,7",""};
    typedef tokenizer<escaped_list_separator<char> > Tok;
    escaped_list_separator<char> sep("\\^",",;","\"\'");
    Tok t(test_string,sep);
    BOOST_REQUIRE(std::equal(t.begin(),t.end(),answer));
  }

  {
    const std::string test_string = "12252001";
    std::string answer[] = {"12","25","2001"};
    typedef tokenizer<offset_separator > Tok;
    boost::array<int,3> offsets = {{2,2,4}};
    offset_separator func(offsets.begin(),offsets.end());
    Tok t(test_string,func);
    BOOST_REQUIRE(std::equal(t.begin(),t.end(),answer));
  }

  // Use token_iterator_generator
  {
    const std::string test_string = "This,,is, a.test..";
    std::string answer[] = {"This","is","a","test"};
    typedef token_iterator_generator<char_delimiters_separator<char> >::type Iter;
    Iter begin = make_token_iterator<std::string>(test_string.begin(),
      test_string.end(),char_delimiters_separator<char>());
    Iter end;
    BOOST_REQUIRE(std::equal(begin,end,answer));
  }

  {
    const std::string test_string = "Field 1,\"embedded,comma\",quote \\\", escape \\\\";
    std::string answer[] = {"Field 1","embedded,comma","quote \""," escape \\"};
    typedef token_iterator_generator<escaped_list_separator<char> >::type Iter;
    Iter begin = make_token_iterator<std::string>(test_string.begin(),
      test_string.end(),escaped_list_separator<char>());
    Iter begin_c(begin);
    Iter end;
    BOOST_REQUIRE(std::equal(begin,end,answer));

    while(begin_c != end)
    {
       BOOST_REQUIRE(begin_c.at_end() == 0);
       ++begin_c;
    }
    BOOST_REQUIRE(begin_c.at_end());
  }

  {
    const std::string test_string = "12252001";
    std::string answer[] = {"12","25","2001"};
    typedef token_iterator_generator<offset_separator>::type Iter;
    boost::array<int,3> offsets = {{2,2,4}};
    offset_separator func(offsets.begin(),offsets.end());
    Iter begin = make_token_iterator<std::string>(test_string.begin(),
      test_string.end(),func);
    Iter end= make_token_iterator<std::string>(test_string.end(),
      test_string.end(),func);
    BOOST_REQUIRE(std::equal(begin,end,answer));
  }

  // Test copying
  {
    const std::string test_string = "abcdef";
    token_iterator_generator<offset_separator>::type beg, end, other;
    boost::array<int,3> ar = {{1,2,3}};
    offset_separator f(ar.begin(),ar.end());
    beg = make_token_iterator<std::string>(test_string.begin(),test_string.end(),f);

    ++beg;
    other = beg;
    ++other;

    BOOST_REQUIRE(*beg=="bc");
    BOOST_REQUIRE(*other=="def");

    other = make_token_iterator<std::string>(test_string.begin(),
        test_string.end(),f);

    BOOST_REQUIRE(*other=="a");
  }

  // Test token_iterator
  {
    // compare equality of two iterators: valid & valid (same target string)
    {
      const std::string test_string = "abc";
      typedef char_separator<char> separator_type;
      typedef token_iterator_generator<separator_type>::type token_iterator_type;
      const std::string::const_iterator first = boost::begin(test_string);
      const std::string::const_iterator last = boost::end(test_string);
      const separator_type separator;
      const token_iterator_type a = make_token_iterator<std::string>(first, last, separator);
      const token_iterator_type b = make_token_iterator<std::string>(first, last, separator);
      BOOST_REQUIRE(a == b);
    }

    // compare equality of two iterators: valid & valid (same but partial target string)
    {
      const std::string test_string = "abc";
      typedef char_separator<char> separator_type;
      typedef token_iterator_generator<separator_type>::type token_iterator_type;
      const std::string::const_iterator first = boost::begin(test_string);
      std::string::const_iterator last = boost::end(test_string);
      const token_iterator_type a = make_token_iterator<std::string>(first, last, separator_type());
      const token_iterator_type b = make_token_iterator<std::string>(first, --last, separator_type());
      BOOST_REQUIRE(a != b);
    }

    // compare equality of two iterators: valid & valid (same but partial target string)
    {
      const std::string test_string = "abc,def";
      typedef char_delimiters_separator<char> separator_type;
      typedef token_iterator_generator<separator_type>::type token_iterator_type;
      const std::string::const_iterator first = boost::begin(test_string);
      std::string::const_iterator last = boost::end(test_string);
      const token_iterator_type a = make_token_iterator<std::string>(first, last, separator_type());
      const token_iterator_type b = make_token_iterator<std::string>(first, --last, separator_type());
      BOOST_REQUIRE(a != b);
    }

    // compare equality of two iterators: invalid & valid
    {
      const std::string empty_string = "";
      const std::string non_empty_string = "abc";
      typedef char_delimiters_separator<char> separator_type;
      typedef token_iterator_generator<separator_type>::type token_iterator_type;
      const token_iterator_type a = make_token_iterator<std::string>(boost::begin(empty_string), boost::end(empty_string), separator_type());
      const token_iterator_type b = make_token_iterator<std::string>(boost::begin(non_empty_string), boost::end(non_empty_string), separator_type());
      BOOST_REQUIRE(a != b);
    }
  }

  // Test escaped_list_separator
  {
    // input contains "\n"
    {
      const std::string test_string = "\\n";
      const std::string answer[] = {"\n"};
      tokenizer<escaped_list_separator<char> > tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // input ends with escape
    {
      const std::string test_string = "\\";
      tokenizer<escaped_list_separator<char> > t(test_string);

      PP_REQUIRE_EXCEPTION(
        { std::distance(boost::begin(t), boost::end(t)); },
        boost::escaped_list_error,
        "cannot end with escape"
      );
    }

    // input contains unknown escape sequence
    {
      const std::string test_string = "\\q";
      tokenizer<escaped_list_separator<char> > t(test_string);

      PP_REQUIRE_EXCEPTION(
        { std::distance(boost::begin(t), boost::end(t)); },
        boost::escaped_list_error,
        "unknown escape sequence"
      );
    }
  }

  // Test default constructed offset_separator
  {
    // use std::string content
    { 
      typedef std::string string_type;
      const string_type test_string = "1234567";
      const string_type answer[] = {"1", "2", "3", "4", "5", "6", "7"};
      tokenizer<offset_separator, string_type::const_iterator, string_type> tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // use boost::string_view content
    { 
      typedef boost::string_view string_type;
      const string_type test_string = "1234567";
      const string_type answer[] = {"1", "2", "3", "4", "5", "6", "7"};
      tokenizer<offset_separator, string_type::const_iterator, string_type> tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

  #if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
    // use std::string_view content
    { 
      typedef std::string_view string_type;
      const string_type test_string = "1234567";
      const string_type answer[] = {"1", "2", "3", "4", "5", "6", "7"};
      tokenizer<offset_separator, string_type::const_iterator, string_type> tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }
  #endif
  }

  // Test non-default constructed offset_separator
  {
    const std::string test_string = "1234567";

    // empty offsets, wrap_offsets = false, return_partial_last = false
    {
      const int offset = 0;
      typedef offset_separator separator_type;
      offset_separator separator(&offset, &offset, false, false);
      typedef token_iterator_generator<separator_type>::type token_iterator_type;

      PP_REQUIRE_EXCEPTION(
        { make_token_iterator<std::string>(boost::begin(test_string), boost::end(test_string), separator); },
        std::runtime_error,
        "assertion failed"
      );
    }

    // wrap_offsets = false, return_partial_last = false
    {
      const std::string answer[] = {"1", "234"};

      boost::array<int,3> offsets = {{1, 3, 5}};
      typedef offset_separator separator_type;
      offset_separator separator(offsets.begin(), offsets.end(), false, false);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // wrap_offsets = false, return_partial_last = true
    {
      {
        const std::string answer[] = {"1", "234", "567"};

        boost::array<int,3> offsets = {{1, 3, 5}};
        typedef offset_separator separator_type;
        offset_separator separator(offsets.begin(), offsets.end(), false, true);
        tokenizer<separator_type> tokenizer(test_string, separator);
        BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
      }

      {
        const std::string answer[] = {"12345"};

        boost::array<int,1> offsets = {{5}};
        typedef offset_separator separator_type;
        offset_separator separator(offsets.begin(), offsets.end(), false, true);
        tokenizer<separator_type> tokenizer(test_string, separator);
        BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
      }
    }

    // wrap_offsets = true, return_partial_last = false
    {
      const std::string answer[] = {"1", "234"};

      boost::array<int,3> offsets = {{1, 3, 5}};
      typedef offset_separator separator_type;
      offset_separator separator(offsets.begin(), offsets.end(), true, false);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }
  }

  // Test default constructed char_separator
  {
    // use std::string content
    {
      typedef std::string string_type;
      typedef tokenizer<
        char_separator<string_type::value_type>,
        string_type::const_iterator,
        string_type
      > tokenizer_type;
      const string_type test_string = ";Hello|world-";
      const string_type answer[] = {";", "Hello", "|", "world", "-"};
      tokenizer_type tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // use boost::string_view content
    {
      typedef boost::string_view string_type;
      typedef tokenizer<
        char_separator<string_type::value_type>,
        string_type::const_iterator,
        string_type
      > tokenizer_type;
      const string_type test_string = ";Hello|world-";
      const string_type answer[] = {";", "Hello", "|", "world", "-"};
      tokenizer_type tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

  #if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
    // use std::string_view content
    {
      typedef std::string_view string_type;
      typedef tokenizer<
        char_separator<string_type::value_type>,
        string_type::const_iterator,
        string_type
      > tokenizer_type;
      const string_type test_string = ";Hello|world-";
      const string_type answer[] = {";", "Hello", "|", "world", "-"};
      tokenizer_type tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }
  #endif
  }

  // Test non-default contstructed char_separator
  {
    const std::string test_string = ";Hello||world-";

    // dropped_delims = non-null, kept_delims = null, empty_tokens = drop_empty_tokens
    {
      const std::string answer[] = {"Hello||world"};
      typedef char_separator<char> separator_type;
      separator_type separator("-;", 0, boost::drop_empty_tokens);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // dropped_delims = non-null, kept_delims = null, empty_tokens = keep_empty_tokens
    {
      const std::string answer[] = {"", "Hello", "", "world", ""};
      typedef char_separator<char> separator_type;
      separator_type separator("-;|", 0, boost::keep_empty_tokens);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // dropped_delims = non-null, kept_delims = non-null, empty_tokens = drop_empty_tokens
    {
      const std::string answer[] = {"Hello", "|", "|", "world"};
      typedef char_separator<char> separator_type;
      separator_type separator("-;", "|", boost::drop_empty_tokens);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // dropped_delims = non-null, kept_delims = non-null, empty_tokens = keep_empty_tokens
    {
      const std::string answer[] = {"", "Hello", "|", "", "|", "world", ""};
      typedef char_separator<char> separator_type;
      separator_type separator("-;", "|", boost::keep_empty_tokens);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // dropped_delims = non-null (empty), kept_delims = null, empty_tokens = drop_empty_tokens
    {
      const std::string answer[] = {";Hello||world-"};
      typedef char_separator<char> separator_type;
      separator_type separator("", 0, boost::keep_empty_tokens);
      tokenizer<separator_type> tokenizer(test_string, separator);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }
  }

  // Test default constructed char_delimiters_separator  
  {
    // use std::string content
    {
      typedef std::string string_type;
      typedef tokenizer<
        char_delimiters_separator<string_type::value_type>,
        string_type::const_iterator,
        string_type
      > tokenizer_type;
      const string_type test_string = "This,,is, a.test..";
      const string_type answer[] = {"This","is","a","test"};
      tokenizer_type tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // use boost::string_view content
    {
      typedef boost::string_view string_type;
      typedef tokenizer<
        char_delimiters_separator<string_type::value_type>,
        string_type::const_iterator,
        string_type
      > tokenizer_type;
      const string_type test_string = "This,,is, a.test..";
      const string_type answer[] = {"This","is","a","test"};
      tokenizer_type tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

  #if !defined(BOOST_NO_CXX17_HDR_STRING_VIEW)
    // use std::string_view content
    {
      typedef std::string_view string_type;
      typedef tokenizer<
        char_delimiters_separator<string_type::value_type>,
        string_type::const_iterator,
        string_type
      > tokenizer_type;
      const string_type test_string = "This,,is, a.test..";
      const string_type answer[] = {"This","is","a","test"};
      tokenizer_type tokenizer(test_string);
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }
  #endif
  }

  // Test non-default constructed char_delimiters_separator
  {
    const std::string test_string = "how,are you, doing?";

    // return_delims = true, returnable = non-null, nonreturnable = non-null
    {
      const std::string answer[] = {"how",",","are you",","," doing"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(true,",","?"));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = true, returnable = non-null, nonreturnable = non-null (empty)
    {
      const std::string answer[] = {"how",",","are you",","," doing?"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(true,",",""));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = true, returnable = non-null (empty), nonreturnable = non-null
    {
      const std::string answer[] = {"how,are you, doing"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(true,"","?"));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = true, returnable = non-null (empty), nonreturnable = non-null (empty)
    {
      const std::string answer[] = {"how,are you, doing?"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(true,"",""));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = false, returnable = non-null, nonreturnable = non-null
    {
      const std::string answer[] = {"how","are you"," doing"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(false,",","?"));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = false, returnable = non-null, nonreturnable = non-null (empty)
    {
      const std::string answer[] = {"how","are you"," doing?"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(false,",",""));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = false, returnable = non-null (empty), nonreturnable = non-null
    {
      const std::string answer[] = {"how,are you, doing"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(false,"","?"));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }

    // return_delims = false, returnable = non-null (empty), nonreturnable = non-null (emppty)
    {
      const std::string answer[] = {"how,are you, doing?"};
      tokenizer<> tokenizer(test_string,char_delimiters_separator<char>(false,"",""));
      BOOST_REQUIRE(boost::range::equal(tokenizer, answer));
    }
  }

  // Test iterator operations
  {
    const std::string test_string;

    {
      // increment invalid iterator
      typedef tokenizer<> tokenizer_type;
      tokenizer_type tokenizer(test_string);
      tokenizer_type::iterator first = boost::begin(tokenizer);
      PP_REQUIRE_EXCEPTION(
        { std::advance(first, 1); },
        std::runtime_error,
        "assertion failed"
      );
    }

    {
      // dereference invalid iterator
      typedef tokenizer<> tokenizer_type;
      tokenizer_type tokenizer(test_string);
      const tokenizer_type::iterator first = boost::begin(tokenizer);
      PP_REQUIRE_EXCEPTION(
        { *first; },
        std::runtime_error,
        "assertion failed"
      );
    }
  }

  return 0;
}

