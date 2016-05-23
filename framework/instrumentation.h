#ifndef FRAMEWORK_INSTRUMENTATION_H_
#define FRAMEWORK_INSTRUMENTATION_H_

#include <chrono>
#include <ostream>
#include <tuple>
#include <vector>

namespace framework {

class Instrumentation {
 public:
    virtual ~Instrumentation() {}

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::ostream& print(std::ostream&) const = 0;
    virtual std::ostream& result(std::ostream&, const std::string& = "\t") const = 0;
};

class TimeInstrumentation : public Instrumentation {
 public:
    void start() override;
    void stop() override;
    std::ostream& print(std::ostream&) const override;
    std::ostream& result(std::ostream&, const std::string&) const override;

 private:
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::duration duration_;
};

class PapiInstrumentation : public Instrumentation {
 public:
    struct Event {
        int id;
        std::string name;
        std::string desc;
    };

    PapiInstrumentation(const std::vector<Event>&);
    ~PapiInstrumentation();

    void start() override;
    void stop() override;
    std::ostream& print(std::ostream&) const override;
    std::ostream& result(std::ostream&, const std::string&) const override;

 private:
    int events_;
    std::vector<long long> counters_;
    std::vector<std::pair<std::string, std::string>> names_;
};

class DefaultPapiInstrumentation : public PapiInstrumentation {
 public:
    DefaultPapiInstrumentation();
};

}  // namespace framework
#endif  // FRAMEWORK_INSTRUMENTATION_H_

