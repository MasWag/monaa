#include <iostream>
#include <boost/program_options.hpp>

#include "timedFJS.hh"
#include "tre_driver.hh"

using namespace boost::program_options;

int main(int argc, char *argv[])
{
  // visible options
  options_description visible("description of options");
  std::string fileName;
  std::string tre;
  std::stringstream treStream;
  bool isBinary = false;
  bool isSignal = false;
  visible.add_options()
    ("help,h", "help")
    ("quiet,q", "quiet")
    ("binary,b", "binary mode")
    ("version,V", "version")
    ("event,e", "event mode (default)")
    ("signal,s", "signal mode")
    ("input,i", value<std::string>(&fileName)->default_value("stdin"),"input file")
    ("pattern,p", value<std::string>(&tre)->default_value(""),"pattern Timed Regular Expression");

  // positional options
  positional_options_description positional;
  positional.add("pattern", 1).add("input", 1);

  command_line_parser parser(argc, argv);
  parser.options(visible).positional(positional);
  variables_map vm;
  store(parser.run(), vm);
  notify(vm);

  std::cout << tre << std::endl;

  if (tre.empty() || vm.count("help")) {
    std::cout << "tgrep [option]... [pattern] [file]\n"
              << visible << std::endl;
    return 0;
  }
  if (vm.count("version")) {
    std::cout << "tgrep (Timed grep) 0.0.0\n"
              << visible << std::endl;
    return 0;
  }
  if (vm.count("binary")) {
    isBinary = true;
  }
  if (vm.count("signal")) {
    isSignal = true;
  }
  if (vm.count("event")) {
    isSignal = false;
  }

  // parse TRE

  TREDriver driver;
  if (!driver.parse(treStream)) {
    std::cerr << "Failed to parse TRE" << std::endl;
    return -1;
  }

  int N;
  FILE* file = stdin;
  if (fileName != "stdin") {
    file = fopen(fileName.c_str(), "r");
  }
  if (isBinary) {
    fread(&N, sizeof(int), 1, file);
  } else {
    fscanf(file, "%d", &N);
  }
  
#ifdef LAZY_READ
  WordLazyDeque<std::pair<Alphabet,double> > w(N, file, isBinary);
  AnsNum<ansZone> ans;
#else
  WordVector<std::pair<Alphabet,double> > w(N, file, isBinary);
  AnsVec<Zone> ans;
#endif

  TimedAutomaton TA;

  timedFranekJenningsSmyth (w, TA, ans);

  std::cout << ans.size() << " zones" << std::endl;
  
  if (vm.count ("quiet")) {
    return 0;
  }

#if 0
  // print result
  std::cout << "Results" << std::endl;
  for (const auto &a : ans) {
    std::cout << a.lowerBeginConstraint.first <<std::setw(10)<< 
      (a.lowerBeginConstraint.second ? " <= " : " < ") << "t" << 
      (a.upperBeginConstraint.second ? " <= " : " < ") <<
      a.upperBeginConstraint.first << std::endl;
    std::cout << a.lowerEndConstraint.first << std::setw(10)<< 
      (a.lowerEndConstraint.second ? " <= " : " < ") << "t'" << 
      (a.upperEndConstraint.second ? " <= " : " < ") <<
      a.upperEndConstraint.first << std::endl;
    std::cout << a.lowerDeltaConstraint.first << std::setw(10)<< 
      (a.lowerDeltaConstraint.second ? " <= " : " < ") << "t' - t" << 
      (a.upperDeltaConstraint.second ? " <= " : " < ") <<
      a.upperDeltaConstraint.first << std::endl;
    std::cout << "=============================" << std::endl;
  }
#endif

  return 0;
}
