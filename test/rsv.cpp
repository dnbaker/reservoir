#include "include/reservoir.h"

using namespace dogs::reservoir;
int main() {
    int rssz = 300;
    int mseq = 4000000;
    Reservoir<unsigned> rsv(rssz);
    for(int i = 0; i < rssz; rsv.add(i++));
    for(const auto v: rsv.container())
        std::fprintf(stderr, "zomg: %u\n", v);
    for(int i = rssz + 1; i < mseq; rsv.add(i++));
    for(const auto v: rsv.container())
        std::fprintf(stderr, "zomgafter: %u\n", v);
    std::vector<unsigned> zomgs(mseq);
    for(auto &e: zomgs) e = std::rand();
    rsv.add(zomgs.begin(), zomgs.end());
    size_t lt = 0, gt = 0;
    for(const auto v: rsv.container()) {
        std::fprintf(stderr, "zomgafterb: %u\n", v);
        v > mseq ? ++gt: ++lt;
    }
    std::fprintf(stderr, "counts: %zu, %zu\n", gt, lt);
}
