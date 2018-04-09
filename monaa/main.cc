#include <iostream>
#include <boost/program_options.hpp>

#include "monaa.hh"
#include "tre_driver.hh"
#include "timed_automaton_parser.hh"
#include "zone_printer.hh"
#include "word_lazy_deque.hh"

using namespace boost::program_options;

int main(int argc, char *argv[])
{
  const auto programName = "monaa";
  const auto errorHeader = "monaa: ";

  const auto die = [&errorHeader] (const char* message, int status) {
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
    ("ascii,a", "ascii mode (default)")
    ("dollar,D", "dollar mode")
    ("binary,b", "binary mode")
    ("version,V", "version")
    ("event,E", "event mode (default)")
    ("signal,S", "signal mode")
    ("input,i", value<std::string>(&timedWordFileName)->default_value("stdin"),"input file of Timed Words")
    ("automaton,f", value<std::string>(&timedAutomatonFileName)->default_value(""),"input file of Timed Automaton")
    ("expression,e", value<std::string>(&tre)->default_value(""),"pattern Timed Regular Expression");

  command_line_parser parser(argc, argv);
  parser.options(visible);
  variables_map vm;
  const auto parseResult = parser.run();
  store(parseResult, vm);
  notify(vm);

  for (auto const& str: collect_unrecognized(parseResult.options, include_positional)) {
    if (timedAutomatonFileName.empty() && tre.empty()) {
      tre = std::move(str);
    } else if (timedWordFileName == "stdin") {
      timedWordFileName = std::move(str);
    }
  }

  if ((timedAutomatonFileName.empty() && tre.empty()) || vm.count("help")) {
    std::cout << programName << " [OPTIONS] PATTERN [FILE]\n" 
              << programName << " [OPTIONS] -e PATTERN [FILE]\n" 
              << programName << " [OPTIONS] -f FILE [FILE]\n" 
              << visible << std::endl;
    return 0;
  }
  if (vm.count("version")) {
    std::cout << "MONAA (a MONitoring tool Acceralated by Automata) 0.0.0\n"
              << visible << std::endl;
    return 0;
  }
  if ((vm.count("ascii") && vm.count("binary")) || (vm.count("signal") && vm.count("event"))) {
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
    // parse TA
    std::ifstream taStream(timedAutomatonFileName);
    BoostTimedAutomaton BoostTA;
    parseBoostTA(taStream, BoostTA);
    convBoostTA(BoostTA, TA);
  }

  FILE* file = fopen(timedWordFileName.c_str(), "r");
  ZonePrinter ans(vm.count("quiet"));
  WordLazyDeque w(file, isBinary);
  if (vm.count("dollar")) {
    monaaDollar(w, TA, ans);
  } else {
    monaa(w, TA, ans);
  }

  return 0;
}
