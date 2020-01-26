#include "DOGS/priority.h"
using namespace DOGS;
int main() {
    PrioritySampler<int> ps(1000);
    for(size_t i = 0; i < 100000u; ++i) {
        ps.add(i, 100000u - i);
    }
    PrioritySampledSketch<int, double> pss(ps);
    for(size_t i = 0; i < 100000u; ++i) {
        auto e = pss.estimate(i);
        if(e)
            std::fprintf(stderr, "estimate: %f\n", e);
    }
}
