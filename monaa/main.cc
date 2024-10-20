#include <boost/program_options.hpp>
#include <iostream>

#include "monaa.hh"
#include "timed_automaton_parser.hh"
#include "tre_driver.hh"

using namespace boost::program_options;

#ifndef MONAA_VERSION
#define MONAA_VERSION "HEAD"
#endif

int main(int argc, char *argv[]) {
  const auto programName = "monaa";
  const auto errorHeader = "monaa: ";

  const auto die = [&errorHeader](const char *message, int status) {
    std::cerr << errorHeader << message << std::endl;
    exit(status);
  };

  // visible options
  options_description visible("description of options");
  std::string timedWordFileName;
  std::string timedAutomatonFileName;
  std::string tre;
  std::stringstream treStream;
  bool isBinary = false;
  bool isSignal = false;
  visible.add_options()
    ("help,h", "help")
    ("quiet,q", "quiet")
    ("version,V", "version")
    ("ascii,a", "ascii mode [default]")
    ("binary,b", "binary mode (experimental)")
    ("event,E", "event mode [default]")
    ("signal,S", "signal mode (experimental)")
    ("input,i", value<std::string>(&timedWordFileName)->default_value("stdin"), "input file of Timed Words")
    ("automaton,f", value<std::string>(&timedAutomatonFileName)->default_value(""), "input file of Timed Automaton")
    ("expression,e", value<std::string>(&tre)->default_value(""), "pattern Timed Regular Expression");

  command_line_parser parser(argc, argv);
  parser.options(visible);
  variables_map vm;
  const auto parseResult = parser.run();
  store(parseResult, vm);
  notify(vm);

  for (auto const &str :
       collect_unrecognized(parseResult.options, include_positional)) {
    if (timedAutomatonFileName.empty() && tre.empty()) {
      tre = std::move(str);
    } else if (timedWordFileName == "stdin") {
      timedWordFileName = std::move(str);
    }
  }

  if (vm.count("version")) {
    std::cout
        << "MONAA (a MONitoring tool Acceralated by Automata) " << MONAA_VERSION << "\n"
        << "Copyright (C) 2017-2024 Masaki Waga.\n\n"
        << "This program is free software; you can redistribute it and/or modify\n"
        << "it under the terms of the GNU General Public License as published by\n"
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
  if ((timedAutomatonFileName.empty() && tre.empty()) || vm.count("help")) {
    std::cout << programName << " [OPTIONS] PATTERN [FILE]\n"
              << programName << " [OPTIONS] -e PATTERN [FILE]\n"
              << programName << " [OPTIONS] -f FILE [FILE]\n"
              << visible << std::endl;
    return 0;
  }
  if ((vm.count("ascii") && vm.count("binary")) ||
      (vm.count("signal") && vm.count("event"))) {
    die("conflicting input formats specified", 1);
  }
  if (vm.count("binary")) {
    isBinary = true;
  } else if (vm.count("ascii")) {
    isBinary = false;
  }
  if (vm.count("signal")) {
    isSignal = true;
  } else if (vm.count("event")) {
    isSignal = false;
  }
  if (!timedAutomatonFileName.empty() && !tre.empty()) {
    die("both a timed automaton and a timed regular expression are specified", 1);
  }

  TimedAutomaton TA;

  if (timedAutomatonFileName.empty()) {
    // parse TRE
    TREDriver driver;
    treStream << tre.c_str();
    if (!driver.parse(treStream)) {
      die("Failed to parse TRE", 2);
    }
    if (isSignal) {
      toSignalTA(driver.getResult(), TA);
    } else {
      driver.getResult()->toEventTA(TA);
    }
  } else {
    if (isSignal) {
      die("signal-mode is not supported only for TAs", 1);
    }
    // parse TA
    std::ifstream taStream(timedAutomatonFileName);
    BoostTimedAutomaton BoostTA;
    parseBoostTA(taStream, BoostTA);
    convBoostTA(BoostTA, TA);
  }

  FILE *file = stdin;
  if (timedWordFileName != "stdin") {
    file = fopen(timedWordFileName.c_str(), "r");
    if (!file) {
      perror("timed word file");
      return 1;
    }
  }
  AnsPrinter ans(vm.count("quiet"));
  // online mode
  WordLazyDeque w(file, isBinary);
  if (vm.count("signal")) {
    monaa(w, TA, ans);
  } else {
    monaaDollar(w, TA, ans);
  }

  return 0;
}
