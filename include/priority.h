#ifndef PRIORITY_SAMPLING_H__
#define PRIORITY_SAMPLING_H__
#include <queue>
#include "./shared.h"
#include <algorithm>

namespace dogs {
inline namespace priority_sampling {

template<typename VT, typename FT=double, typename Cmp=std::greater<>>
struct PrioritySampler {
    size_t n_;
    Cmp cmp_;
    std::priority_queue<std::tuple<FT, FT, VT>> heap_;
    wy::WyHash<uint64_t, 4> gen_;
    std::uniform_real_distribution<double> dist_;
    template<typename...Args>
    PrioritySampler(size_t n, uint64_t seed=137, Args &&...args): n_(n), cmp_(std::forward<Args>(args)...), gen_(seed) {
    }
    void add(VT &&vt, FT weight) {
        auto p = weight / dist_(gen_);
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
} // dogs


#endif
