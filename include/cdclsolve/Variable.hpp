#ifndef CDCLSOLVE_VARIABLE
#define CDCLSOLVE_VARIABLE

#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace cdclsolve {

class Clause;

class Variable {
public:
  friend std::ostream &operator<<(std::ostream &ostream,
                                  cdclsolve::Variable variable);
  explicit Variable(std::int32_t variable);
  void addPositiveOccurance(std::shared_ptr<const Clause> clause);
  void addNegativeOccurance(std::shared_ptr<const Clause> clause);
  void assignTrue(std::shared_ptr<const Clause> antecedent,
                  std::int32_t decisionLevel);
  void assignFalse(std::shared_ptr<const Clause> antecedent,
                   std::int32_t decisionLevel);
  void unassign();
  bool isAssigned() const;
  bool isTrue() const;
  bool isFalse() const;
  std::int32_t getValue() const;
  std::shared_ptr<const Clause> getAntecedent() const;
  std::int32_t getDecisionLevel() const;

private:
  enum class Assignment { UNASSIGNED, FALSE, TRUE };
  std::int32_t variable;
  Assignment assignment{Assignment::UNASSIGNED};
  std::shared_ptr<const Clause> antecedent{nullptr};
  std::int32_t decisionLevel{-1};
  std::vector<std::shared_ptr<const Clause>> positiveOccurances;
  std::vector<std::shared_ptr<const Clause>> negativeOccurances;
};

} // namespace cdclsolve

#endif
