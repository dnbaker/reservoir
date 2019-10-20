#ifndef RESERVOIR_DOGS_H__
#define RESERVOIR_DOGS_H__
#include <vector>
#include <random>
#include <type_traits>
#include <cstdlib>
#include <cassert>
#include "./div.h"
#include "aesctr/wy.h"

namespace reservoir_dogs {
using std::size_t;

template<typename C>
struct Emplacer {
    static void reserve(C &c, size_t n) {
        c.reserve(n);
    }
    template<typename Item>
    static void insert(C &c, Item &&item) {
        c.emplace_back(std::move(item));
    }
};


template<typename T>
struct pass_by_val_faster: public std::integral_constant<bool, (sizeof(T) & 15 == 0) || !std::is_trivially_copyable_v<T>> {};
template<typename T>
bool pass_by_val_faster_v = pass_by_val_faster<T>::value;

template<typename T, typename RNG=wy::WyHash<uint32_t, 16>,
         template<typename...> class Container=std::vector,
         typename...VectorTemplateArgs>
class Reservoir {
    using rng_type = typename RNG::result_type;
    using CType = Container<T, VectorTemplateArgs...>;

    size_t n_;
    size_t t_;
    CType v_;
    RNG rng_;
    schism::Schismatic<uint32_t> div_;

public:
    Reservoir(size_t n): n_(n), t_(0), div_(n) {
        Emplacer<CType>::reserve(v_, n);
    }
    template<typename...Args>
    Reservoir(size_t n, Args &&...args): n_(n), t_(0), v_(std::forward<Args>(args)...), div_(n) {
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
    size_t size() const {
        using std::size;
        return size(v_);
    }
    size_t n()    const {return n_;}
    bool full() const {return n_ == size();}
    CType       &container()       {return v_;}
    const CType &container() const {return v_;}
};  

}

namespace rsvd = reservoir_dogs;

#endif /* RESERVOIR_DOGS_H__ */
