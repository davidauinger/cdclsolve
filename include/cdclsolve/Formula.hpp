#ifndef CDCLSOLVE_FORMULA
#define CDCLSOLVE_FORMULA

#include "Clause.hpp"
#include "Variable.hpp"

#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>

namespace cdclsolve {

class Formula {
public:
  static Formula readInput(std::istream &from);
  static void writeOutput(const Formula &formula, std::ostream &to);
  bool hasFreeLiterals() const;
  void propagateUnitLiterals(std::int32_t decisionLevel);
  void assign(std::int32_t literal, std::int32_t decisionLevel);
  std::int32_t decideBasic() const;
  std::int32_t decideJeroslovWang() const;
  std::int32_t decideDlis() const;
  std::int32_t decideVsids() const;
  bool hasConflict() const;
  std::int32_t resolveConflict(std::int32_t decisionLevel);

private:
  static constexpr std::string_view COMMENT_LINE_C{"c"};
  static constexpr std::string_view PROBLEM_LINE_P{"p"};
  static constexpr std::string_view PROBLEM_LINE_CNF{"cnf"};
  static constexpr std::string_view SOLUTION_LINE_S{"s"};
  static constexpr std::string_view SOLUTION_LINE_CNF{"cnf"};
  static constexpr std::string_view CERT_LINE_V{"V"};
  const std::int32_t numberOfClauses;
  std::vector<std::shared_ptr<Variable>> variables;
  std::vector<std::shared_ptr<Clause>> clauses;
  std::shared_ptr<Variable> conflict;
  mutable std::vector<std::int32_t> vsidsScoresPositive;
  mutable std::vector<std::int32_t> vsidsScoresNegative;
  Formula(std::int32_t numberOfAtoms, std::int32_t numberOfClauses);
  bool isSatisfied() const;
  bool isUnsatisfied() const;
  void addClause(std::shared_ptr<Clause> clause);
  void assignTrue(std::int32_t variable, std::shared_ptr<Clause> antecedent,
                  std::int32_t decisionLevel);
  void assignFalse(std::int32_t variable, std::shared_ptr<Clause> antecedent,
                   std::int32_t decisionLevel);
  void unassign(std::shared_ptr<Variable> variable);
  std::int32_t getAnswer() const;
  bool resolveConflict(std::int32_t decisionLevel,
                       std::shared_ptr<Clause> conflictClause);
  float getJeroslovWangHeuristic(std::int32_t literal) const;
  std::int32_t getSatisfyingClauses(std::int32_t literal) const;
  void precomputeVsidsScores() const;
  void updateVsidsScores(const std::set<std::int32_t> &conflictClause) const;
  void rebalanceVsidsScores() const;
};

} // namespace cdclsolve

#endif
