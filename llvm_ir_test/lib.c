#include <stdio.h>

int func(int k) {
    unsigned int i = 99;
    i += k;
    //printf("Hello, world!%d \n", i);
    auto add_thread = std::thread(
                        [](int index) {

                        },3);

    return 9*i-20;
}


int fib(int n) {
    int i, t, a = 0, b = 1;
    for (i = 0; i < n; i++) {
        t = a + b; a = b; b = t;
    }
    return b;
}
