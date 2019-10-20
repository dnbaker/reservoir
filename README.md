### Reservoir sampling for Data frOm Gigantic Streams

Using:


```c++
#include "reservoir.h"

bool get_next_item(std::FILE *fp, uint64_t &item) {
    int rc = std::fread(&item, sizeof(item), 1, fp);
    return rc == 1;
}

int main(int argc, char *argv[]) {
    std::FILE *fp = argc > 1 ? std::fopen(argv[1]): stdin;
    rsvd::Reservoir<uint64_t> reservoir;
    reservoir.seed(137);
    uint64_t item;
    while(get_next_item(fp, item)) {
        reservoir.add(item);
    }
    auto sampled_items = std::move(reservoir.container());
    if(fp != stdin) std::fclose(fp);
}
```

For very large streams where consecutive chunks are available in memory and random access is supported,
we support a batch method with better runtime properties:

```
    rsvd::Reservoir<uint64_t> reservoir;
    std::vector<uint64_t> buf;
    // ... resizing, intiialization
    while(fill_buffer(stream, buf)) {
        reservoir.add(buf.begin(), buf.end());
    }
    auto sampled_items = std::move(reservoir.container());
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
