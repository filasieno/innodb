#include <cstdio>


__attribute__((preserve_none)) void step2(int y) {
    printf("step2: %d\n", y);
    return;
}

__attribute__((preserve_none)) void step1(int x) {
    printf("step1: %d\n", x);
    __attribute__((musttail)) return step2(2);
}

__attribute__((preserve_none)) void async_main(int x) {
    __attribute__((musttail)) return step1(1);
}
int main() {
    async_main(1);
    return 0;
}



