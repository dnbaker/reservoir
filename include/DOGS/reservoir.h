#ifndef RESERVOIR_DOGS_H__
#define RESERVOIR_DOGS_H__
#include "./shared.h"

namespace DOGS {
inline namespace reservoir {

template<typename T, typename RNG=wy::WyHash<uint32_t, 16>,
         template<typename...> class Container=std::vector,
         typename...VectorTemplateArgs>
class ReservoirSampler {
    using rng_type = typename RNG::result_type;
    using CType = Container<T, VectorTemplateArgs...>;

    size_t n_;
    size_t t_;
    CType v_;
    RNG rng_;
    schism::Schismatic<uint32_t> div_;

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
        // Based on https://erikerlandson.github.io/blog/2015/11/20/very-fast-reservoir-sampling/
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

} // reservoir
} // DOGS

namespace rsvd = DOGS::reservoir;

#endif /* RESERVOIR_DOGS_H__ */
