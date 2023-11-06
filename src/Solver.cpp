#include <cdclsolve/Solver.hpp>

cdclsolve::Solver::Solver(Formula &formula,
                          const std::string &decisionHeuristic)
    : decide{decisionHeuristicMap.at(decisionHeuristic)}, formula{formula} {}

cdclsolve::Solver::Result cdclsolve::Solver::solve() {
  std::chrono::steady_clock::time_point start{std::chrono::steady_clock::now()};
  auto result{doSolve()};
  durationTotal = std::chrono::steady_clock::now() - start;
  return result;
}

std::chrono::steady_clock::duration
cdclsolve::Solver::getDurationBooleanConstraintPropagation() const {
  return durationBooleanConstraintPropagation;
}

double
cdclsolve::Solver::getRelativeDurationBooleanConstraintPropagation() const {
  if (durationTotal == std::chrono::steady_clock::duration::zero()) {
    return 0.0;
  }
  return std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationBooleanConstraintPropagation) /
         std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationTotal);
}

std::chrono::steady_clock::duration
cdclsolve::Solver::getDurationConflictResolution() const {
  return durationConflictResolution;
}

double cdclsolve::Solver::getRelativeDurationConflictResolution() const {
  if (durationTotal == std::chrono::steady_clock::duration::zero()) {
    return 0.0;
  }
  return std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationConflictResolution) /
         std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationTotal);
}

std::chrono::steady_clock::duration
cdclsolve::Solver::getDurationDecision() const {
  return durationDecision;
}

double cdclsolve::Solver::getRelativeDurationDecision() const {
  if (durationTotal == std::chrono::steady_clock::duration::zero()) {
    return 0.0;
  }
  return std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationDecision) /
         std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationTotal);
}

std::chrono::steady_clock::duration
cdclsolve::Solver::getDurationTotal() const {
  return durationTotal;
}

double cdclsolve::Solver::getRelativeDurationTotal() const {
  if (durationTotal == std::chrono::steady_clock::duration::zero()) {
    return 0.0;
  }
  return std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationTotal) /
         std::chrono::duration_cast<
             std::chrono::duration<double, std::chrono::steady_clock::period>>(
             durationTotal);
}

const std::map<std::string, cdclsolve::Solver::decisionFunction>
    cdclsolve::Solver::decisionHeuristicMap{
        {"basic", [](const Formula &formula) { return formula.decideBasic(); }},
        {"jeroslovwang",
         [](const Formula &formula) { return formula.decideJeroslovWang(); }},
        {"dlis", [](const Formula &formula) { return formula.decideDlis(); }},
        {"vsids",
         [](const Formula &formula) { return formula.decideVsids(); }}};

cdclsolve::Solver::Result cdclsolve::Solver::doSolve() {
  doBooleanConstraintPropagation();
  if (formula.hasConflict()) {
    return Result::UNSAT;
  }
  while (formula.hasFreeLiterals()) {
    doDecision();
    doBooleanConstraintPropagation();
    if (formula.hasConflict()) {
      doConflictResolution();
      doBooleanConstraintPropagation();
      if (formula.hasConflict()) {
        return Result::UNSAT;
      }
    }
  }
  return Result::SAT;
}

void cdclsolve::Solver::doBooleanConstraintPropagation() {
  std::chrono::steady_clock::time_point start{std::chrono::steady_clock::now()};
  formula.propagateUnitLiterals(decisionLevel);
  durationBooleanConstraintPropagation +=
      std::chrono::steady_clock::now() - start;
}

void cdclsolve::Solver::doDecision() {
  std::chrono::steady_clock::time_point start{std::chrono::steady_clock::now()};
  auto literal{decide(formula)};
  ++decisionLevel;
  formula.assign(literal, decisionLevel);
  durationDecision += std::chrono::steady_clock::now() - start;
}

void cdclsolve::Solver::doConflictResolution() {
  std::chrono::steady_clock::time_point start{std::chrono::steady_clock::now()};
  decisionLevel = formula.resolveConflict(decisionLevel);
  durationConflictResolution += std::chrono::steady_clock::now() - start;
}
