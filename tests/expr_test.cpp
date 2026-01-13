#include <fstream>

#include "fixture.h"

TEST_F(bcause, unary_operators)
{
    auto output = compile_and_run(R"(
        main() {
            extrn x;
            auto y;

            printf("global -x = %d, expect %d*n", -x, -42);
            x = 0;
            printf("global !x = %d, expect %d*n", !x, 1);
            y = 987;
            x = &y;
            printf("global **x = %d, expect %d*n", *x, y);

            printf("local -y = %d, expect %d*n", -y, -987);
            y = 0;
            printf("local !y = %d, expect %d*n", !y, 1);
            x = 42;
            y = &x;
            printf("local **y = %d, expect %d*n", *y, x);
        }

        x 42;
    )");
    const std::string expect = R"(global -x = -42, expect -42
global !x = 1, expect 1
global *x = 987, expect 987
local -y = -987, expect -987
local !y = 1, expect 1
local *y = 42, expect 42
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, negation_in_conditional_context)
{
    auto output = compile_and_run(R"(
        main() {
            extrn x;
            auto y;

            y = x + 100;
            printf("x = %d, y = %d*n", x, y);
            if (x)
                printf("if (x) WRONG*n");
            else
                printf("if (x) Correct*n");

            if (y)
                printf("if (y) Correct*n");
            else
                printf("if (y) WRONG*n");

            if (!x)
                printf("if (!x) Correct*n");
            else
                printf("if (!x) WRONG*n");

            if (!y)
                printf("if (!y) WRONG*n");
            else
                printf("if (!y) Correct*n");

            while (!x) {
                printf("while (!x) x = %d*n", x);
                x = 42;
            }
        }

        x;
    )");
    const std::string expect = R"(x = 0, y = 100
if (x) Correct
if (y) Correct
if (!x) Correct
if (!y) Correct
while (!x) x = 0
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, postfix_operators)
{
    auto output = compile_and_run(R"(
        incr(x) {
            printf("increment %d*n", x++);
            return (x);
        }

        add(a, b) {
            printf("add %d + %d*n", a, b);
            return (a + b);
        }

        decr(x) {
            printf("decrement %d*n", x--);
            return (x);
        }

        sub(a, b) {
            printf("subtract %d - %d*n", a, b);
            return (a - b);
        }

        assign_local(x) {
            auto result;
            printf("assign local %d*n", x);
            result = x;
            return (result);
        }

        assign_global(x) {
            extrn g;

            printf("assign global %d*n", x);
            g = x;
        }

        main() {
            extrn g;

            printf("%d*n", incr(42));
            printf("%d*n", add(42, 123));
            printf("%d*n", decr(42));
            printf("%d*n", sub(42, 123));
            printf("%d*n", assign_local(42));
            assign_global(42);
            printf("%d*n", g);
        }

        g;
    )");
    const std::string expect = R"(increment 42
43
add 42 + 123
165
decrement 42
41
subtract 42 - 123
-81
assign local 42
42
assign global 42
42
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, local_array)
{
    auto output = compile_and_run(R"(
        main() {
            auto l[3];

            l[0] = 123;
            l[1] = 'local';
            l[2] = "string";
            printf("local = %d, '%c', *"%s*"*n", l[0], l[1], l[2]);
        }
    )");
    const std::string expect = R"(local = 123, 'local', "string"
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, global_array)
{
    auto output = compile_and_run(R"(
        g[3] -345, 'foo', "bar";

        main() {
            extrn g;

            printf("global = %d, '%c', *"%s*"*n", g[0], g[1], g[2]);
            printf("address = %d, %d, %d*n", (&g[0]) - g, (&g[1]) - g, (&g[2]) - g);
        }
    )");
    const std::string expect = R"(global = -345, 'foo', "bar"
address = 0, 8, 16
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, local_mix)
{
    auto output = compile_and_run(R"(
        main() {
            auto e, d;
            auto c[1];
            auto b, a;
            auto p;

            a = 11;
            b = 22;
            c[0] = 33;
            d = 44;
            e = 55;

            printf("%d %d %d %d", a, b, c - &c, c[0]);
            printf(" %d %d*n", d, e);
            p = &a;
            printf("%d %d %d %d", p[0], p[1], p[2] - &c, p[3]);
            printf(" %d %d*n", p[4], p[5]);
        }
    )");
    const std::string expect = R"(11 22 8 33 44 55
11 22 8 33 44 55
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, binary_operators)
{
    auto output = compile_and_run(R"(
        x 42;

        main() {
            extrn x;
            auto y;

            y = 345;
            printf("%d + %d -> %d*n", x, y, x + y);
            printf("%d + %d -> %d*n", y, x, y + x);

            printf("%d - %d -> %d*n", x, y, x - y);
            printf("%d - %d -> %d*n", y, x, y - x);

            printf("%d ** %d -> %d*n", x, y, x * y);
            printf("%d ** %d -> %d*n", y, x, y * x);

            printf("%d / %d -> %d*n", x, y, x / y);
            printf("%d / %d -> %d*n", y, x, y / x);

            printf("%d %% %d -> %d*n", x, y, x % y);
            printf("%d %% %d -> %d*n", y, x, y % x);

            printf("%d < %d -> %d*n", x, y, x < y);
            printf("%d < %d -> %d*n", y, x, y < x);

            printf("%d <= %d -> %d*n", x, y, x <= y);
            printf("%d <= %d -> %d*n", y, x, y <= x);

            printf("%d > %d -> %d*n", x, y, x > y);
            printf("%d > %d -> %d*n", y, x, y > x);

            printf("%d >= %d -> %d*n", x, y, x >= y);
            printf("%d >= %d -> %d*n", y, x, y >= x);

            printf("%d == %d -> %d*n", x, y, x == y);
            printf("%d == %d -> %d*n", y, x, y == x);

            printf("%d != %d -> %d*n", x, y, x != y);
            printf("%d != %d -> %d*n", y, x, y != x);

            printf("%d & %d -> %d*n", x, y, x & y);
            printf("%d & %d -> %d*n", y, x, y & x);

            printf("%d | %d -> %d*n", x, y, x | y);
            printf("%d | %d -> %d*n", y, x, y | x);
        }
    )");
    const std::string expect = R"(42 + 345 -> 387
345 + 42 -> 387
42 - 345 -> -303
345 - 42 -> 303
42 * 345 -> 14490
345 * 42 -> 14490
42 / 345 -> 0
345 / 42 -> 8
42 % 345 -> 42
345 % 42 -> 9
42 < 345 -> 1
345 < 42 -> 0
42 <= 345 -> 1
345 <= 42 -> 0
42 > 345 -> 0
345 > 42 -> 1
42 >= 345 -> 0
345 >= 42 -> 1
42 == 345 -> 0
345 == 42 -> 0
42 != 345 -> 1
345 != 42 -> 1
42 & 345 -> 8
345 & 42 -> 8
42 | 345 -> 379
345 | 42 -> 379
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, eq_by_bitmask)
{
    auto output = compile_and_run(R"(

        main() {
            auto cval;

            cval = 51;
            if ((cval & 017777) == cval) {
                printf("Small positive: %d*n", cval);
            } else {
                printf("Wrong: %d*n", cval);
            }
        }
    )");
    EXPECT_EQ(output, "Small positive: 51\n");
}

TEST_F(bcause, octal_literals)
{
    auto output = compile_and_run(R"(

        main() {
            auto v;
            v = 012345;
            printf("%d*n", v);
            v = -04567;
            printf("%d*n", v);
        }
    )");
    const std::string expect = R"(5349
-2423
)";
    EXPECT_EQ(output, expect);
}
