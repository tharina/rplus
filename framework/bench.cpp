#include <string.h>
#include <unistd.h>

#include "../contenders.h"
#include "../range_search.h"
#include "command_line_options.h"
#include "benchmark.h"
#include "experiments.h"
#include "generator.h"
#include "instrumentation.h"
#ifdef MALLOC_INSTR
#include "instrumentation_malloc.h"
#endif

using namespace framework;
using Point = std::array<double, 2>;
using RangeSearch = range_search::RangeSearch<Point>;

namespace {
struct RandomBenchmark {
    std::string name;
    Generator::Distribution distribution;
    double min, max;
};

std::vector<RandomBenchmark> randomBenchmarks() {
    return {
        { "uniform", Generator::Distribution::kUniform, 0.0, 1.0 },
        { "skewed", Generator::Distribution::kSkewed, 0.0, 1.0 },
        { "normal", Generator::Distribution::kGaussian, -3.0, 3.0 },
        { "clustered", Generator::Distribution::kClusters, 0.0, 1.0 },
        { "stacked", Generator::Distribution::kStackedClusters, 0.0, 1.0 }
    };
}

std::unique_ptr<Benchmark<RangeSearch>> make_benchmark(const RandomBenchmark& b,
        size_t n, size_t q, bool reporting) {
    return std::unique_ptr<Benchmark<RangeSearch>>(new RangeSearchQueries<Point>{
        Generator::generatePoints(n, b.distribution, n),
        Generator::generateRectangles(q, b.min, b.max, q),
        reporting, "\tbench=" + b.name
    });
}
} // namespace

int main(int argc, char* argv[]) {
    const auto options = CommandLineOptions::parse(argc, argv);
    if (options.has_invalid_option)
        return -1;

    Experiments<RangeSearch> experiments("results/");

    const auto addBenchmarks = [&experiments, &options](size_t num) {
        for (auto& b : randomBenchmarks())
            if (options.hasBenchmark(b.name))
                experiments.addBenchmark(b.name + " (n=" + std::to_string(num) + ')',
                        make_benchmark(b, num, options.num_queries, options.reporting_query));
    };
    if (options.set_size)
        addBenchmarks(options.set_size);
    else for (size_t num = 32, exp = 5;
            exp <= static_cast<size_t>(options.max_exponent); num *= 2, exp++)
        addBenchmarks(num);

#ifndef MALLOC_INSTR
    if (options.hasInstrumentation("time"))
        experiments.addInstrumentation("time",
                std::unique_ptr<Instrumentation>(new TimeInstrumentation()));
    if (options.hasInstrumentation("papi"))
        experiments.addInstrumentation("papi",
                std::unique_ptr<Instrumentation>(new DefaultPapiInstrumentation()));
#else
    if (options.hasInstrumentation("memory"))
        experiments.addInstrumentation("memory",
                std::unique_ptr<Instrumentation>(new MemoryInstrumentation()));
#endif

#define ADD_CONTENDERS
    using namespace range_search;
#include "../contenders.h"
#undef ADD_CONTENDERS

    if (options.compare) {
        if (experiments.compare()) {
            std::cerr << "\nQuery results mismatch; aborting benchmark" << std::endl;
            return -2;
        } else {
            std::cout << "Query results match; continuing with benchmark" << std::endl;
        }
    }

    experiments.run(options.iterations, options.append_results);

    char** args = nullptr;
#ifndef MALLOC_INSTR
    if (options.hasInstrumentation("memory")) {
        auto alt_options = options;
        alt_options.removeInstrumentation("time");
        alt_options.removeInstrumentation("papi");
        args = alt_options.makeCommandLine("./bench_malloc");
    }
#else
    if (options.hasInstrumentation("time") || options.hasInstrumentation("papi")) {
        auto alt_options = options;
        alt_options.removeInstrumentation("memory");
        args = alt_options.makeCommandLine("./bench");
    }
#endif
    if (args) {
        execv(args[0], args);
        std::cerr << "Error executing " << args[0] << ": " << strerror(errno) << std::endl;
        delete[] args[0];
        delete[] args;
        return -3;
    }

    return 0;
}

