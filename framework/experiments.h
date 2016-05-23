#ifndef FRAMEWORK_EXPERIMENTS_H_
#define FRAMEWORK_EXPERIMENTS_H_

#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "benchmark.h"
#include "instrumentation.h"

namespace framework {

template<class Datastructure>
class Experiments {
 public:
    explicit Experiments(const std::string& file_prefix)
        : file_prefix_(file_prefix) {}

    void addContender(const std::string& name, std::function<Datastructure*()> factory) {
        contenders_.push_back({name, std::move(factory)});
    }

    void addInstrumentation(const std::string& name, std::unique_ptr<Instrumentation> instr) {
        instrumentation_.push_back({name, std::move(instr)});
    }

    void addBenchmark(const std::string& name, std::unique_ptr<Benchmark<Datastructure>> benchmark) {
        benchmarks_.push_back({name, std::move(benchmark)});
    }

    void run(size_t iterations = 1, bool append_results = false, bool quiet = false) {
        const std::fstream::openmode openmode = append_results
            ? std::fstream::out | std::fstream::app
            : std::fstream::out;
        for (const auto& instr : instrumentation_) {
            std::ofstream results(file_prefix_ + instr.name + ".txt", openmode);
            for (const auto& contender : contenders_) {
                for (const auto& benchmark : benchmarks_) {
                    std::cout << "\nBenchmarking " << contender.name;
                    std::cout << " using " << instr.name << " instrumentation on "
                        << benchmark.name << std::endl;
                    for (size_t i = 0; i < iterations; ++i) {
                        results << "RESULT\tds=" << contender.name;
                        benchmark.benchmark->result(results);

                        if (!quiet) std::cout << "Preprocessing:\n";
                        instr.instr->start();
                        std::unique_ptr<Datastructure> ds(contender.factory());
                        benchmark.benchmark->runPreprocessing(*ds);
                        instr.instr->stop();
                        instr.instr->result(results, "\tpre");
                        if (!quiet) instr.instr->print(std::cout);

                        if (!quiet) std::cout << "Queries:\n";
                        instr.instr->start();
                        benchmark.benchmark->runQueries(*ds);
                        instr.instr->stop();
                        instr.instr->result(results);
                        if (!quiet) instr.instr->print(std::cout);
                        results << std::endl;
                    }
                }
            }
        }
    }

    bool compare() const {
        if (contenders_.size() < 2) {
            std::cerr << "At least 2 contenders must be registered." << std::endl;
            return false;
        }
        bool mismatch = false;
        for (const auto& benchmark : benchmarks_) {
            std::cout << "\nComparing query results on benchmark " << benchmark.name
                << " using " << contenders_.front().name << " as base" << std::endl;
            std::unique_ptr<Datastructure> base(contenders_.front().factory());
            benchmark.benchmark->runPreprocessing(*base);

            for (size_t i = 1; i < contenders_.size(); ++i) {
                std::cout << "Comparing to " << contenders_[i].name << std::endl;
                std::unique_ptr<Datastructure> ds(contenders_[i].factory());
                benchmark.benchmark->runPreprocessing(*ds);
                mismatch |= benchmark.benchmark->compare(*base, *ds);
            }
        }
        return mismatch;
    }

 private:
    struct Contender {
        std::string name;
        std::function<Datastructure*()> factory;
    };
    struct NamedInstr {
        std::string name;
        std::unique_ptr<Instrumentation> instr;
    };
    struct NamedBenchmark {
        std::string name;
        std::unique_ptr<Benchmark<Datastructure>> benchmark;
    };

    std::string file_prefix_;
    std::vector<Contender> contenders_;
    std::vector<NamedBenchmark> benchmarks_;
    std::vector<NamedInstr> instrumentation_;
};

}  // namespace framework
#endif  // FRAMEWORK_EXPERIMENTS_H_

