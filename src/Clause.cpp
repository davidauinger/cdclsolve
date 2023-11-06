#include <cdclsolve/Clause.hpp>

#include <algorithm>
#include <iterator>

cdclsolve::Clause::Clause(
    std::vector<std::pair<std::shared_ptr<Variable>, bool>> variables)
    : variables{variables} {
  watch0 = 0;
  watch1 = variables.size() > 1 ? 1 : 0;
}

std::size_t cdclsolve::Clause::size() const { return variables.size(); }

bool cdclsolve::Clause::hasLiteral(std::int32_t literal) const {
  return std::find_if(
             variables.cbegin(), variables.cend(),
             [literal](const std::pair<std::shared_ptr<Variable>, bool> &p) {
               return (p.first->getValue() == std::abs(literal)) &&
                      (p.second == literal > 0);
             }) != variables.cend();
}

void cdclsolve::Clause::setSatisfied(std::int32_t variable) const {
  if (isSatisfied(variables[watch0]) || isSatisfied(variables[watch1])) {
    return;
  }
  auto it{std::find_if(
      variables.cbegin(), variables.cend(),
      [variable](const std::pair<std::shared_ptr<Variable>, bool> &p) {
        return std::abs(p.first->getValue()) == std::abs(variable);
      })};
  if (it == variables.cend()) {
    return;
  }
  watch0 = std::distance(variables.cbegin(), it);
}

void cdclsolve::Clause::setUnsatisfied(std::int32_t variable) const {
  auto it{std::find_if(
      variables.cbegin(), variables.cend(),
      [variable](const std::pair<std::shared_ptr<Variable>, bool> &p) {
        return std::abs(p.first->getValue()) == std::abs(variable);
      })};
  if (it == variables.cend()) {
    return;
  }
  if (watch0 == std::distance(variables.cbegin(), it)) {
    auto it2{std::find_if(
        variables.cbegin(), variables.cend(),
        [this](const std::pair<std::shared_ptr<Variable>, bool> &p) {
          return !p.first->isAssigned() && p.first != variables[watch1].first;
        })};
    if (it2 != variables.cend()) {
      watch0 = std::distance(variables.cbegin(), it2);
    }
  }
  if (watch1 == std::distance(variables.cbegin(), it)) {
    auto it2{std::find_if(
        variables.cbegin(), variables.cend(),
        [this](const std::pair<std::shared_ptr<Variable>, bool> &p) {
          return !p.first->isAssigned() && p.first != variables[watch0].first;
        })};
    if (it2 != variables.cend()) {
      watch1 = std::distance(variables.cbegin(), it2);
    }
  }
}

bool cdclsolve::Clause::isSatisfied() const {
  if (variables.empty()) {
    return false;
  }
  return isSatisfied(variables[watch0]) || isSatisfied(variables[watch1]);
}

bool cdclsolve::Clause::isUnsatisfied() const {
  if (variables.empty()) {
    return true;
  }
  return isUnsatisfied(variables[watch0]) && isUnsatisfied(variables[watch1]);
}

bool cdclsolve::Clause::isUnit() const {
  if (variables.empty()) {
    return false;
  }
  if (watch0 == watch1 && !isUnsatisfied(variables[watch0])) {
    return true;
  }
  return (isUnsatisfied(variables[watch0]) &&
          isUnassigned(variables[watch1])) ||
         (isUnassigned(variables[watch0]) && isUnsatisfied(variables[watch1]));
}

bool cdclsolve::Clause::isUndecided() const {
  return !isSatisfied() && !isUnsatisfied() && !isUnit();
}

bool cdclsolve::Clause::isSatisfying(std::int32_t literal) const {
  return std::find_if(
             variables.cbegin(), variables.cend(),
             [literal](const std::pair<std::shared_ptr<Variable>, bool> &p) {
               return p.first->getValue() == std::abs(literal) &&
                      (p.second == literal > 0) && !p.first->isAssigned();
             }) != variables.cend();
}

std::int32_t cdclsolve::Clause::getSatisfyingAssignment() const {
  if (!variables[watch0].first->isAssigned()) {
    return variables[watch0].second ? variables[watch0].first->getValue()
                                    : -variables[watch0].first->getValue();
  }
  if (!variables[watch1].first->isAssigned()) {
    return variables[watch1].second ? variables[watch1].first->getValue()
                                    : -variables[watch1].first->getValue();
  }
  return 0;
}

std::set<std::int32_t> cdclsolve::Clause::getLiterals() const {
  std::set<std::int32_t> literals;
  for (const auto &[v, b] : variables) {
    literals.insert(b ? v->getValue() : -v->getValue());
  }
  return literals;
}

std::vector<std::int32_t> cdclsolve::Clause::getLiteralsAsVector() const {
  std::vector<std::int32_t> literals;
  for (const auto &[v, b] : variables) {
    literals.push_back(b ? v->getValue() : -v->getValue());
  }
  return literals;
}

bool cdclsolve::Clause::resolve(std::shared_ptr<const Clause> clause) {
  bool isModified{false};
  for (const auto &literal : clause->getLiterals()) {
    const auto it{std::find_if(
        variables.cbegin(), variables.cend(),
        [literal](const std::pair<std::shared_ptr<Variable>, bool> &p) {
          return (p.first->getValue() == std::abs(literal)) &&
                 (p.second != literal > 0);
        })};
    if (it != variables.cend()) {
      variables.erase(it);
      isModified = true;
    } else {
      const auto it{std::find_if(
          variables.cbegin(), variables.cend(),
          [literal](const std::pair<std::shared_ptr<Variable>, bool> &p) {
            return (p.first->getValue() == std::abs(literal)) &&
                   (p.second == literal > 0);
          })};
      if (it == variables.cend()) {
        variables.push_back(*std::find_if(
            clause->variables.cbegin(), clause->variables.cend(),
            [literal](const std::pair<std::shared_ptr<Variable>, bool> &p) {
              return p.first->getValue() == std::abs(literal);
            }));
        isModified = true;
      }
    }
  }
  if (isModified) {
    updateWatches();
  }
  return isModified;
}

bool cdclsolve::Clause::isSatisfied(
    const std::pair<std::shared_ptr<Variable>, bool> &variable) const {
  return (variable.first->isTrue() && variable.second) ||
         (variable.first->isFalse() && !variable.second);
}

bool cdclsolve::Clause::isUnsatisfied(
    const std::pair<std::shared_ptr<Variable>, bool> &variable) const {
  return (variable.first->isTrue() && !variable.second) ||
         (variable.first->isFalse() && variable.second);
}

bool cdclsolve::Clause::isUnassigned(
    const std::pair<std::shared_ptr<Variable>, bool> &variable) const {
  return !variable.first->isAssigned();
}

void cdclsolve::Clause::updateWatches() const {
  watch0 = 0;
  watch1 = variables.size() > 1 ? 1 : 0;
  if (variables.size() < 3) {
    return;
  }
  auto it{
      std::find_if(variables.cbegin(), variables.cend(),
                   [this](const std::pair<std::shared_ptr<Variable>, bool> &p) {
                     return isSatisfied(p);
                   })};
  if (it != variables.cend()) {
    watch0 = std::distance(variables.cbegin(), it);
    return;
  }
  if (isUnsatisfied(variables[watch0])) {
    auto it{std::find_if(
        variables.cbegin(), variables.cend(),
        [this](const std::pair<std::shared_ptr<Variable>, bool> &p) {
          return isUnassigned(p) && p.first != variables[watch1].first;
        })};
    if (it != variables.cend()) {
      watch0 = std::distance(variables.cbegin(), it);
    }
  }
  if (isUnsatisfied(variables[watch1])) {
    auto it{std::find_if(
        variables.cbegin(), variables.cend(),
        [this](const std::pair<std::shared_ptr<Variable>, bool> &p) {
          return isUnassigned(p) && p.first != variables[watch0].first;
        })};
    if (it != variables.cend()) {
      watch1 = std::distance(variables.cbegin(), it);
    }
  }
}
