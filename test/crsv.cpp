#include "DOGS/reservoir.h"

using namespace DOGS::reservoir;
int main() {
    int rssz = 300;
    unsigned mseq = 4000000;
    CalaverasReservoirSampler<unsigned> rsv(rssz);
    for(int i = 0; i < rssz; rsv.add(i++));
    for(const auto v: rsv.container())
        std::fprintf(stderr, "zomg: %u/%g\n", v.second, v.first);
    for(int i = rssz + 1; i < int(mseq); rsv.add(i++));
    for(const auto v: rsv.container())
        std::fprintf(stderr, "zomgafter: %u\n", v.second);
    std::vector<unsigned> zomgs(mseq);
    for(auto &e: zomgs) e = std::rand();
    rsv.add(zomgs.begin(), zomgs.end());
    size_t lt = 0, gt = 0;
    for(const auto v: rsv.container()) {
        std::fprintf(stderr, "zomgafterb: %u with priority %g\n", v.second, v.first);
        v.second > mseq ? ++gt: ++lt;
    }
    std::fprintf(stderr, "counts: %zu, %zu\n", gt, lt);
}
