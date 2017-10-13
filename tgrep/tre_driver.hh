#pragma once
/*!
  @file tre_driver.hh
  @brief A driver for timed regular expression parser
  @author Masaki Waga <masakiwaga@gmail.com>
*/

#include <string>
#include <cstddef>
#include <fstream>

#include "tre_scanner.hh"
#include "tre_parser.tab.hh"
#include "tre.hh"

/*!
  @brief A driver for regular expression parser
*/
class TREDriver{
public:
  TREDriver() = default;
   
  /** 
   * parse - parse from a file
   * @param filename - valid string with input file
   */
  bool parse( const char * const filename ) {
    assert( filename != nullptr );
    std::ifstream in_file( filename );
    if( !in_file.good()) {
      exit(EXIT_FAILURE);
    }

    return parse_helper(in_file);
  }
  /** 
   * parse - parse from a c++ input stream
   * @param is - std::istream&, valid input stream
   */
  bool parse( std::istream &stream ) {
    if( !stream.good() && stream.eof()) {
      return false;
    }

    return parse_helper(stream);
  }

  std::shared_ptr<const TRE> getResult() const {
    return result;
  }

  std::ostream& print(std::ostream &stream);
private:

  std::shared_ptr<TRE> result;
  bool parse_helper( std::istream &stream ) {
    TREScanner scanner(&stream);
    tgrep::TREParser parser( scanner /* scanner */, 
                            (*this) /* driver */ );

    return parser.parse() == 0;
  }

  friend tgrep::TREParser;
};
