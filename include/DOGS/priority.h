#ifndef PRIORITY_SAMPLING_H__
#define PRIORITY_SAMPLING_H__
#include <queue>
#include "./shared.h"
#include <algorithm>
#include <tuple>

namespace DOGS {
inline namespace priority_sampling {

template<typename VT, typename FT=double>
struct PrioritySampler {
    size_t n_;
    using Tuple = std::tuple<FT, FT, VT>;
    std::greater<> cmp_;
    std::priority_queue<Tuple, std::vector<Tuple>, std::greater<>> heap_;
    wy::WyHash<uint64_t, 4> gen_;
    std::uniform_real_distribution<FT> dist_;
    template<typename...Args>
    PrioritySampler(size_t n, uint64_t seed=137, Args &&...args): n_(n), gen_(seed) {
    }
    void add(VT &&vt, FT weight) {
        FT p = weight / dist_(gen_);
        if(heap_.size() < n_ + 1) {
            heap_.emplace(p, weight, std::move(vt));
        } else {
            if(cmp_(std::tie(p, weight, vt), heap_.top())) {
                heap_.pop();
                heap_.emplace(p, weight, std::move(vt));
            }
        }
    }
};

}
namespace priosa = priority_sampling;
} // DOGS


#endif
