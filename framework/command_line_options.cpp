#include "command_line_options.h"

#include <getopt.h>
#include <string.h>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace framework {

constexpr decltype(CommandLineOptions::benchmarkNames) CommandLineOptions::benchmarkNames;
constexpr decltype(CommandLineOptions::instrumentationNames) CommandLineOptions::instrumentationNames;

namespace {
bool setBitset(const char* arg, int& bs, const char* const* names, int count) {
    for (int i = 0; i < count; ++i)
        if (!strcmp(names[i], arg)) {
            bs |= (1 << i);
            return true;
        }
    return false;
}

bool getBitset(const std::string& name, int bs, const char* const* names, int count) {
    if (!bs) return true;
    for (int i = 0; i < count; ++i)
        if (name == names[i])
            return bs & (1 << i);
    throw std::domain_error("Invalid option: " + name);
}
}  // namespace

void CommandLineOptions::printUsage(const char* program_name) {
    std::cerr << "Usage: " << program_name << " <options>..."
        "\n  -h, --help                    print this message"
        "\n  -a, --append                  append results instead of overwriting"
        "\n  -b, --benchmark <name>        benchmark(s) to run. Default is all."
        "\n                                  Valid arguments: uniform, skewed,"
        "\n                                  normal, clustered, stacked"
        "\n  -c, --compare                 compare query results before benchmarking"
        "\n  -e, --max-exponent <n>        generate benchmarks with up to 2^n points."
        "\n                                  Default is 22. See also -n."
        "\n  -i, --iterations <n>          set number of iterations of experiments"
        "\n  -m, --instrumentation <name>  instrumentations to use (default is all)"
        "\n                                  valid arguments: time, papi, memory"
        "\n  -n, --num-points <n>          set number of points in generated data sets"
        "\n  -q, --num-queries <n>         set number of queries to run. Default is 10000."
        "\n  -r, --reporting-query         do range reporting query. Default is counting."
        << std::endl;
}

CommandLineOptions CommandLineOptions::parse(int argc, char* const argv[]) {
    CommandLineOptions o;
    ::option options[] = {
        { "help", no_argument, nullptr, 'h' },
        { "append", no_argument, &o.append_results, 1},
        { "benchmark", required_argument, nullptr, 'b' },
        { "compare", no_argument, nullptr, 'c' },
        { "iterations", required_argument, nullptr, 'i' },
        { "instrumentation", required_argument, nullptr, 'm' },
        { "max-exponent", required_argument, nullptr, 'e' },
        { "num-points", required_argument, nullptr, 'n' },
        { "num-queries", required_argument, nullptr, 'q' },
        { "reporting-query", no_argument, &o.reporting_query, 1},
        { 0, 0, 0, 0 }
    };

    opterr = 0;
    int c;
    while ((c = getopt_long(argc, argv, ":ab:ce:hi:m:n:q:r", options, nullptr)) != -1) {
        switch (c) {
        case 0: break;
        case 'a':
            o.append_results = 1;
            break;
        case 'b':
            if (!setBitset(optarg, o.benchmarks, benchmarkNames,
                        std::extent<decltype(benchmarkNames)>::value)) {
                std::cerr << "Invalid argument to '-b': " << optarg << std::endl;
                o.has_invalid_option = true;
            }
            break;
        case 'c':
            o.compare = 1;
            break;
        case 'e':
            o.max_exponent = std::atoi(optarg);
            if (o.max_exponent <= 4) {
                std::cerr << "Invalid argument to '-e': Must be > 4" << std::endl;
                o.has_invalid_option = true;
            }
            break;
        case 'h':
            o.has_invalid_option = true;
            break;
        case 'i':
            o.iterations = std::atoi(optarg);
            if (o.iterations <= 0) {
                std::cerr << "Invalid argument to '-i': Must be > 0" << std::endl;
                o.has_invalid_option = true;
            }
            break;
        case 'm':
            if (!setBitset(optarg, o.instrumentations, instrumentationNames,
                        std::extent<decltype(instrumentationNames)>::value)) {
                std::cerr << "Invalid argument to '-m': " << optarg << std::endl;
                o.has_invalid_option = true;
            }
            break;
        case 'n':
            o.set_size = std::atol(optarg);
            if (o.set_size <= 0) {
                std::cerr << "Invalid argument to '-n': Must be > 0" << std::endl;
                o.has_invalid_option = true;
            }
            break;
        case 'q':
            o.num_queries = std::atoi(optarg);
            if (o.num_queries <= 0) {
                std::cerr << "Invalid argument to '-q': Must be > 0" << std::endl;
                o.has_invalid_option = true;
            }
            break;
        case 'r':
            o.reporting_query = 1;
            break;
        case ':':
            o.has_invalid_option = true;
            if (optopt)
                std::cerr << "Missing argument to '-" << static_cast<char>(optopt) << '\'' << std::endl;
            else
                std::cerr << "Missing argument to '" << argv[optind - 1] << '\''<< std::endl;
            break;
        case '?':
        default:
            o.has_invalid_option = true;
            if (optopt)
                std::cerr << "Invalid option '-" << static_cast<char>(optopt) << '\''<< std::endl;
            else
                std::cerr << "Invalid option '" << argv[optind - 1] << '\''<< std::endl;
        }
    }

    if (o.has_invalid_option)
        printUsage(argv[0]);
    return o;
}

bool CommandLineOptions::hasBenchmark(const std::string& name) const {
    return getBitset(name, benchmarks, benchmarkNames,
            std::extent<decltype(benchmarkNames)>::value);
}

bool CommandLineOptions::hasInstrumentation(const std::string& name) const {
    return getBitset(name, instrumentations, instrumentationNames,
            std::extent<decltype(instrumentationNames)>::value);
}

void CommandLineOptions::removeInstrumentation(const std::string& name) {
    size_t index = 0;
    for (; index < std::extent<decltype(instrumentationNames)>::value; ++index)
        if (name == instrumentationNames[index]) break;
    if (index >= std::extent<decltype(instrumentationNames)>::value)
        throw std::domain_error("Invalid option: " + name);
    if (!instrumentations) {
        instrumentations = ~(1 << index);
    } else {
        instrumentations &= ~(1 << index);
    }
}

char** CommandLineOptions::makeCommandLine(const char* program_name) const {
    char** argv = new char*[32];
    argv[0] = new char[1024];
    char* p = argv[0];
    int argc = 0;
    const auto add = [&](const char* opt) {
        argv[argc++] = p;
        do {
            *p++ = *opt;
        } while (*opt++);
    };
    const auto addN = [&](size_t n) {
        argv[argc++] = p;
        p += sprintf(p, "%tu", n);
    };
    add(program_name);

    if (benchmarks)
        for (size_t i = 0; i < std::extent<decltype(benchmarkNames)>::value; ++i)
            if (benchmarks & (1 << i)) {
                add("-b");
                add(benchmarkNames[i]);
            }
    if (instrumentations)
        for (size_t i = 0; i < std::extent<decltype(instrumentationNames)>::value; ++i)
            if (instrumentations & (1 << i)) {
                add("-m");
                add(instrumentationNames[i]);
            }
    add("-e");
    addN(max_exponent);
    if (set_size) {
        add("-n");
        addN(set_size);
    }
    add("-q");
    addN(num_queries);
    add("-i");
    addN(iterations);
    if (reporting_query)
        add("-r");
    if (append_results)
        add("-a");
    // never compare, already did that this run if requested

    argv[argc] = nullptr;
    return argv;
}

}  // namespace framework

