#ifndef FRAMEWORK_COMMAND_LINE_OPTIONS_H_
#define FRAMEWORK_COMMAND_LINE_OPTIONS_H_

#include <cstddef>
#include <string>

namespace framework {

struct CommandLineOptions {
    bool has_invalid_option = false;
    int benchmarks = 0;
    int instrumentations = 0;
    int max_exponent = 22;
    long set_size = 0;
    int num_queries = 10000;
    int iterations = 1;
    int reporting_query = 0;
    int append_results = 0;
    int compare = 0;

    static constexpr const char* const benchmarkNames[] = {
        "uniform", "skewed", "normal", "clustered", "stacked"
    };
    static constexpr const char* const instrumentationNames[] = {
        "time", "papi", "memory"
    };

    bool hasBenchmark(const std::string& name) const;
    bool hasInstrumentation(const std::string& name) const;

    void removeInstrumentation(const std::string& name);

    char** makeCommandLine(const char* program_name) const;

    static void printUsage(const char* program_name);
    static CommandLineOptions parse(int argc, char* const argv[]);
};

}  // namespace framework
#endif  // FRAMEWORK_COMMAND_LINE_OPTIONS_H_

