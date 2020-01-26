#ifndef PRIORITY_SAMPLING_H__
#define PRIORITY_SAMPLING_H__
#include <queue>
#include <unordered_map>
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
    struct pqgetter: public std::priority_queue<Tuple, std::vector<Tuple>, std::greater<>> {
        auto getc() const {return this->c;}
    };
    auto getc() const {
        return reinterpret_cast<const pqgetter &>(heap_).getc();
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
template<typename VT, typename FT, template<typename...> class Container=std::unordered_map, typename Hasher=std::hash<VT>>
struct PrioritySampledSketch {
    using Tuple = typename PrioritySampler<VT, FT>::Tuple;
    Container<VT, const Tuple *> c_;
    std::vector<Tuple> items_;
    FT pm1_priority_;
    PrioritySampledSketch(const PrioritySampler<VT, FT> &o){
        auto c = o.getc();
        for(const auto v: c) items_.emplace_back(v);
        std::sort(items_.begin(), items_.end());
        for(const auto &tup: items_) {
            c_.emplace(std::get<2>(tup), &tup);
        }
        for(const auto &p: c_) {
            std::fprintf(stderr, "key: %d. tup: %f/%f/%d\n", p.first, std::get<0>(*p.second), std::get<1>(*p.second), std::get<2>(*p.second));
        }
        pm1_priority_ = std::get<0>(items_.back());
    }
    FT estimate(const VT &o) {
        auto it = c_.find(o);
        if(it == c_.end()) return 0.;
        return std::max(pm1_priority_, std::get<1>(*it->second));
    }
};

} // priority_sampling
namespace priosa = priority_sampling;
} // DOGS


#endif
