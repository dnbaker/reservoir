#ifndef RESERVOIR_DOGS_H__
#define RESERVOIR_DOGS_H__
#include "./shared.h"
#include <queue>

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
    static_assert(std::is_integral_v<IT>, "integer type must be integral");

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

template<typename T, typename RNG=wy::WyRand<uint32_t>,
         template<typename...> class Container=std::vector,
         typename IT=uint32_t,
         typename...VectorTemplateArgs>
class CalaverasReservoirSampler {
    using CType = std::priority_queue<std::pair<double, T>, Container<VectorTemplateArgs...>, std::greater<>>;
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
    CalaverasReservoirSampler(size_t n): n_(n), t_(0), div_(n) {
        Emplacer<CType>::reserve(v_, n);
    }
    template<typename...Args>
    CalaverasReservoirSampler(size_t n, Args &&...args): n_(n), t_(0), v_(std::forward<Args>(args)...), div_(n) {
        Emplacer<CType>::reserve(v_, n);
    }
    void seed(uint64_t s) {rng_.seed(s);}
    bool add(T &&x, double weight=1.) {
        std::uniform_real_distribution<double> urd;
        bool ret = true;
        if(size() < n()) {
            v_.push(std::make_pair(std::pow(urd(rng_), 1. / weight), std::move(x)));
            if(v_.size() == n_) {
                x_ = std::log(urd(rng_)) / std::log(v_.top().first);
            }
        } else {
            x_ -= weight;
            if(x_ < 0.) {
                auto t = std::pow(v_.top().first, weight);
                v_.pop();
                auto t1 = urd(rng_) * (1. - t) + t; // Uniform between t and 1
                auto r = std::pow(t1, 1. / weight);
                v_.push(std::make_pair(r, std::move(x)));
                x_ = std::log(urd(rng_)) / std::log(v_.top().first);
            } else ret = false;
        }
        return ret;
    }
    size_t size() const {
        using std::size;
        return size(v_);
    }
    size_t n()  const {return n_;}
    bool full() const {return n_ == size();}
    CType       &container()       {return v_;}
    const CType &container() const {return v_;}
};

} // reservoir
} // DOGS

namespace rsvd = DOGS::reservoir;

#endif /* RESERVOIR_DOGS_H__ */
