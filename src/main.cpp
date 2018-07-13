
#include <cstdio>
#include <iostream>
#include <string>
#include <cstdarg>
#include <stdexcept>
#include "execution_context.h"
#include "generator.h"

enum token_type {
    TOKEN_NONE, TOKEN_EOF, TOKEN_ADD, TOKEN_IDENT
};

struct token {
    token_type t;
    std::string v;
};

char* vasprintf(const char* fmt, va_list args) {
    int size = 0;
    va_list tmpa;
    va_copy(tmpa, args);
    size = vsnprintf(NULL, size, fmt, tmpa);
    va_end(tmpa);

    if(size < 0)
        return NULL;

    char* str = (char*)malloc(size + 1);
    if(str == NULL)
        return NULL;
    size = vsprintf(str, fmt, args);
    return str;
}

char* asprintf(const char* fmt, ...) {
    int size = 0;
    va_list args;
    va_start(args, fmt);
    char* str = vasprintf(fmt, args);
    va_end(args);
    return str;
}

struct parser {
    size_t r, r0;
    std::string in;
    parser(std::string in) : r(0), r0(-1), in(std::move(in)) {}

    void operator()(generator_yield<token> yield) {
        while(true) {
            int c = getr();
            while(c == ' ' || c == '\t' || c == '\r')
                c = getr();

            if('0' <= c && c <= '9') {
                size_t x = r - 1;
                while('0' <= c && c <= '9') {
                    c = getr();
                }
                ungetr();
                yield({ TOKEN_IDENT, in.substr(x, r - x) });
                continue;
            }
            switch(c) {
                case -1:
                    yield({ TOKEN_EOF, "<eof>" });
                    break;
                case '+':
                    yield({ TOKEN_ADD, "+" });
                    break;
                default:
                    throw std::runtime_error(asprintf("unknown char <%d>", (int)c));
            }
        }
    }
    int getr() {
        r0 = r;
        if(r == in.length())
            return -1;
        int c = in[r];
        r += 1;
        return c;
    }
    void ungetr() {
        r = r0;
    }
};

int main() {
    auto gen = make_generator<token>(parser{"2 + 2"});
    token tk = { TOKEN_NONE };

    while(tk.t != TOKEN_EOF) {
        tk = *gen.next();
        std::cout << tk.t << " " << tk.v << std::endl;
    }

    return 0;
}
