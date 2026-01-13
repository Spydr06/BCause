#include <fstream>

#include "fixture.h"

TEST_F(bcause, example_hello_putchar)
{
    auto output = compile_and_run(R"(
        main() {
            putchar('Hello, W');
            putchar('orld!*n');
        }
    )");
    EXPECT_EQ(output, "Hello, World!\n");
}

TEST_F(bcause, example_hello_printf)
{
    auto output = compile_and_run(file_contents(TEST_DIR "/../examples/helloworld.b"));
    EXPECT_EQ(output, "Hello, World!\n");
}
