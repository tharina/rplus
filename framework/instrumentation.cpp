#include "instrumentation.h"

#include <papi.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace framework {

namespace {
template<class R, class P>
std::ostream& formatDuration(std::ostream& str, const std::chrono::duration<R, P>& d) {
    using namespace std::chrono;
    if (d >= seconds(10)) {
        const auto h = duration_cast<hours>(d);
        const auto m = duration_cast<minutes>(d - h);
        const auto s = duration_cast<seconds>(d - h - m);
        if (h > hours::zero())
            str << h.count() << " h ";
        if (h > hours::zero() || m > minutes::zero())
            str << m.count() << " m ";
        str << s.count() << " s";
    } else if (d >= milliseconds(100)) {
        str << duration_cast<milliseconds>(d).count() << " ms";
    } else if (d >= microseconds(100)) {
        str << duration_cast<microseconds>(d).count() << " µs";
    } else {
        str << duration_cast<nanoseconds>(d).count() << " ns";
    }
    return str;
}

template<class It>
size_t maxWidth(It begin, It end) {
    double max = 1;
    while (begin != end)
        max = std::max(max, std::log10(*begin++));
    return std::ceil(max);
}
}  // namespace

void TimeInstrumentation::start() {
    start_ = std::chrono::steady_clock::now();
}

void TimeInstrumentation::stop() {
    duration_ = std::chrono::steady_clock::now() - start_;
}

std::ostream& TimeInstrumentation::print(std::ostream& str) const {
    str << "Time elapsed: ";
    formatDuration(str, duration_);
    return str << '\n';
}

std::ostream& TimeInstrumentation::result(std::ostream& str, const std::string& sep) const {
    return str << sep << "time=" <<
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration_).count();
}

PapiInstrumentation::PapiInstrumentation(const std::vector<Event>& events) {
    const auto init = PAPI_library_init(PAPI_VER_CURRENT);
    if (init != PAPI_VER_CURRENT)
        throw std::runtime_error(std::string("Failed to initialize PAPI library: ")
                + (init > 0 ? " Version mismatch" : PAPI_strerror(init)));
    events_ = PAPI_NULL;
    if (auto err = PAPI_create_eventset(&events_))
        throw std::runtime_error(std::string("Failed to create PAPI event set: ") + PAPI_strerror(err));

    PAPI_event_info_t info;
    for (auto& e : events) {
        if (auto err = PAPI_add_event(events_, e.id)) {
            std::cerr << "Error: Failed to add PAPI event ";
            if (!PAPI_get_event_info(e.id, &info))
                std::cerr << info.symbol;
            else std::cerr << e.name;
            std::cerr << ": " << PAPI_strerror(err) << std::endl;
        } else {
            names_.emplace_back(e.name, e.desc);
        }
    }
    counters_.resize(names_.size(), 0);
}

PapiInstrumentation::~PapiInstrumentation() {
    if (events_ != PAPI_NULL) {
        if (auto err = PAPI_cleanup_eventset(events_))
            std::cerr << "Failed to clean up PAPI event set: " << PAPI_strerror(err) << std::endl;
        if (auto err = PAPI_destroy_eventset(&events_))
            std::cerr << "Failed to destroy PAPI event set: " << PAPI_strerror(err) << std::endl;
    }
}

void PapiInstrumentation::start() {
    if (auto err = PAPI_start(events_))
        throw std::runtime_error(std::string("Failed to start PAPI counters: ") + PAPI_strerror(err));
}

void PapiInstrumentation::stop() {
    if (auto err = PAPI_stop(events_, counters_.data()))
        throw std::runtime_error(std::string("Failed to read PAPI counters: ") + PAPI_strerror(err));
}

std::ostream& PapiInstrumentation::print(std::ostream& str) const {
    if (counters_.empty()) return str;
    const auto w = std::setw(maxWidth(counters_.begin(), counters_.end()));
    str << names_[0].second << w << counters_[0];
    for (size_t i = 1; i < counters_.size(); ++i)
        str << '\n' << names_[i].second << w << counters_[i];
    return str << '\n';
}

std::ostream& PapiInstrumentation::result(std::ostream& str, const std::string& sep) const {
    for (size_t i = 0; i < counters_.size(); ++i)
        str << sep << names_[i].first << '=' << counters_[i];
    return str;
}

DefaultPapiInstrumentation::DefaultPapiInstrumentation()
    : PapiInstrumentation{{
        { PAPI_L3_TCM,  "l3tcm",  "L3 total cache misses: "},
        // { PAPI_L2_DCM,  "l2dcm",  "L2 data cache misses:  "},
        { PAPI_L1_DCM,  "l1dcm",  "L1 data cache misses:  "},
        { PAPI_BR_MSP,  "brmsp",  "Branch mispredictions: "},
        { PAPI_TOT_INS, "totins", "Total instructions:    "},
        { PAPI_TOT_CYC, "totcyc", "Total cycles elapsed:  "}
    }}
{}

}  // namespace framework

