#ifndef CDCLSOLVE_SOLVER
#define CDCLSOLVE_SOLVER

#include "Formula.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <string>

namespace cdclsolve {

class Solver {
public:
  enum class Result { SAT, UNSAT };
  Solver(Formula &formula, const std::string &decisionHeuristic);
  Result solve();
  std::chrono::steady_clock::duration
  getDurationBooleanConstraintPropagation() const;
  double getRelativeDurationBooleanConstraintPropagation() const;
  std::chrono::steady_clock::duration getDurationDecision() const;
  double getRelativeDurationDecision() const;
  std::chrono::steady_clock::duration getDurationConflictResolution() const;
  double getRelativeDurationConflictResolution() const;
  std::chrono::steady_clock::duration getDurationTotal() const;
  double getRelativeDurationTotal() const;

private:
  using decisionFunction = std::function<std::int32_t(const Formula &formula)>;
  static const std::map<std::string, decisionFunction> decisionHeuristicMap;
  const decisionFunction decide;
  Formula &formula;
  std::chrono::steady_clock::duration durationBooleanConstraintPropagation{
      std::chrono::steady_clock::duration::zero()};
  std::chrono::steady_clock::duration durationDecision{
      std::chrono::steady_clock::duration::zero()};
  std::chrono::steady_clock::duration durationConflictResolution{
      std::chrono::steady_clock::duration::zero()};
  std::chrono::steady_clock::duration durationTotal{
      std::chrono::steady_clock::duration::zero()};
  std::int32_t decisionLevel{0};
  Result doSolve();
  void doBooleanConstraintPropagation();
  void doDecision();
  void doConflictResolution();
};

} // namespace cdclsolve

#endif
