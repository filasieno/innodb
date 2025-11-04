#include <fmt/format.h>
#include <random>
#include <cstdlib>
#include "tail_call.hpp"

IB_ASYNC do_print4(int x, int y, int z) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> int_dist(0, 99);
    if (int_dist(gen) > 50) {
        fmt::print("4 >\n");
        fmt::print("end\n");
        std::exit(0);
    } else {
        fmt::print("4 <=\n");
        co_do(do_print4, 1, 2, 3);
    }
}

IB_ASYNC do_print3(void* ptr = nullptr) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> int_dist(0, 99);
    int val = int_dist(gen);
    if (val > 50) {
        fmt::print("3 >\n");
        co_do(do_print4, 1, 2, 3);
    } else {
        fmt::print("3 <=\n");
        co_do(do_print3, ptr);
    }
}

IB_ASYNC do_print2(void* ptr = nullptr) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> int_dist(0, 99);
    int val = int_dist(gen);
    if (val > 50) {
        fmt::print("2 >\n");
        co_do(do_print3, ptr);
    } else {
        fmt::print("2 <=\n");
        co_do(do_print2, ptr);
    }
}

IB_ASYNC do_print1(void* ptr = nullptr) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> int_dist(0, 99);
    int val = int_dist(gen);
    if (val > 50) {
        fmt::print("1 >\n");
        co_do(do_print2, ptr);
    } else {
        fmt::print("1 <=\n");
        co_do(do_print1, ptr);
    }
}

IB_ASYNC async_main(int argc, char** argv) {
    fmt::print("begin\n");
    co_do(do_print1, nullptr);
}

int main(int argc, char** argv) {
    co_do(async_main, argc, argv);
    __builtin_unreachable();
}
