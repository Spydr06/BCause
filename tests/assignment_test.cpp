#include <fstream>

#include "fixture.h"

TEST_F(bcause, assign_add)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 10;
            x =+ 5;
            printf("%d*n", x); /* Expected: x = x + 5 = 15 */
        }
    )");
    EXPECT_EQ(output, "15\n");
}

TEST_F(bcause, assign_subtract)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 10;
            x =- 3;
            printf("%d*n", x); /* Expected: x = x - 3 = 7 */
        }
    )");
    EXPECT_EQ(output, "7\n");
}

TEST_F(bcause, assign_multiply)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 4;
            x =* 3;
            printf("%d*n", x); /* Expected: x = x * 3 = 12 */
        }
    )");
    EXPECT_EQ(output, "12\n");
}

TEST_F(bcause, assign_divide)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 15;
            x =/ 3;
            printf("%d*n", x); /* Expected: x = 15 / 3 = 5 */
        }
    )");
    EXPECT_EQ(output, "5\n");
}

TEST_F(bcause, assign_modulo)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 17;
            x =% 5;
            printf("%d*n", x); /* Expected: x = 17 % 5 = 2 */
        }
    )");
    EXPECT_EQ(output, "2\n");
}

TEST_F(bcause, assign_shift_left)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 2;
            x =<< 2;
            printf("%d*n", x); /* Expected: x = 2 << 2 = 8 */
        }
    )");
    EXPECT_EQ(output, "8\n");
}

TEST_F(bcause, assign_less_or_equal)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 5;
            x =<= 3;
            printf("%d*n", x); /* Expected: x = (5 <= 3) = 0 (false) */
            x = 2;
            x =<= 3;
            printf("%d*n", x); /* Expected: x = (2 <= 3) = 1 (true) */
        }
    )");
    EXPECT_EQ(output, "0\n1\n");
}

TEST_F(bcause, assign_less_than)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 5;
            x =< 5;
            printf("%d*n", x); /* Expected: x = (5 < 5) = 0 (false) */
            x = 4;
            x =< 5;
            printf("%d*n", x); /* Expected: x = (4 < 5) = 1 (true) */
        }
    )");
    EXPECT_EQ(output, "0\n1\n");
}

TEST_F(bcause, assign_shift_right)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 16;
            x =>> 2;
            printf("%d*n", x); /* Expected: x = 16 >> 2 = 4 */
        }
    )");
    EXPECT_EQ(output, "4\n");
}

TEST_F(bcause, assign_greater_or_equal)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 3;
            x =>= 5;
            printf("%d*n", x); /* Expected: x = (3 >= 5) = 0 (false) */
            x = 5;
            x =>= 5;
            printf("%d*n", x); /* Expected: x = (5 >= 5) = 1 (true) */
        }
    )");
    EXPECT_EQ(output, "0\n1\n");
}

TEST_F(bcause, assign_greater_than)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 4;
            x => 5;
            printf("%d*n", x); /* Expected: x = (4 > 5) = 0 (false) */
            x = 6;
            x => 5;
            printf("%d*n", x); /* Expected: x = (6 > 5) = 1 (true) */
        }
    )");
    EXPECT_EQ(output, "0\n1\n");
}

TEST_F(bcause, assign_not_equal)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 5;
            x =!= 5;
            printf("%d*n", x); /* Expected: x = (5 != 5) = 0 (false) */
            x = 5;
            x =!= 3;
            printf("%d*n", x); /* Expected: x = (5 != 3) = 1 (true) */
        }
    )");
    EXPECT_EQ(output, "0\n1\n");
}

TEST_F(bcause, assign_equal)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 5;
            x === 5;
            printf("%d*n", x); /* Expected: x = (5 == 5) = 1 (true) */
            x = 5;
            x === 6;
            printf("%d*n", x); /* Expected: x = (5 == 6) = 0 (false) */
        }
    )");
    EXPECT_EQ(output, "1\n0\n");
}

TEST_F(bcause, assign_and)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 12;            /* 1100 in binary */
            x =& 10;           /* 1010 in binary */
            printf("%d*n", x); /* Expected: x = 12 & 10 = 8 (1000) */
        }
    )");
    EXPECT_EQ(output, "8\n");
}

TEST_F(bcause, assign_or)
{
    auto output = compile_and_run(R"(
        main() {
            auto x;
            x = 12;             /* 1100 in binary */
            x =| 10;            /* 1010 in binary */
            printf("%d*n", x);  /* Expected: x = 12 | 10 = 14 (1110) */
        }
    )");
    EXPECT_EQ(output, "14\n");
}
