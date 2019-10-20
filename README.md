### Reservoir sampling for Data frOm Gigantic Streams

Using:


```c++
#include "reservoir.h"

int main() {
    rsvd::Reservoir<uint64_t> reservoir;
    reservoir.seed(137);
    uint64_t item;
    while(get_next_item(item) >= 0) {
        reservoir.add(item);
    }
    auto sampled_items = std::move(reservoir.container());
}
```

Template arguments are:

1. `value type`
    1. Note: these are added by rvalue reference, so they work for any RAII type.
2. `RNG`
    1. Defaults to `std::mt19937_64`, which is rather slow
    2. seed with `reservoir.seed(newseed)`
3.  `VectorArgs...` 
    1. Defaults to nothing, but can be used to provide additional template arguments to the container.

We may allow templating by container template additionally.
