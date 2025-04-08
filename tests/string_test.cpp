#include <fstream>

#include "fixture.h"

TEST_F(bcause, string_literals)
{
    auto output = compile_and_run(R"(
        sa "*t*0x";
        sb "foo*ebar";
        sc "bar";

        main() {
            extrn sa, sb;

            printf("*(*)***"*n");
            printf("%d %d %d*n", char(sa, 0), char(sa, 1), char(sa, 2));
            printf("%d %d %d %d %d*n", char(sb, 0), char(sb, 1), char(sb, 2), char(sb, 3), char(sb, 4));
        }
    )");
    const std::string expect = R"(()*"
9 0 102
102 111 111 0 98
)";
    EXPECT_EQ(output, expect);
}

TEST_F(bcause, char_literals)
{
    auto output = compile_and_run(R"(
        ca '*t*0x';
        cb 'foo*ebar';
        cc 'bar';

        main() {
            extrn sa, sb;

            printf("%d*n", '*0');
            printf("%d*n", '*e');
            printf("%d*n", '*t');
            printf("%d*n", '*n');
            printf("%d*n", '*r');
            printf("%c*n", '*(*)***'*"');
        }
    )");
    const std::string expect = R"(0
0
9
10
13
()*'"
)";
    EXPECT_EQ(output, expect);
}
