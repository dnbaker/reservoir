#ifndef RESERVOIR_DOGS_H__
#define RESERVOIR_DOGS_H__
#include "./shared.h"
#include <queue>
#include <thread>

namespace DOGS {
inline namespace reservoir {

template<typename T, typename RNG=wy::WyRand<uint32_t>,
         template<typename...> class Container=std::vector,
         typename IT=uint32_t,
         typename...VectorTemplateArgs>
class ReservoirSampler {
    using rng_type = typename RNG::result_type;
    using CType = Container<T, VectorTemplateArgs...>;

    size_t n_;
    size_t t_;
    CType v_;
    RNG rng_;
    schism::Schismatic<IT> div_;
    static_assert(std::is_integral_v<IT>, "integer type must be integral");;

public:
    ReservoirSampler(size_t n): n_(n), t_(0), div_(n) {
        Emplacer<CType>::reserve(v_, n);
    }
    template<typename...Args>
    ReservoirSampler(size_t n, Args &&...args): n_(n), t_(0), v_(std::forward<Args>(args)...), div_(n) {
        Emplacer<CType>::reserve(v_, n);
    }
    void seed(uint64_t s) {rng_.seed(s);}
    bool add(T &&x) {
        static constexpr double dt = double(std::numeric_limits<rng_type>::max());
        bool ret = true;
        if(size() < n()) {
            Emplacer<CType>::insert(v_, std::move(x));
            if(v_.size() == n())
                t_ = n_;
        } else if (rng_() < (dt * (double(n_) / ++t_))) {
            auto it = std::begin(v_);
            std::advance(it,  div_.mod(rng_()));
            *it = std::move(x);
        } else ret = false;
        return ret;
    }
    template<typename It>
    void add(It beg, It end) {
        if(size() < n_) {
            do {
                Emplacer<CType>::insert(v_, std::move(*beg++));
            } while(size() < n_ && beg < end);
            if(beg != end) t_ = n_;
        }
        size_t ta = std::distance(beg, end);
        if(!ta) return;
        for(;;) {
            static constexpr double maxinv = 1. / std::numeric_limits<rng_type>::max();
            const double p = double(n_) / t_;
            unsigned g = std::log(double(rng_()) * maxinv) / std::log1p(-p);
            std::advance(beg, g);
            t_ += g;
            if(beg < end) {
                auto it = std::begin(v_);
                std::advance(it, div_.mod(rng_()));
                *it = std::move(*beg);
            } else break;
            ++t_;
        }
    }
    size_t size() const {
        using std::size;
        return size(v_);
    }
    size_t n()    const {return n_;}
    bool full() const {return n_ == size();}
    CType       &container()       {return v_;}
    const CType &container() const {return v_;}
};

template<typename T>
T __roundup(T n) {
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return ++n;
}

template<typename Q>
void queue_reduce_pair(Q &destchunk, Q &srcchunk) {
    for(const auto v: srcchunk.getc()) {
        destchunk.push(v);
        destchunk.pop();
    }
}

template<typename T>
typename T::container_type queue_reduce(std::vector<T> &queues, std::deque<std::thread> &threads, int nt) {
    size_t n = queues.size(), nru = __roundup(n), nrul = size_t(std::log2(nru));
    for(unsigned i = 0; i < nrul; ++i) {
        unsigned chunksize = 1 << (i + 1);
        unsigned nchunks = nru >> (i + 1);
        unsigned chunkstep = 1 << i;
        auto compute = [&](unsigned myid) {
            auto destchunkid = chunksize * myid;
            auto &destchunk = queues.at(destchunkid);
            for(unsigned ochunkid = chunkstep; ochunkid < chunksize; ochunkid += chunkstep) {
                if(ochunkid + destchunkid < queues.size())
                    queue_reduce_pair(destchunk, queues[ochunkid + destchunkid]);
            }
        };
        for(unsigned subchunk = 0; subchunk < nchunks; ++subchunk) {
            if(threads.size() == nt) {
                threads.front().join();
                threads.pop_front();
            }
            threads.emplace_back(compute, subchunk);
        }
        for(auto &t: threads) t.join();
        threads.clear();
    }
    return std::move(queues.front().getc());
}

template<typename T, typename RNG=wy::WyRand<uint32_t>,
         typename IT=uint32_t,
         typename...VectorTemplateArgs>
class CalaverasReservoirSampler {
    using base_type = std::priority_queue<std::pair<double, T>, std::vector<std::pair<double, T>, VectorTemplateArgs...>, std::greater<>>;
    struct CType: public base_type {
        //template<typename...Args>
        //CType(Args &&...args): base_type(std::forward<Args>(args)...) {}
        auto &getc() {return this->c;}
        const auto &getc() const {return this->c;}
    };
    using rng_type = typename RNG::result_type;

    double x_;
    size_t n_;
    size_t t_;
    CType v_;
    RNG rng_;
    schism::Schismatic<IT> div_;
    static_assert(std::is_integral_v<IT>, "integer type must be integral");
    static_assert(sizeof(rng_type) == sizeof(IT), "Ensure RNG type and integral type are of the same size");
public:
    CalaverasReservoirSampler(size_t n, uint64_t seed=0): n_(n), t_(0), rng_(seed), div_(n) {
        v_.getc().reserve(n);
    }
    void seed(uint64_t s) {rng_.seed(s);}
    template<typename WIT=double*>
    void add_range(size_t beg, size_t end, WIT w=static_cast<WIT>(nullptr)) {
        if(w)
            while(beg != end) add(beg++, *w++);
        else
            while(beg != end) add(beg++);
    }
    template<typename IT1, typename WIT=double *>
    void add(IT1 beg, IT1 end, WIT wbeg=static_cast<WIT>(nullptr)) {
        if(wbeg)
            while(beg != end) add(*beg++, *wbeg++);
        else
             while(beg != end) add(*beg++);
    }
    template<typename It, typename It2, typename WIT=double *>
    static auto parallel_create(It beg, It2 end, size_t n, int nthreads=4, WIT ptr=static_cast<WIT>(nullptr), uint64_t seed=0, size_t threshold=100) {
        static constexpr bool using_ints = std::is_integral_v<It> && std::is_integral_v<It2>;
        auto dist = end - beg;
        if(dist < threshold || nthreads <= 1) {
            CalaverasReservoirSampler sampler(n, seed);
            if constexpr(using_ints)
                sampler.add_range(beg, end, ptr);
            else
                sampler.add(beg, end, ptr);
            return std::move(sampler.container());
        }
        auto nperblock = (dist + (nthreads - 1) ) / nthreads;
        std::vector<CalaverasReservoirSampler> samplers;
        for(size_t i = nthreads; i--;samplers.emplace_back(n, seed++));
        std::deque<std::thread> threads;
        auto compute = [&samplers,nperblock,beg,end,ptr](auto blockid) {
            auto mystart = beg + nperblock * blockid;
            auto myend = std::min(mystart + nperblock, end);
            using myit = decltype(mystart);
            if constexpr(std::is_integral_v<myit> && !std::is_pointer_v<myit>) {
                if(ptr) samplers[blockid].add_range(mystart, myend, ptr + nperblock * blockid);
                else    samplers[blockid].add_range(mystart, myend);
            } else {
                if(ptr) samplers[blockid].add(mystart, myend, ptr + nperblock * blockid);
                else    samplers[blockid].add(mystart, myend);
            }
        };
        for(size_t i = 0; i < nthreads; ++i)
            threads.emplace_back(compute, i);
        for(auto &x: threads)
            x.join();
        threads.clear();
        std::vector<CType> c;
        c.reserve(nthreads);
        for(auto &s: samplers) {
            c.emplace_back(std::move(s.heap()));
        }
        return queue_reduce(c, threads, nthreads);
    }
    template<typename WIT>
    static auto parallel_sample_weights(WIT it, WIT it2, size_t n, int nthreads=4, uint64_t seed=0, size_t threshold=100) {
        auto diff = std::distance(it, it2);
        return parallel_create(size_t(0), size_t(diff), n, nthreads, it, seed, threshold);
    }
    template<typename WIT>
    static auto sample1(WIT it, WIT it2, uint64_t seed=0, size_t threshold=100) {
        auto diff = std::distance(it, it2);
        CalaverasReservoirSampler<T> ret(1);
        for(long int i = 0; i < diff; ++i)
            ret.add(i++, *it++);
        return ret.container().front().second;
    }
    template<typename WIT>
    static auto parallel_sample1(WIT it, WIT it2, int nthreads=4, uint64_t seed=0, size_t threshold=100) {
        if(nthreads <= 1) {
            return sample1(it, it2, seed, threshold);
        }
        auto diff = std::distance(it, it2);
        return parallel_create(size_t(0), size_t(diff), 1, nthreads, it, seed, threshold).front().second;
    }
    bool add(T x, double weight=1.) {
        std::uniform_real_distribution<double> urd;
        if(weight <= 0.) return false;
        if(size() < n()) {
            v_.push(std::make_pair(std::pow(urd(rng_), 1. / weight), std::move(x)));
            if(v_.size() == n_) {
                x_ = std::log(urd(rng_)) / std::log(v_.top().first);
            }
            return true;
        } else if((x_ -= weight) <= 0.) {
            auto t = std::pow(v_.top().first, weight);
            v_.pop();
            auto t1 = urd(rng_) * (1. - t) + t; // Uniform between t and 1
            auto r = std::pow(t1, 1. / weight);
            v_.push(std::make_pair(r, x));
            x_ = std::log(urd(rng_)) / std::log(v_.top().first);
            return true;
        }
        return false;
    }
    size_t size() const {
        using std::size;
        return size(v_);
    }
    size_t n()  const {return n_;}
    bool full() const {return n_ == size();}
    auto &heap()       {return v_;}
    const auto &heap() const {return v_;}
    auto &container()       {return v_.getc();}
    const auto &container() const {return v_.getc();}
};

} // reservoir
} // DOGS

namespace rsvd = DOGS::reservoir;

#endif /* RESERVOIR_DOGS_H__ */
