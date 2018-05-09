#include <iostream>
#include <boost/program_options.hpp>

#include "monaa.hh"
#include "tre_driver.hh"
#include "timed_automaton_parser.hh"
#include "zone_printer.hh"
#include "word_lazy_deque.hh"

using namespace boost::program_options;

inline static std::ostream& 
operator<<( std::ostream &stream, const std::vector<ClockVariables> &resetVars) {
  if (!resetVars.empty()) {
    stream << "reset=\"{";
    bool notFirst = false;
    for (const auto &x: resetVars) {
      if (notFirst) {
        stream << ", ";
      }
      stream << static_cast<int>(x);
      notFirst = true;
    }
    stream << "}\"]";
  }
  return stream;
}

inline static std::ostream& 
operator<<( std::ostream &stream, const std::vector<Constraint> &guard) {
  if (!guard.empty()) {
    stream << "guard=\"{";
    bool notFirst = false;
    for (const auto &x: guard) {
      if (notFirst) {
        stream << ", ";
      }
      stream << x;
      notFirst = true;
    }
    stream << "}\"]";
  }
  return stream;
}

inline static std::ostream& 
operator<<( std::ostream &stream, const TimedAutomaton& A)
{
  std::unordered_map<TAState*, int> rev_map;
  std::vector<bool> init_vec;
  for (int i = 0; i < A.states.size(); i++) {
    rev_map[A.states[i].get()] = i;
  }
  init_vec.resize(A.states.size(), false);
  for (const auto &s: A.initialStates) {
    init_vec[rev_map[s.get()]] = true;
  }
  stream << "digraph G {\n";
  for (int i = 0; i < A.states.size(); i++) {
    stream << "        " << i 
           << " [init=" << init_vec[i] << "][match=" << A.states[i]->isMatch << "];\n";
  }
  for (const auto &s: A.states) {
    for (const auto& edgesPair: s->next) {
      for (const auto& transition: edgesPair.second) {
        stream << "        " << rev_map[s.get()] << "->" << rev_map[transition.target]
               << " [label=" << edgesPair.first << "]"
               << transition.resetVars
               << transition.guard;
                                                   //[match=" << A.states[i]->isMatch; ]
        stream << ";\n";
      }
    }
  }
  //    "        6->7 [label=a][guard=\"{x0 > 1}\"];\n"
  stream << "}";
  return stream;
}

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
      boost::unordered_map<TAState*, uint32_t> indegree;
      driver.getResult()->toEventTA(TA, indegree);
    }
  } else {
    // parse TA
    std::ifstream taStream(timedAutomatonFileName);
    BoostTimedAutomaton BoostTA;
    parseBoostTA(taStream, BoostTA);
    convBoostTA(BoostTA, TA);
  }

  FILE* file = timedWordFileName == "stdin" ? stdin : fopen(timedWordFileName.c_str(), "r");
  ZonePrinter ans(vm.count("quiet"));
  WordLazyDeque w(file, isBinary);
  if (vm.count("dollar")) {
    monaaDollar(w, TA, ans);
  } else {
#ifdef DEBUG
    std::cout << TA.states.size() << std::endl;
    for (const auto &s : TA.states) {
      std::cout << s->next.size() << " " 
                << (s->next.empty() ? 0 : s->next.begin()->second.size()) << std::endl;
    }
    std::cout << TA << std::endl;
    std::cout << TA.clockSize() << std::endl;
    auto start = std::chrono::system_clock::now();
#endif
    monaa(w, TA, ans);
#ifdef DEBUG
    auto end = std::chrono::system_clock::now();
    auto dur = end - start;
    auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    std::cout << "parse TRE: " << nsec / 1000000.0 << " ms" << std::endl;
#endif
  }

  return 0;
}
