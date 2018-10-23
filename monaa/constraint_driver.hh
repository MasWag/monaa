#pragma once
/*!
  @file constraint_driver.hh
  @brief A driver for PPL constraint parser
  @author Masaki Waga <masakiwaga@gmail.com>
*/

#include <string>
#include <cstddef>
#include <fstream>

#include "constraint_scanner.hh"
#include "../build/constraint/constraint_parser.tab.hh"

/*!
  @brief A driver for PPL's constraint parser
*/
class ConstraintDriver{
public:
  ConstraintDriver(const std::size_t paramDimensions) : paramDimensions(paramDimensions) {};
   
  /** 
   * parse - parse from a file
   * @param filename - valid string with input file
   */
  bool parse( const char * const filename) {
    assert( filename != nullptr );
    std::ifstream in_file( filename );
    if( !in_file.good()) {
      exit(EXIT_FAILURE);
    }

    return parse_helper(in_file);
  }
  /*!
    @brief parse from a c++ input stream
    @params stream valid input stream
   */
  bool parse( std::istream &stream) {
    if( !stream.good() && stream.eof()) {
      return false;
    }

    return parse_helper(stream);
  }

  const Parma_Polyhedra_Library::Constraint_System& getResult() const {
    return result;
  }

  std::ostream& print(std::ostream);
private:
  //! @params paramDimensions parameter dimension of the PTA 
  const std::size_t paramDimensions;

  Parma_Polyhedra_Library::Constraint_System result;
  bool parse_helper( std::istream &stream) {
    ConstraintScanner scanner(&stream);
    MONAA::Parametric::ConstraintParser parser( scanner /* scanner */, 
                                                (*this) /* driver */,
                                                paramDimensions);

    return parser.parse() == 0;
  }

  friend MONAA::Parametric::ConstraintParser;
};
