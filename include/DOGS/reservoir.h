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
    CalaverasReservoirSampler(size_t n): n_(n), t_(0), div_(n) {
        v_.getc().reserve(n);
    }
    template<typename...Args>
    CalaverasReservoirSampler(size_t n, Args &&...args): n_(n), t_(0), v_(std::forward<Args>(args)...), div_(n) {
        v_.getc().reserve(n);
    }
    void seed(uint64_t s) {rng_.seed(s);}
    template<typename IT1, typename IT2, typename WIT>
    void add(IT1 beg, IT2 end, WIT wbeg) {
        while(beg != end) add(*beg++, *wbeg++);
    }
    template<typename IT1, typename IT2>
    void add(IT1 beg, IT2 end) {
        while(beg != end) add(*beg++);
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
#if 0
    bool add(T &x, double weight=1.) {
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
            v_.push(std::make_pair(r, std::move(x)));
            x_ = std::log(urd(rng_)) / std::log(v_.top().first);
            return true;
        }
        return false;
    }
#endif
    size_t size() const {
        using std::size;
        return size(v_);
    }
    size_t n()  const {return n_;}
    bool full() const {return n_ == size();}
    auto &container()       {return v_.getc();}
    const auto &container() const {return v_.getc();}
};

} // reservoir
} // DOGS

namespace rsvd = DOGS::reservoir;

#endif /* RESERVOIR_DOGS_H__ */
