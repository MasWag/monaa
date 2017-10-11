#pragma once
/*!
  @file regexpr_driver.hh
  @brief A driver for regular expression parser
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
  void parse( const char * const filename ) {
    assert( filename != nullptr );
    std::ifstream in_file( filename );
    if( !in_file.good()) {
      exit(EXIT_FAILURE);
    }
    parse_helper(in_file);
    return;
  }
  /** 
   * parse - parse from a c++ input stream
   * @param is - std::istream&, valid input stream
   */
  void parse( std::istream &stream ) {
    if( !stream.good() && stream.eof()) {
      return;
    }
    parse_helper(stream);
    return;
  }

  std::shared_ptr<const TRE> getResult() const {
    return result;
  }

  std::ostream& print(std::ostream &stream);
private:

  std::shared_ptr<TRE> result;
  void parse_helper( std::istream &stream ) {
    TREScanner scanner(&stream);
    tgrep::TREParser parser( scanner /* scanner */, 
                            (*this) /* driver */ );

    if( parser.parse() != 0 ) {
      std::cerr << "Parse failed!!\n";
    }
    return;
  }

  friend tgrep::TREParser;
};
