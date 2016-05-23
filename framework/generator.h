#ifndef FRAMEWORK_GENERATOR_H_
#define FRAMEWORK_GENERATOR_H_

#include <algorithm>
#include <array>
#include <random>
#include <vector>

namespace framework {

class Generator {
 public:
    enum class Distribution {
        kUniform, kSkewed, kGaussian, kClusters, kStackedClusters
    };
    static constexpr auto kSkewExponent = 5;
    static constexpr auto kNumClusters = 128;
    static constexpr auto kNumStackedClusters = 10;

    template<class Scalar = double, size_t kDimensions = 2>
    static std::vector<std::array<Scalar, kDimensions>>
    generatePoints(size_t num, Distribution dist, size_t seed = std::random_device{}()) {
        DistHelper<Scalar, kDimensions> helper{seed};
        std::vector<std::array<Scalar, kDimensions>> result{num};

        switch (dist) {
        case Distribution::kUniform:
            std::generate(result.begin(), result.end(), [&helper]() { return helper.uniform(); });
            break;
        case Distribution::kSkewed:
            std::generate(result.begin(), result.end(), [&helper]() { return helper.skewed(); });
            break;
        case Distribution::kGaussian:
            std::generate(result.begin(), result.end(), [&helper]() { return helper.gaussian(); });
            break;
        case Distribution::kClusters:
            std::generate(result.begin(), result.end(), [&helper]() { return helper.clustered(); });
            break;
        case Distribution::kStackedClusters:
            std::generate(result.begin(), result.end(), [&helper]() { return helper.stacked(); });
            break;
        }

        return result;
    }

    template<class Scalar = double, size_t kDimensions = 2>
    static std::vector<std::pair<std::array<Scalar, kDimensions>, std::array<Scalar, kDimensions>>>
    generateRectangles(size_t num, Scalar min, Scalar max, size_t seed = std::random_device{}()) {
        DistHelper<Scalar, kDimensions> helper{seed, min, max};
        std::vector<std::pair<std::array<Scalar, kDimensions>, std::array<Scalar, kDimensions>>> result{num};

        std::generate(result.begin(), result.end(), [&helper]() {
            return std::make_pair(helper.uniform(), helper.uniform());
        });
        for (auto& p : result)
            for (size_t i = 0; i < kDimensions; ++i)
                if (p.first[i] > p.second[i])
                    std::swap(p.first[i], p.second[i]);

        return result;
    }

 private:
    template<class Scalar, size_t kDimensions>
    struct DistHelper {
        using Point = std::array<Scalar, kDimensions>;

        std::mt19937_64 re;
        std::uniform_real_distribution<double> d_uniform;
        std::normal_distribution<double> d_normal{0.0, 1.0};
        std::uniform_int_distribution<size_t> d_clusters{0, kNumClusters - 1};
        std::uniform_int_distribution<size_t> d_stacked{0, kNumStackedClusters};
        std::normal_distribution<double> d_spread{0.0, 0.01};
        std::vector<Point> clusters;

        DistHelper(size_t seed, double min = 0.0, double max = 1.0)
            : re{seed}
            , d_uniform{min, max}
            , clusters{kNumClusters}
        {
            std::generate(clusters.begin(), clusters.end(), [this]() { return uniform(); });
        }

        Point uniform() {
            Point p;
            std::generate(p.begin(), p.end(), [this]() { return d_uniform(re); });
            return p;
        }

        Point skewed() {
            Point p = uniform();
            // do not skew first dimension
            for (size_t i = 1; i < kDimensions; ++i)
                p[i] = std::pow(p[i], kSkewExponent);
            return p;
        }

        Point gaussian() {
            Point p;
            std::generate(p.begin(), p.end(), [this]() { return d_normal(re); });
            return p;
        }

        Point clustered() {
            const auto cluster = clusters[d_clusters(re)];
            Point p;
            for (size_t i = 0; i < kDimensions; ++i)
                p[i] = cluster[i] + d_spread(re);
            return p;
        }

        Point stacked() {
            Point p;
            for (size_t i = 0; i < kDimensions - 1; ++i)
                p[i] = d_spread(re);
            p[kDimensions - 1] = 1.0 * d_stacked(re) / kNumStackedClusters + d_spread(re);
            return p;
        }
    };
};

}  // namespace framework
#endif  // FRAMEWORK_GENERATOR_H_

