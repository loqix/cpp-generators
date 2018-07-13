
#include <cstdio>
#include "execution_context.h"
#include "generator.h"

int main() {
    auto gen = make_generator<int>([](generator_yield<int> yield) {
        int i = 1;
        while(true) {
            yield(i);
            i *= 2;
        }
    });
    for(int i = 0; i < 10; i++) {
        printf("%d\n", *gen.next());
    }
    return 0;
}
