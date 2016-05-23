#ifndef FRAMEWORK_INSTRUMENTATION_MALLOC_H_
#define FRAMEWORK_INSTRUMENTATION_MALLOC_H_

#include "instrumentation.h"

namespace framework {

class MemoryInstrumentation : public Instrumentation {
 public:
    void start() override;
    void stop() override;
    std::ostream& print(std::ostream&) const override;
    std::ostream& result(std::ostream&, const std::string&) const override;

 private:
    size_t base_, peak_, total_, count_;
};

}  // namespace framework
#endif  // FRAMEWORK_INSTRUMENTATION_MALLOC_H_

