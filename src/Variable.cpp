#include <cdclsolve/Clause.hpp>
#include <cdclsolve/Variable.hpp>

#include <sstream>

cdclsolve::Variable::Variable(std::int32_t variable) : variable{variable} {}

void cdclsolve::Variable::addPositiveOccurance(
    const std::shared_ptr<const Clause> clause) {
  positiveOccurances.push_back(clause);
}

void cdclsolve::Variable::addNegativeOccurance(
    const std::shared_ptr<const Clause> clause) {
  negativeOccurances.push_back(clause);
}

void cdclsolve::Variable::assignTrue(std::shared_ptr<const Clause> antecedent,
                                     std::int32_t decisionLevel) {
  this->antecedent = antecedent;
  this->decisionLevel = decisionLevel;
  assignment = Assignment::TRUE;
  for (const auto &c : positiveOccurances) {
    c->setSatisfied(variable);
  }
  for (const auto &c : negativeOccurances) {
    c->setUnsatisfied(variable);
  }
}

void cdclsolve::Variable::assignFalse(std::shared_ptr<const Clause> antecedent,
                                      std::int32_t decisionLevel) {
  this->antecedent = antecedent;
  this->decisionLevel = decisionLevel;
  assignment = Assignment::FALSE;
  for (const auto &c : positiveOccurances) {
    c->setUnsatisfied(variable);
  }
  for (const auto &c : negativeOccurances) {
    c->setSatisfied(variable);
  }
}

void cdclsolve::Variable::unassign() {
  this->antecedent = nullptr;
  this->decisionLevel = -1;
  assignment = Assignment::UNASSIGNED;
}

bool cdclsolve::Variable::isAssigned() const {
  return assignment != Assignment::UNASSIGNED;
}

bool cdclsolve::Variable::isTrue() const {
  return assignment == Assignment::TRUE;
}

bool cdclsolve::Variable::isFalse() const {
  return assignment == Assignment::FALSE;
}

std::int32_t cdclsolve::Variable::getValue() const { return variable; }

std::shared_ptr<const cdclsolve::Clause>
cdclsolve::Variable::getAntecedent() const {
  return antecedent;
}

std::int32_t cdclsolve::Variable::getDecisionLevel() const {
  return decisionLevel;
}
