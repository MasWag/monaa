#include <iostream>
#include <boost/program_options.hpp>

#include "../libmonaa/parametric_monaa.hh"
#include "parametric_timed_automaton_parser.hh"

using namespace boost::program_options;

int main(int argc, char *argv[])
{
  const auto programName = "pmonaa";
  const auto errorHeader = "pmonaa: ";

  std::cin.tie(0);
  std::ios::sync_with_stdio(false);

  const auto die = [&errorHeader] (const char* message, int status) {
    std::cerr << errorHeader << message << std::endl;
    exit(status);
  };

  // visible options
  options_description visible("description of options");
//  std::string timedWordFileName;
  std::string timedAutomatonFileName;
  bool isBinary = false;
  visible.add_options()
    ("help,h", "help")
    ("quiet,q", "quiet")
    ("ascii,a", "ascii mode (default)")
    ("binary,b", "binary mode")
    ("version,V", "version")
    ("rational,r", "use rational number of GMP (default)")
    ("float,F", "use floating point number")
  //    ("input,i", value<std::string>(&timedWordFileName)->default_value("stdin"),"input file of Timed Words")
    ("automaton,f", value<std::string>(&timedAutomatonFileName)->default_value(""),"input file of Parametric Timed Automaton");

  command_line_parser parser(argc, argv);
  parser.options(visible);
  variables_map vm;
  const auto parseResult = parser.run();
  store(parseResult, vm);
  notify(vm);

  if (timedAutomatonFileName.empty() || vm.count("help")) {
    std::cout << programName << " [OPTIONS] -f FILE [FILE]\n" 
              << visible << std::endl;
    return 0;
  }
  if (vm.count("version")) {
    std::cout << "MONAA (a MONitoring tool Acceralated by Automata) 0.0.0\n"
              << visible << std::endl;
    return 0;
  }
  if ((vm.count("ascii") && vm.count("binary"))) {
    die("conflicting input formats specified", 1);
  }
  if (vm.count("binary")) {
    isBinary = true;
  } else if (vm.count("ascii")) {
    isBinary = false;
  }

  ParametricTimedAutomaton TA;

  // parse PTA
  std::ifstream taStream(timedAutomatonFileName);
  BoostParametricTimedAutomaton BoostTA;
  parseBoostTA(taStream, BoostTA);
  convBoostTA(BoostTA, TA);

  FILE* file = stdin;
  AnsPolyhedronPrinter ans(vm.count("quiet"));
  // online mode
  if(vm.count("float")) {
    WordLazyDeque w(file, isBinary);
    parametricMonaa(w, TA, ans);
  } else {
    WordLazyDequeMPQ w(file, isBinary);
    parametricMonaa(w, TA, ans);
  }
  return 0;
}
