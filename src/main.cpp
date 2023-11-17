#include <cdclsolve/Formula.hpp>
#include <cdclsolve/Solver.hpp>

#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

static int constexpr returnSatisfiable{10};
static int constexpr returnUnsatisfiable{20};

int main(int argc, char **argv) {
  boost::program_options::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "input,i", boost::program_options::value<std::string>(),
      "file name of input formula")(
      "output,o", boost::program_options::value<std::string>(),
      "file name of output solution")(
      "decision,d", boost::program_options::value<std::string>(),
      "decision heuristic to use")("measure,m",
                                   "measure and print solving times");
  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  std::ifstream input;
  if (vm.count("input")) {
    input.open(vm["input"].as<std::string>());
    if (input.is_open()) {
      std::cin.rdbuf(input.rdbuf());
    }
  }
  cdclsolve::Formula formula{cdclsolve::Formula::readInput(std::cin)};
  const std::string decisionHeuristic{vm["decision"].as<std::string>()};
  cdclsolve::Solver solver{cdclsolve::Solver(formula, decisionHeuristic)};
  auto s{solver.solve()};

  std::ofstream output;
  if (vm.count("output")) {
    output.open(vm["output"].as<std::string>());
    if (output.is_open()) {
      std::cout.rdbuf(output.rdbuf());
    }
  }
  cdclsolve::Formula::writeOutput(formula, std::cout);

  if (vm.count("measure")) {
    std::chrono::steady_clock::duration duration;
    duration = solver.getDurationBooleanConstraintPropagation();
    std::cerr << std::chrono::duration_cast<std::chrono::duration<double>>(
                     duration)
                     .count()
              << " " << solver.getRelativeDurationBooleanConstraintPropagation()
              << std::endl;
    duration = solver.getDurationConflictResolution();
    std::cerr << std::chrono::duration_cast<std::chrono::duration<double>>(
                     duration)
                     .count()
              << " " << solver.getRelativeDurationConflictResolution()
              << std::endl;
    duration = solver.getDurationDecision();
    std::cerr << std::chrono::duration_cast<std::chrono::duration<double>>(
                     duration)
                     .count()
              << " " << solver.getRelativeDurationDecision() << std::endl;
    duration = solver.getDurationTotal();
    std::cerr << std::chrono::duration_cast<std::chrono::duration<double>>(
                     duration)
                     .count()
              << " " << solver.getRelativeDurationTotal() << std::endl;
  }
  return s == cdclsolve::Solver::Result::SAT ? returnSatisfiable
                                             : returnUnsatisfiable;
}
