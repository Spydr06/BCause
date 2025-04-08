#include <fstream>

#include "fixture.h"

TEST_F(bcause, precedence_add_mul)
{
    auto output = compile_and_run(R"(
        main() {
            printf("3 + 4 ** 2 -> %d*n", 3 + 4 * 2);
        }
    )");
    EXPECT_EQ(output,  "3 + 4 * 2 -> 11\n");
}

TEST_F(bcause, precedence_mul_add_mul)
{
    auto output = compile_and_run(R"(
        main() {
            printf("5 ** 2 + 3 ** 4 -> %d*n", 5 * 2 + 3 * 4);
        }
    )");
    EXPECT_EQ(output, "5 * 2 + 3 * 4 -> 22\n");
}

TEST_F(bcause, precedence_sub_div)
{
    auto output = compile_and_run(R"(
        main() {
            printf("10 - 6 / 2 -> %d*n", 10 - 6 / 2);
        }
    )");
    EXPECT_EQ(output, "10 - 6 / 2 -> 7\n");
}

TEST_F(bcause, precedence_mod_add)
{
    auto output = compile_and_run(R"(
        main() {
            printf("7 %% 3 + 2 -> %d*n", 7 % 3 + 2);
        }
    )");
    EXPECT_EQ(output, "7 % 3 + 2 -> 3\n");
}

TEST_F(bcause, precedence_add_lt)
{
    auto output = compile_and_run(R"(
        main() {
            printf("5 + 3 < 9 -> %d*n", 5 + 3 < 9);
        }
    )");
    EXPECT_EQ(output, "5 + 3 < 9 -> 1\n");
}

TEST_F(bcause, precedence_lt_eq)
{
    auto output = compile_and_run(R"(
        main() {
            printf("4 < 6 == 1 -> %d*n", 4 < 6 == 1);
        }
    )");
    EXPECT_EQ(output, "4 < 6 == 1 -> 1\n");
}

TEST_F(bcause, precedence_eq_and)
{
    auto output = compile_and_run(R"(
        main() {
            printf("3 == 3 & 1 -> %d*n", 3 == 3 & 1);
        }
    )");
    EXPECT_EQ(output, "3 == 3 & 1 -> 1\n");
}

TEST_F(bcause, precedence_and_or)
{
    auto output = compile_and_run(R"(
        main() {
            printf("2 & 3 | 4 -> %d*n", 2 & 3 | 4);
        }
    )");
    EXPECT_EQ(output, "2 & 3 | 4 -> 6\n");
}

TEST_F(bcause, precedence_mul_add_lt)
{
    auto output = compile_and_run(R"(
        main() {
            printf("2 ** 3 + 4 < 11 -> %d*n", 2 * 3 + 4 < 11);
        }
    )");
    EXPECT_EQ(output, "2 * 3 + 4 < 11 -> 1\n");
}

TEST_F(bcause, precedence_mul_ge_eq)
{
    auto output = compile_and_run(R"(
        main() {
            printf("5 ** 2 >= 10 == 1 -> %d*n", 5 * 2 >= 10 == 1);
        }
    )");
    EXPECT_EQ(output, "5 * 2 >= 10 == 1 -> 1\n");
}

TEST_F(bcause, precedence_mul_and_add)
{
    auto output = compile_and_run(R"(
        main() {
            printf("4 ** 2 & 3 + 1 -> %d*n", 4 * 2 & 3 + 1);
        }
    )");
    EXPECT_EQ(output, "4 * 2 & 3 + 1 -> 0\n");
}

TEST_F(bcause, precedence_div_add_gt_or)
{
    auto output = compile_and_run(R"(
        main() {
            printf("6 / 2 + 1 > 3 | 2 -> %d*n", 6 / 2 + 1 > 3 | 2);
        }
    )");
    EXPECT_EQ(output, "6 / 2 + 1 > 3 | 2 -> 3\n");
}

TEST_F(bcause, precedence_div_mod)
{
    auto output = compile_and_run(R"(
        main() {
            printf("10 / 2 %% 3 -> %d*n", 10 / 2 % 3);
        }
    )");
    EXPECT_EQ(output, "10 / 2 % 3 -> 2\n");
}

TEST_F(bcause, precedence_mul_or)
{
    auto output = compile_and_run(R"(
        main() {
            printf("0 ** 5 | 3 -> %d*n", 0 * 5 | 3);
        }
    )");
    EXPECT_EQ(output, "0 * 5 | 3 -> 3\n");
}

TEST_F(bcause, precedence_mul_lshift)
{
    auto output = compile_and_run(R"(
        main() {
            printf("4 ** 3 << 2 -> %d*n", 4 * 3 << 2);
        }
    )");
    EXPECT_EQ(output, "4 * 3 << 2 -> 48\n");
}

TEST_F(bcause, precedence_lshift_lt)
{
    auto output = compile_and_run(R"(
        main() {
            printf("1 << 2 < 5 -> %d*n", 1 << 2 < 5);
        }
    )");
    EXPECT_EQ(output, "1 << 2 < 5 -> 1\n");
}

TEST_F(bcause, precedence_sub_rshift)
{
    auto output = compile_and_run(R"(
        main() {
            printf("16 - 8 >> 1 -> %d*n", 16 - 8 >> 1);
        }
    )");
    EXPECT_EQ(output, "16 - 8 >> 1 -> 4\n");
}

TEST_F(bcause, precedence_lshift_and)
{
    auto output = compile_and_run(R"(
        main() {
            printf("3 << 2 & 7 -> %d*n", 3 << 2 & 7);
        }
    )");
    EXPECT_EQ(output, "3 << 2 & 7 -> 4\n");
}

TEST_F(bcause, precedence_or_rshift)
{
    auto output = compile_and_run(R"(
        main() {
            printf("2 | 4 >> 1 -> %d*n", 2 | 4 >> 1);
        }
    )");
    EXPECT_EQ(output, "2 | 4 >> 1 -> 2\n");
}

TEST_F(bcause, precedence_rshift_eq)
{
    auto output = compile_and_run(R"(
        main() {
            printf("8 >> 2 == 2 -> %d*n", 8 >> 2 == 2);
        }
    )");
    EXPECT_EQ(output, "8 >> 2 == 2 -> 1\n");
}

TEST_F(bcause, precedence_mul_lshift_add)
{
    auto output = compile_and_run(R"(
        main() {
            printf("5 ** 2 << 1 + 3 -> %d*n", 5 * 2 << 1 + 3);
        }
    )");
    EXPECT_EQ(output, "5 * 2 << 1 + 3 -> 160\n");
}

TEST_F(bcause, precedence_mod_lshift)
{
    auto output = compile_and_run(R"(
        main() {
            printf("15 %% 4 << 2 -> %d*n", 15 % 4 << 2);
        }
    )");
    EXPECT_EQ(output, "15 % 4 << 2 -> 12\n");
}

TEST_F(bcause, precedence_lshift_gt_and)
{
    auto output = compile_and_run(R"(
        main() {
            printf("1 << 3 > 5 & 2 -> %d*n", 1 << 3 > 5 & 2);
        }
    )");
    EXPECT_EQ(output, "1 << 3 > 5 & 2 -> 0\n");
}

TEST_F(bcause, precedence_add_lshift)
{
    auto output = compile_and_run(R"(
        main() {
            printf("12345 + 10 << 4 -> %d*n", 12345 + 10 << 4);
        }
    )");
    EXPECT_EQ(output, "12345 + 10 << 4 -> 197680\n");

}

TEST_F(bcause, precedence_div_rshift)
{
    auto output = compile_and_run(R"(
        main() {
            printf("16 / 2 >> 1 -> %d*n", 16 / 2 >> 1);
        }
    )");
    EXPECT_EQ(output, "16 / 2 >> 1 -> 4\n");

}

TEST_F(bcause, precedence_and_lshift_or)
{
    auto output = compile_and_run(R"(
        main() {
            printf("7 & 3 << 2 | 8 -> %d*n", 7 & 3 << 2 | 8);
        }
    )");
    EXPECT_EQ(output, "7 & 3 << 2 | 8 -> 12\n");

}

TEST_F(bcause, precedence_lshift_ne)
{
    auto output = compile_and_run(R"(
        main() {
            printf("1 << 4 != 15 -> %d*n", 1 << 4 != 15);
        }
    )");
    EXPECT_EQ(output, "1 << 4 != 15 -> 1\n");

}

TEST_F(bcause, precedence_rshift_ge)
{
    auto output = compile_and_run(R"(
        main() {
            printf("98765 >> 3 >= 12345 -> %d*n", 98765 >> 3 >= 12345);
        }
    )");
    EXPECT_EQ(output, "98765 >> 3 >= 12345 -> 1\n");
}
