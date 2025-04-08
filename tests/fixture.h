#ifndef TESTS_FIXTURE_H
#define TESTS_FIXTURE_H

#include <gtest/gtest.h>

#include <random>
#include <string>

//
// Fixture with preallocated machine.
//
// For details, see: https://github.com/google/googletest/blob/main/docs/primer.md
//
class bcause : public ::testing::Test {
protected:
    // Name of current test, as specified in TEST() macro.
    const std::string test_name{ ::testing::UnitTest::GetInstance()->current_test_info()->name() };

    void SetUp() override
    {
        //TODO
    }

    // Compile and run B code.
    // Return captured output.
    std::string compile_and_run(const std::string &input);
};

//
// Read file contents and return it as a string.
//
std::string file_contents(const std::string &filename);

//
// Read file contents as vector of strings.
//
std::vector<std::string> file_contents_split(const std::string &filename);

//
// Split multi-line text as vector of strings.
//
std::vector<std::string> multiline_split(const std::string &multiline);

//
// Read FILE* stream contents and return it as a string.
//
std::string stream_contents(FILE *input);

//
// Create file with given contents.
//
void create_file(const std::string &filename, const std::string &contents);
void create_file(const std::string &dest_filename, const std::string &prolog,
                 const std::string &src_filename, const std::string &epilog);

//
// Check whether string starts with given prefix.
//
bool starts_with(const std::string &str, const char *prefix);

#endif // TESTS_FIXTURE_H
