#pragma once

#include "execution_context.h"

template <class R>
struct generator_yield {
    R* cur;
    execution_context_yield yield;
    void operator()(R&& ret) {
        *cur = ret;
        yield(cur);
    }
    void operator()(const R& ret) {
        yield(&ret);
    }
};

template <class R, class Fn>
class generator {
public:
    generator(Fn fn) : _econtext(generator_func{fn, *this}) {}
    
    const R* next() {
        return static_cast<const R*>(_econtext.resume(0));
    }
    struct generator_func {
        Fn fn;
        generator<R, Fn>& gen;
        void operator()(execution_context_yield yield) {
            generator_yield<R> yieldfunc{&gen._cur, yield};
            fn(yieldfunc);
        }
    };
private:
    execution_context<generator_func> _econtext;
};

template <class R, class Fn>
generator<R, Fn> make_generator(Fn f) {
    return { f };
}
