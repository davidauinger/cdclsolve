#include <cdclsolve/Formula.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <string>

cdclsolve::Formula cdclsolve::Formula::readInput(std::istream &from) {
  std::int32_t numberOfAtoms;
  std::int32_t numberOfClauses;
  std::string token;
  std::int32_t atom;
  from >> token;
  while (token == std::string(COMMENT_LINE_C)) {
    std::string commentLine{};
    std::getline(from, commentLine);
    from >> token;
  }
  if (token != std::string(PROBLEM_LINE_P)) {
    throw std::runtime_error("invalid token in problem line: expected '" +
                             std::string(PROBLEM_LINE_P) + "' but got '" +
                             token + "'");
  }
  from >> token;
  if (token != std::string(PROBLEM_LINE_CNF)) {
    throw std::runtime_error("invalid token in problem line: expected '" +
                             std::string(PROBLEM_LINE_CNF) + "' but got '" +
                             token + "'");
  }
  from >> numberOfAtoms;
  from >> numberOfClauses;
  from >> token;
  while (token == std::string(COMMENT_LINE_C)) {
    std::getline(from, token);
    from >> token;
  }
  Formula formula{numberOfAtoms, numberOfClauses};
  while (!from.eof()) {
    std::vector<std::pair<std::shared_ptr<Variable>, bool>> clause;
    while (token != "0") {
      atom = std::stoi(token);
      if (std::abs(atom) > numberOfAtoms) {
        throw std::runtime_error(
            "unexpected literal in matrix line: expected " +
            std::to_string(numberOfAtoms) + " distinct atoms but got literal " +
            std::to_string(atom));
      }
      clause.push_back({formula.variables[std::abs(atom)], atom > 0});
      from >> token;
    }
    if (from.eof()) {
      throw std::runtime_error("unexpected end of file while parsing clause");
    }
    formula.addClause(std::make_shared<Clause>(clause));
    from >> token;
  }
  if (formula.clauses.size() != numberOfClauses) {
    throw std::runtime_error(
        "invalid number of clauses: expected " +
        std::to_string(numberOfClauses) + " clauses but parsed " +
        std::to_string(formula.clauses.size()) + " clauses");
  }
  return formula;
}

void cdclsolve::Formula::writeOutput(const Formula &formula, std::ostream &to) {
  to << SOLUTION_LINE_S << " " << SOLUTION_LINE_CNF << " "
     << formula.getAnswer() << " " << formula.variables.size() - 1 << " "
     << formula.numberOfClauses << std::endl;
  if (formula.isSatisfied()) {
    for (const auto &v : formula.variables) {
      if (v->getValue() != 0 && v->isAssigned()) {
        to << CERT_LINE_V << " "
           << (v->isTrue() ? v->getValue() : -v->getValue()) << std::endl;
      }
    }
  }
}

bool cdclsolve::Formula::hasFreeLiterals() const {
  for (const auto &v : variables) {
    if (!v->isAssigned()) {
      return true;
    }
  }
  return false;
}

void cdclsolve::Formula::propagateUnitLiterals(std::int32_t decisionLevel) {
  bool hasMoreUnitLiterals{true};
  while (hasMoreUnitLiterals) {
    hasMoreUnitLiterals = false;
    for (const auto &c : clauses) {
      if (c->isUnsatisfied()) {
        conflict = std::make_shared<Variable>(0);
        conflict->assignTrue(c, decisionLevel);
        return;
      } else if (c->isUnit()) {
        auto l{c->getSatisfyingAssignment()};
        if (!variables.at(std::abs(l))->isAssigned()) {
          hasMoreUnitLiterals = true;
          if (l > 0) {
            assignTrue(l, c, decisionLevel);
          } else {
            assignFalse(-l, c, decisionLevel);
          }
        }
      }
    }
  }
}

void cdclsolve::Formula::assign(std::int32_t literal,
                                std::int32_t decisionLevel) {
  if (literal > 0) {
    assignTrue(literal, nullptr, decisionLevel);
  } else {
    assignFalse(-literal, nullptr, decisionLevel);
  }
}

std::int32_t cdclsolve::Formula::decideBasic() const {
  const auto it{std::find_if(variables.cbegin(), variables.cend(),
                             [](const std::shared_ptr<Variable> &v) {
                               return v->getValue() != 0 && !v->isAssigned();
                             })};
  if (it != variables.cend()) {
    return (*it)->getValue();
  }
  return 0;
}

std::int32_t cdclsolve::Formula::decideJeroslovWang() const {
  std::int32_t literal{0};
  float maxHeuristic{-1.0f};
  for (std::size_t vi{1}; vi < variables.size(); ++vi) {
    if (!variables.at(vi)->isAssigned()) {
      if (auto h{getJeroslovWangHeuristic(vi)}; h > maxHeuristic) {
        literal = vi;
        maxHeuristic = h;
      }
      if (auto h{getJeroslovWangHeuristic(-vi)}; h > maxHeuristic) {
        literal = -vi;
        maxHeuristic = h;
      }
    }
  }
  return literal;
}

std::int32_t cdclsolve::Formula::decideDlis() const {
  std::int32_t literal{0};
  std::int32_t maxSatisfyingClauses{-1};
  for (std::size_t vi{1}; vi < variables.size(); ++vi) {
    if (!variables.at(vi)->isAssigned()) {
      if (auto msc{getSatisfyingClauses(vi)}; msc > maxSatisfyingClauses) {
        literal = vi;
        maxSatisfyingClauses = msc;
      }
      if (auto msc{getSatisfyingClauses(-vi)}; msc > maxSatisfyingClauses) {
        literal = -vi;
        maxSatisfyingClauses = msc;
      }
    }
  }
  return literal;
}

std::int32_t cdclsolve::Formula::decideVsids() const {
  if (vsidsScoresPositive.empty() || vsidsScoresNegative.empty()) {
    precomputeVsidsScores();
  }
  std::int32_t literal{0};
  std::int32_t maxScore{-1};
  for (std::size_t vi{1}; vi < variables.size(); ++vi) {
    if (!variables.at(vi)->isAssigned()) {
      if (auto score{vsidsScoresPositive[vi]}; score > maxScore) {
        literal = vi;
        maxScore = score;
      }
      if (auto score{vsidsScoresNegative[vi]}; score > maxScore) {
        literal = -vi;
        maxScore = score;
      }
    }
  }
  return literal;
}

bool cdclsolve::Formula::hasConflict() const { return (bool)conflict; }

std::int32_t cdclsolve::Formula::resolveConflict(std::int32_t decisionLevel) {
  std::int32_t backtrackLevel{decisionLevel};
  std::vector<std::pair<std::shared_ptr<Variable>, bool>> antecedent;
  for (const auto &l : conflict->getAntecedent()->getLiterals()) {
    antecedent.push_back({variables[std::abs(l)], l > 0});
  }
  auto conflictClause{std::make_shared<Clause>(antecedent)};
  while (resolveConflict(decisionLevel, conflictClause)) {
  }
  if (conflictClause->size() == 0) {
    backtrackLevel = -1;
  } else if (conflictClause->size() == 1) {
    backtrackLevel =
        variables[std::abs(conflictClause->getLiteralsAsVector()[0])]
            ->getDecisionLevel() -
        1;
  } else {
    for (const auto &l : conflictClause->getLiterals()) {
      backtrackLevel =
          std::min(backtrackLevel, variables[std::abs(l)]->getDecisionLevel());
    }
  }
  while (decisionLevel >= std::max(backtrackLevel, 0)) {
    for (auto &v : variables) {
      if (v->getDecisionLevel() == decisionLevel) {
        unassign(v);
      }
    }
    --decisionLevel;
  }
  if (backtrackLevel >= 0) {
    conflict.reset();
  }
  updateVsidsScores(conflictClause->getLiterals());
  addClause(conflictClause);
  return backtrackLevel;
}

cdclsolve::Formula::Formula(std::int32_t numberOfAtoms,
                            std::int32_t numberOfClauses)
    : numberOfClauses{numberOfClauses} {
  for (std::int32_t i{0}; i <= numberOfAtoms; ++i) {
    variables.push_back(std::make_shared<Variable>(i));
  }
}

bool cdclsolve::Formula::isSatisfied() const {
  for (const auto &c : clauses) {
    if (!c->isSatisfied()) {
      return false;
    }
  }
  return true;
}

bool cdclsolve::Formula::isUnsatisfied() const {
  for (const auto &c : clauses) {
    if (c->isUnsatisfied()) {
      return true;
    }
  }
  return false;
}

void cdclsolve::Formula::addClause(std::shared_ptr<Clause> clause) {
  clauses.push_back(clause);
  for (std::int32_t l : clause->getLiterals()) {
    if (l > 0) {
      variables.at(l)->addPositiveOccurance(clause);
    } else {
      variables.at(-l)->addNegativeOccurance(clause);
    }
  }
}

void cdclsolve::Formula::assignTrue(std::int32_t variable,
                                    std::shared_ptr<Clause> antecedent,
                                    std::int32_t decisionLevel) {
  variables.at(variable)->assignTrue(antecedent, decisionLevel);
}

void cdclsolve::Formula::assignFalse(std::int32_t variable,
                                     std::shared_ptr<Clause> antecedent,
                                     std::int32_t decisionLevel) {
  variables.at(variable)->assignFalse(antecedent, decisionLevel);
}

void cdclsolve::Formula::unassign(std::shared_ptr<Variable> variable) {
  variable->unassign();
}

std::int32_t cdclsolve::Formula::getAnswer() const {
  if (isSatisfied()) {
    return 1;
  }
  if (isUnsatisfied()) {
    return -1;
  }
  return 0;
}

bool cdclsolve::Formula::resolveConflict(
    std::int32_t decisionLevel, std::shared_ptr<Clause> conflictClause) {
  auto assignedLiteralsAtDecisionLevel{0};
  std::shared_ptr<Variable> decisionVariable;
  for (const auto literal : conflictClause->getLiterals()) {
    std::shared_ptr<Variable> variable{variables[std::abs(literal)]};
    if (variable->getDecisionLevel() == decisionLevel) {
      ++assignedLiteralsAtDecisionLevel;
      if (variable->getAntecedent() && !decisionVariable) {
        decisionVariable = variable;
      }
      if (assignedLiteralsAtDecisionLevel > 1 && decisionVariable) {
        conflictClause->resolve(decisionVariable->getAntecedent());
        return true;
      }
    }
  }
  return false;
}

float cdclsolve::Formula::getJeroslovWangHeuristic(std::int32_t literal) const {
  float h{0.0f};
  for (const auto &clause : clauses) {
    if (clause->hasLiteral(literal)) {
      h += std::pow(2.0f, -clause->size());
    }
  }
  return h;
}

std::int32_t
cdclsolve::Formula::getSatisfyingClauses(std::int32_t literal) const {
  std::int32_t satisfyingClauses{0};
  for (const auto &c : clauses) {
    if (c->isUndecided() && c->isSatisfying(literal)) {
      ++satisfyingClauses;
    }
  }
  return satisfyingClauses;
}

void cdclsolve::Formula::precomputeVsidsScores() const {
  vsidsScoresPositive = std::vector<std::int32_t>(variables.size(), 0);
  vsidsScoresNegative = std::vector<std::int32_t>(variables.size(), 0);
  for (std::int32_t atom{1}; atom < variables.size(); ++atom) {
    vsidsScoresPositive[atom] = getSatisfyingClauses(atom);
    vsidsScoresNegative[atom] = getSatisfyingClauses(-atom);
  }
}

void cdclsolve::Formula::updateVsidsScores(
    const std::set<std::int32_t> &conflictClause) const {
  if (!vsidsScoresPositive.empty() && !vsidsScoresNegative.empty()) {
    static std::int32_t rebalanceCount{32};
    for (const auto &literal : conflictClause) {
      if (literal > 0) {
        ++vsidsScoresPositive[literal];
      } else {
        ++vsidsScoresNegative[-literal];
      }
    }
    --rebalanceCount;
    if (rebalanceCount == 0) {
      rebalanceVsidsScores();
      rebalanceCount = 32;
    }
  }
}

void cdclsolve::Formula::rebalanceVsidsScores() const {
  for (auto &s : vsidsScoresPositive) {
    s /= 2;
  }
  for (auto &s : vsidsScoresNegative) {
    s /= 2;
  }
}
