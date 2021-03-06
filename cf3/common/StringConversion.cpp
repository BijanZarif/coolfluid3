// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <iostream>
#include <cstdlib>
#include <limits>
#include <iomanip>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/split.hpp>

#include "common/URI.hpp"
#include "common/BasicExceptions.hpp"
#include "common/StringConversion.hpp"
#include "common/UUCount.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

////////////////////////////////////////////////////////////////////////////////

  template <>
  Common_API std::string to_str<bool> (const bool & v)
  {
    return v ? "true" : "false";
  }

#ifdef _LIBCPP_VERSION
  // libc++ defines following types as different from bool, whereas libstdc++ sees it as duplicate types
  template <>
  Common_API std::string to_str< std::vector<bool>::reference >( const std::vector<bool>::reference & v )
  {
    return v ? "true" : "false";
  }

  template <>
  Common_API std::string to_str< std::vector<bool>::const_reference >( const std::vector<bool>::const_reference & v )
  {
    return v ? "true" : "false";
  }
#endif

  template <>
  Common_API std::string to_str<int> (const int & v)
  {
    return boost::lexical_cast<std::string>(v);
  }

  template <>
  Common_API std::string to_str<long> (const long & v)
  {
    return boost::lexical_cast<std::string>(v);
  }

  template <>
  Common_API std::string to_str<long long> (const long long & v)
  {
    return boost::lexical_cast<std::string>(v);
  }

  template <>
  Common_API std::string to_str<unsigned> (const unsigned & v)
  {
    return boost::lexical_cast<std::string>(v);
  }

  template <>
  Common_API std::string to_str<unsigned long> (const unsigned long & v)
  {
    return boost::lexical_cast<std::string>(v);
  }

  template <>
  Common_API std::string to_str<unsigned long long> (const unsigned  long long & v)
  {
    return boost::lexical_cast<std::string>(v);
  }

  template <>
  Common_API std::string to_str<float> (const float & v)
  {
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(std::numeric_limits<float>::digits10+1);
    oss << v;
    return oss.str();
  }

  template <>
  Common_API std::string to_str<double> (const double & v)
  {
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(std::numeric_limits<double>::digits10+1);
    oss << v;
    return oss.str();
  }


  template <>
  Common_API std::string to_str<URI> (const URI & v)
  {
    return v.string();
  }

  template <>
  Common_API std::string to_str<UUCount> (const UUCount & v)
  {
    return v.string();
  }

  template <>
  Common_API std::string to_str<std::string> (const std::string & v)
  {
    return v;
  }

  template <>
  Common_API std::string to_str< std::vector<bool> > (const std::vector<bool> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  template <>
  Common_API std::string to_str< std::vector<int> > (const std::vector<int> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  template <>
  Common_API std::string to_str< std::vector<unsigned long> > (const std::vector<unsigned long> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  template <>
  Common_API std::string to_str< std::vector<Uint> > (const std::vector<Uint> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  template <>
  Common_API std::string to_str<std::vector<Real> > (const std::vector<Real> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  template <>
  Common_API std::string to_str<std::vector<URI> > (const std::vector<URI> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  template <>
  Common_API std::string to_str<std::vector<std::string> > (const std::vector<std::string> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += v[i] + " ";
      s += v.back();
    }
    return s;
  }


  template <>
  Common_API std::string to_str<std::vector<UUCount> > (const std::vector<UUCount> & v)
  {
    std::string s = "";
    if (v.size())
    {
      for (Uint i=0; i<v.size()-1; ++i)
        s += to_str(v[i]) + " ";
      s += to_str(v.back());
    }
    return s;
  }

  ////////////////////////////////////////////////////////////////////////////////

  /// Converts from std::string
  /// Don't use this to convert to a char, use c_str for that.
  /// Typical use is to convert to numbers.
  /// @param str string to convert from
  /// @return converter type
  template <>
  Common_API bool from_str<bool> (const std::string& str)
  {
    bool match = false;
    boost::algorithm::is_equal test_equal;

    if ( test_equal(str,"true") ||
         test_equal(str,"True") ||
         test_equal(str,"on")   ||
         test_equal(str,"1")     )
    {
      return true;
    }

    if ( test_equal(str,"false") ||
         test_equal(str,"False") ||
         test_equal(str,"off")   ||
         test_equal(str,"0")      )
    {
      return false;
    }

    if (!match)
      throw ParsingFailed (FromHere(), "Incorrect option conversion to bool of string [" + str + "]" );
    return true;
  }

  template <>
  Common_API int from_str<int> (const std::string& str)
  {
    return boost::lexical_cast<int> (str );
  }

  template <>
  Common_API long from_str<long> (const std::string& str)
  {
    return boost::lexical_cast<long> (str );
  }

  template <>
  Common_API long long from_str<long long> (const std::string& str)
  {
    return boost::lexical_cast<long long> (str );
  }

  template <>
  Common_API unsigned from_str<unsigned> (const std::string& str)
  {
    return boost::lexical_cast<unsigned> ( str );
  }

  template <>
  Common_API unsigned long from_str<unsigned long> (const std::string& str)
  {
    return boost::lexical_cast<unsigned long> ( str );
  }

  template <>
  Common_API unsigned long long from_str<unsigned long long> (const std::string& str)
  {
    return boost::lexical_cast<unsigned long long> ( str );
  }

  template <>
  Common_API float from_str<float> (const std::string& str)
  {
    char * error;
    float result = std::strtod(str.c_str(), &error);
    if (*error != 0)
    {
      throw ParsingFailed(FromHere(), "String " + str + " could not be parsed as a number");
    }
    return result;
  }

  template <>
  Common_API double from_str<double> (const std::string& str)
  {
    char * error;
    double result = std::strtod(str.c_str(), &error);
    if (*error != 0)
    {
      throw ParsingFailed(FromHere(), "String " + str + " could not be parsed as a number");
    }
    return result;
  }

  template <>
  Common_API URI from_str<URI> (const std::string& str)
  {
    return URI( str );
  }

  template <>
  Common_API UUCount from_str<UUCount> (const std::string& str)
  {
    return UUCount( str );
  }

  template <>
  Common_API std::string from_str<std::string> (const std::string& str)
  {
    return str;
  }

  template <>
  Common_API std::vector< std::string > from_str<std::vector<std::string> > (const std::string& str)
  {
    std::vector< std::string > new_vector;
    std::stringstream ss(str);
    std::string tmp;
    while (ss >> tmp) new_vector.push_back( tmp );
    return new_vector;
  }

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
