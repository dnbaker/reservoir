#include "DOGS/priority.h"
using namespace DOGS;
int main() {
    PrioritySampler<int> ps(1000);
    for(size_t i = 0; i < 100000u; ++i) {
        ps.add(i, 100000u - i);
    }
}
