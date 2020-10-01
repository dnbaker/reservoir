#include "include/DOGS/reservoir.h"

int main() {
    std::vector<double> weights;
    while(weights.size() < 100) weights.emplace_back(double(std::rand() ) / RAND_MAX * 1e6);
    while(weights.size() < 100000000) weights.emplace_back(double(std::rand() ) / RAND_MAX);
    weights.emplace_back(1e10);
    std::vector<uint32_t> ind(weights.size());
    std::iota(ind.begin(), ind.end(), 0u);
    std::fprintf(stderr, "Starting\n");
    int n = 1;

    auto t = std::chrono::high_resolution_clock::now();
    auto ret = DOGS::CalaverasReservoirSampler<uint32_t>::parallel_create(ind.begin(), ind.end(), n, /*nthreads=*/8, weights.data());
    auto t2 = std::chrono::high_resolution_clock::now();
    std::fprintf(stderr, "Selected point: %u/%g\n", ret.front().second, ret.front().first);
    weights.pop_back();
    ind.pop_back();
    auto t3 = std::chrono::high_resolution_clock::now();
    auto ret2 = DOGS::CalaverasReservoirSampler<uint32_t>::parallel_create(ind.begin(), ind.end(), n, /*nthreads=*/8, weights.data());
    auto t4 = std::chrono::high_resolution_clock::now();
    std::fprintf(stderr, "Selected point (without high-weight last point:): %u/%g\n", ret2.front().second, ret2.front().first);
    auto t5 = std::chrono::high_resolution_clock::now();
    DOGS::CalaverasReservoirSampler<uint32_t> tmp(n);
    tmp.add(ind.begin(), ind.end(), weights.data());
    auto t6 = std::chrono::high_resolution_clock::now();
    long long unsigned d1 = (t2 - t).count(), d2 = (t4 - t3).count(), d3 = (t6 - t5).count();
    std::fprintf(stderr, "First two tries: %llu/%llu. One thread: %llu. Ratio: %g\n", d1, d2, d3, double(d3) / std::max(d1, d2));
}
