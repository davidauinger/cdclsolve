#ifndef CDCLSOLVE_CLAUSE
#define CDCLSOLVE_CLAUSE

#include "Variable.hpp"

#include <cstdint>
#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace cdclsolve {

class Clause {
public:
  explicit Clause(
      std::vector<std::pair<std::shared_ptr<Variable>, bool>> variables);
  void printWatches() const;
  std::size_t size() const;
  bool hasLiteral(std::int32_t literal) const;
  void setSatisfied(std::int32_t variable) const;
  void setUnsatisfied(std::int32_t variable) const;
  bool isSatisfied() const;
  bool isUnsatisfied() const;
  bool isUnit() const;
  bool isUndecided() const;
  bool isSatisfying(std::int32_t literal) const;
  std::int32_t getSatisfyingAssignment() const;
  std::set<std::int32_t> getLiterals() const;
  std::vector<std::int32_t> getLiteralsAsVector() const;
  bool resolve(std::shared_ptr<const Clause> clause);

private:
  std::vector<std::pair<std::shared_ptr<Variable>, bool>> variables;
  mutable std::int32_t watch0;
  mutable std::int32_t watch1;
  bool
  isSatisfied(const std::pair<std::shared_ptr<Variable>, bool> &variable) const;
  bool isUnsatisfied(
      const std::pair<std::shared_ptr<Variable>, bool> &variable) const;
  bool isUnassigned(
      const std::pair<std::shared_ptr<Variable>, bool> &variable) const;
  void updateWatches() const;
};

} // namespace cdclsolve

#endif
