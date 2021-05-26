#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "tre_driver.hh"

using namespace boost::program_options;

#ifndef MONAA_VERSION
#define MONAA_VERSION "HEAD"
#endif

int main(int argc, char *argv[]) {
  const auto programName = "tre2ta";
  const auto errorHeader = "tre2ta: ";

  const auto die = [&errorHeader](const char *message, int status) {
    std::cerr << errorHeader << message << std::endl;
    exit(status);
  };

  // visible options
  options_description visible("description of options");
  std::string tre, outputFilename;
  std::stringstream treStream;
  visible.add_options()("help,h", "help")(
      "version,V",
      "version")("output-file,o",
                 value<std::string>(&outputFilename)->default_value("stdout"),
                 "result to FILE")("expression,e",
                                   value<std::string>(&tre)->default_value(""),
                                   "Pattern timed regular expression");

  command_line_parser parser(argc, argv);
  parser.options(visible);
  variables_map vm;
  const auto parseResult = parser.run();
  store(parseResult, vm);
  notify(vm);

  for (auto const &str :
       collect_unrecognized(parseResult.options, include_positional)) {
    tre = std::move(str);
  }

  if (vm.count("version")) {
    std::cout
        << "MONAA (a MONitoring tool Acceralated by Automata) " << MONAA_VERSION
        << "\n"
        << "Copyright (C) 2017-2019 Masaki Waga.\n\n"
        << "This program is free software; you can redistribute it and/or "
           "modify\n"
        << "it under the terms of the GNU General Public License as published "
           "by\n"
        << "the Free Software Foundation; either version 3 of the License, or\n"
        << "(at your option) any later version.\n\n"
        << "This program is distributed in the hope that it will be useful,\n"
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
        << "GNU General Public License for more details.\n\n"
        << "You should have received a copy of the GNU General Public License\n"
        << "along with this program. If not, see http://www.gnu.org/licenses/."
        << std::endl;
    return 0;
  }
  if (tre.empty() || vm.count("help")) {
    std::cout << programName << " [OPTIONS] PATTERN\n"
              << programName << " [OPTIONS] -e PATTERN\n"
              << visible << std::endl;
    return 0;
  }

  // parse TRE
  if (tre.empty()) {
    // read TRE from stdin
    std::getline(std::cin, tre);
  }
  treStream << tre.c_str();
  TREDriver driver;
  if (!driver.parse(treStream)) {
    die("Failed to parse TRE", 2);
  }

  TimedAutomaton TA;
  driver.getResult()->toEventTA(TA);

  if (outputFilename == "stdout") {
    std::cout << TA << std::endl;
  } else {
    std::ofstream outputStream(outputFilename);
    outputStream << TA << std::endl;
  }

  return 0;
}
