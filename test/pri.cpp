#include "DOGS/priority.h"
using namespace DOGS;
int main() {
    PrioritySampler<int> ps(1000);
    for(size_t i = 0; i < 100000u; ++i) {
        ps.add(i, 100000u - i);
    }
    PrioritySampledSketch<int, double> pss(ps);
    bool emitted = false;
    for(size_t i = 0; i < 100000u; ++i) {
        auto e = pss.estimate(i);
        if(e && !emitted)
            std::fprintf(stderr, "estimate: %f\n", e), emitted = true;
    }
}
