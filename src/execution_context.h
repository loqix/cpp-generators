#pragma once

#include <cstdlib>
#include <exception>
#include "fcontext.h"

#ifdef _MSC_VER
#define UNREACHABLE __assume(0);
#else
#define UNREACHABLE __builtin_unreachable();
#endif

class terminate_exception {
public:
    terminate_exception() {}
};

struct resume_data {
    const void* data;
    bool terminate;
};

struct yield_data {
    const void* data;
    std::exception_ptr exception;
    bool has_exception;
};

class execution_context_yield {
public:
    execution_context_yield(fcontext_t* yield) : _yield(yield) {}
    fcontext_t* _yield;
    const void* operator()(const void* data) {
        yield_data yd = { data, nullptr, false };
        transfer_t t = jump_fcontext(*_yield, &yd);
        *_yield = t.fctx;

        resume_data* rd = static_cast<resume_data*>(t.data);
        if(rd->terminate) {
            throw terminate_exception{};
        }
        return rd->data;
    }
};

constexpr size_t STACK_SIZE = 32 * 1024;

template <class Fn>
class execution_context {
public:
    execution_context(Fn fn) : _stack_size(STACK_SIZE), _fn(fn), _done(false) {
        _stack = (char*)malloc(_stack_size);
        fcontext_t c = make_fcontext(_stack + _stack_size, _stack_size, context_function);
        _resume = jump_fcontext(c, this).fctx;
    }
    ~execution_context() {
        kill();
    }
    static void context_function(transfer_t t) {
        execution_context<Fn>& e = *static_cast<execution_context<Fn>*>(t.data);
        execution_context_yield yieldfn(&e._yield);
        e._yield = jump_fcontext(t.fctx, 0).fctx;

        try {
            e._fn(yieldfn);
        } catch(const terminate_exception&) {
            // nothing
        } catch(...) {
            yield_data yde { nullptr, std::current_exception(), true };
            e._done = true;
            jump_fcontext(e._yield, &yde);
            UNREACHABLE
        }
        yield_data yd { nullptr, nullptr, false };
        e._done = true;
        jump_fcontext(e._yield, &yd);
        UNREACHABLE
    }
    void kill() {
        if(!_done) {
            resume_data rd { nullptr, true };
            _done = true;
            jump_fcontext(_resume, &rd);
        }
        if(_stack) {
            free(_stack);
            _stack = 0;
        }
    }
    bool done() {
        return _done;
    }
    const void* resume(const void* data) {
        if(_done)
            return nullptr;
        
        resume_data rd { data, false };
        transfer_t t = jump_fcontext(_resume, &rd);
        _resume = !_done ? t.fctx : 0;

        yield_data* yd = static_cast<yield_data*>(t.data);
        if(yd->has_exception) {
            std::rethrow_exception(yd->exception);
        }
        return yd->data;
    }

private:
    char* _stack;
    size_t _stack_size;
    Fn _fn;
    fcontext_t _resume;
    fcontext_t _yield;
    bool _done;
};

template <class Fn>
execution_context<Fn> make_execution_context(Fn fn) {
    return execution_context<Fn>(fn);
}
