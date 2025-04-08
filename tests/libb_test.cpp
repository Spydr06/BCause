#include <fstream>

#include "fixture.h"

TEST_F(bcause, libb_putchar)
{
    auto output = compile_and_run(R"(
        main() {
            putchar('Hello,');
            putchar(' World');
            putchar('!*n');
        }
    )");
    const std::string expect = "Hello, World!\n";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, libb_printf)
{
    auto output = compile_and_run(R"(
        main() {
            printf("Hello, World!*n");
            printf("%% %% %%%%*n");
            printf("format %%d: %d %d*n", 123, -123);
            printf("format %%o: %o %o*n", 234, -234);
            printf("format %%c: %c %c*n", 'foo', 'bar');
            printf("format %%s: *"%s*" *"%s*"*n", "Hello", "World");
        }
    )");
    const std::string expect = R"(Hello, World!
% % %%
format %d: 123 -123
format %o: 352 -352
format %c: foo bar
format %s: "Hello" "World"
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, libb_exit)
{
    auto output = compile_and_run(R"(
        main() {
            printf("before exit()*n");
            exit();
            printf("after exit()*n");
        }
    )");
    const std::string expect = "before exit()\n";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, libb_char)
{
    auto output = compile_and_run(R"(
        main() {
            putchar(char("fubar", 2));
            putchar(char("fubar", 4));
            putchar(char("fubar", 1));
            putchar(char("fubar", 0));
            putchar(char("fubar", 3));
            putchar('*n');
        }
    )");
    const std::string expect = "brufa\n";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, libb_lchar)
{
    auto output = compile_and_run(R"(
        main() {
            auto str;

            lchar(&str, 0, 'f');
            lchar(&str, 1, 'u');
            lchar(&str, 2, 'b');
            lchar(&str, 3, 'a');
            lchar(&str, 4, 'r');
            lchar(&str, 5, 0);
            printf("%s*n", &str);
        }
    )");
    const std::string expect = "fubar\n";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, libb_nwrite)
{
    auto output = compile_and_run(R"(
        main() {
            nwrite(1, "foobar*n", 7);
        }
    )");
    const std::string expect = "foobar\n";
    EXPECT_EQ(output, expect);
}

//TODO: read nread
