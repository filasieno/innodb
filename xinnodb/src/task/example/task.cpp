#include "tail_call.hpp"
#include <cstdlib>
#include <fmt/format.h>

struct aysnc_ctx {    
    void* resume = nullptr;
};

struct ctx_t : public aysnc_ctx {
    int value;
};

IB_ASYNC step(ctx_t* ctx);
IB_ASYNC done(ctx_t* ctx);
IB_ASYNC next_step(ctx_t* ctx);

IB_ASYNC step(ctx_t* ctx) {
    fmt::print("step: {}\n", ctx->value);
    co_if(ctx->value == 0, done, next_step, ctx);
}

IB_ASYNC done(ctx_t* ctx) {
    // case value == 0
    delete ctx;
    std::exit(0);
}

IB_ASYNC next_step(ctx_t* ctx) {
    // case value != 0
    ctx->value--;
    co_do(step, ctx);
}

IB_ASYNC aysnc_main(int argc, char** argv) {
    ctx_t* ctx = new ctx_t();
    ctx->value = 20;
    co_do(step, ctx);
}

int main(int argc, char** argv) {
    co_do(aysnc_main, argc, argv);
    return 0;
}