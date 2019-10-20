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
    1. Defaults to WyRand.
    2. seed with `reservoir.seed(newseed)`.
3. `Container`
    1. Defaults to std::vector.
    2. for advanced usage only
    2. must be a template with `value_type` as the first parameter.
    4. Must support `std::begin`
    5. If it doesn't support `emplace_back`, specialize `Emplacer` for your type.
    6. Non-member function `size` is used to assess the size of the container.
        1. Defaults to `std::size`.
        2. If no member function or overload is available, provide one.
3.  `VectorArgs...` 
    1. Defaults to nothing, but can be used to provide additional template arguments to the container.
