#include "tail_call.hpp"
#include <cstdlib>
#include <fmt/format.h>

struct ctx_t {
    int value;
};

IB_ASYNC step(ctx_t* ctx) {
    fmt::print("step: {}\n", ctx->value);
    if (ctx->value == 0) {
        delete ctx;
        std::exit(0);
    } else {
        ctx->value--;
        ib_tail_call(step, ctx);
    }
}

IB_ASYNC aysnc_main(int argc, char** argv) {
    ctx_t* ctx = new ctx_t({ .value = 20 });
    ib_tail_call(step, ctx);
}

int main(int argc, char** argv) {
    ib_tail_call(aysnc_main, argc, argv);
    return 0;
}