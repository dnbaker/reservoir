#include "include/reservoir.h"

using namespace reservoir_dogs;
int main() {
    Reservoir<unsigned> rsv(200);
    for(int i = 0; i < 200; rsv.add(i++));
    for(const auto v: rsv.container())
        std::fprintf(stderr, "zomg: %u\n", v);
    for(int i = 201; i < 4000; rsv.add(i++));
    for(const auto v: rsv.container())
        std::fprintf(stderr, "zomgafter: %u\n", v);
}
