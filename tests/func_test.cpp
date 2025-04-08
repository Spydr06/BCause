#include <fstream>

#include "fixture.h"

TEST_F(bcause, function_definitions)
{
    auto output = compile_and_run(R"(
        a() {}
        b();
        c() label:;
        d() label: goto label;
        e() return;
        f(x) return(x);
        g(x) x;
        h(x) if(x) 123;
        i(x) if(x) 123; else 456;
        j(x) while(x);
        k(x) switch(x);
        l(x) switch(x) case 1:;
        m() extrn x;
        n() auto x;

        main() {
            printf("before a()*n");
            a();    printf("after a(), before b()*n");
            b();    printf("after b(), before c()*n");
            c();    printf("after c(), before e()*n");
            e();    printf("after e(), before f()*n");
            f(42);  printf("after f(), before g()*n");
            g(42);  printf("after g(), before h()*n");
            h(42);  printf("after h(), before i()*n");
            i(42);  printf("after i(), before j()*n");
            j(0);   printf("after j(), before k()*n");
            k(42);  printf("after k(), before l()*n");
            l(42);  printf("after l(), before m()*n");
            m(42);  printf("after m(), before n()*n");
            n(42);  printf("after n()*n");
        }
    )");
    const std::string expect = R"(before a()
after a(), before b()
after b(), before c()
after c(), before e()
after e(), before f()
after f(), before g()
after g(), before h()
after h(), before i()
after i(), before j()
after j(), before k()
after k(), before l()
after l(), before m()
after m(), before n()
after n()
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, function_arguments)
{
    auto output = compile_and_run(R"(
        func(a, b, c)
        {
            printf("a = %d, b = '%c', c = *"%s*"*n", a, b, c);
        }

        main() {
            func(123, 'foo', "bar");
        }
    )");
    const std::string expect = R"(a = 123, b = 'foo', c = "bar"
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, function_ternary_operator)
{
    auto output = compile_and_run(R"(
        choose(a, b, c)
        {
            return (a ? b : c);
        }

        main() {
            printf("%d*n", choose(1, 123, 456));
            printf("%d*n", choose(0, 123, 456));
        }
    )");
    const std::string expect = R"(123
456
)";
    EXPECT_EQ(output, expect);
}
