#include "instrumentation_malloc.h"

#include <cmath>
#include <iomanip>

#include "../malloc_count/malloc_count.h"

namespace framework {

namespace {
template<class T>
size_t maxWidth(const std::initializer_list<T>& args) {
    double max = 1;
    for (T a : args)
        max = std::max(max, std::log10(a));
    return std::ceil(max);
}
}  // namespace

void MemoryInstrumentation::start() {
    base_ = malloc_count_current();
    malloc_count_reset_peak();
    malloc_count_reset_total();
    malloc_count_reset_num_allocs();
}

void MemoryInstrumentation::stop() {
    peak_ = malloc_count_peak() - base_;
    total_ = malloc_count_total();
    count_ = malloc_count_num_allocs();
}

std::ostream& MemoryInstrumentation::print(std::ostream& str) const {
    const auto w = std::setw(maxWidth({peak_ / (1024 * 1024), total_ / (1024 * 1024), count_}));
    return str << "Peak memory usage:      " << w << peak_ / (1024 * 1024) << " MB"
        << "\nTotal memory allocated: " << w << total_ / (1024 * 1024) << " MB"
        << "\nNumber of allocations:  " << w << count_
        << '\n';
}

std::ostream& MemoryInstrumentation::result(std::ostream& str, const std::string& sep) const {
    return str << sep << "peakmem=" << peak_ << sep << "alloc=" << total_
        << sep << "nalloc=" << count_;
}

}  // namespace framework

