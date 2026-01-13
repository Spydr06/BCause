#include "fixture.h"

#include <fstream>
#include <cstring>
#include <filesystem>

static void run_command(std::string &result, const std::string &cmd)
{
    // Run simulator via shell.
    std::cout << cmd << '\n';
    FILE *pipe = popen(cmd.c_str(), "r");
    ASSERT_TRUE(pipe != nullptr);

    // Capture output.
    result = stream_contents(pipe);
    std::cout << result;

    // Check exit code.
    int exit_status = pclose(pipe);
    int exit_code   = WEXITSTATUS(exit_status);
    ASSERT_NE(exit_status, -1);
    ASSERT_EQ(exit_code, 0);
}

//
// Compile and run B code.
// Return captured output.
//
std::string bcause::compile_and_run(const std::string &source_code)
{
    const auto b_filename   = test_name + ".b";
    const auto exe_filename = test_name;

    create_file(b_filename, source_code);
    std::filesystem::remove(exe_filename);

    // Compile B source into executable binary.
    std::string result;
    run_command(result, "../bcause --save-temps -L.. " + b_filename + " -o " + exe_filename);

    // Run the binary.
    run_command(result, "./" + exe_filename);

    // Return output.
    return result;
}

//
// Read file contents and return it as a string.
//
std::string file_contents(const std::string &filename)
{
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << filename << ": " << std::strerror(errno) << std::endl;
        return "";
    }
    std::stringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

//
// Read file contents as vector of strings.
//
std::vector<std::string> split_stream(std::istream &input)
{
    std::vector<std::string> output;
    std::string line;
    while (std::getline(input, line)) {
        output.push_back(line);
    }
    return output;
}

//
// Read file contents as vector of strings.
//
std::vector<std::string> file_contents_split(const std::string &filename)
{
    std::ifstream input(filename);
    return split_stream(input);
}

//
// Read file contents as vector of strings.
//
std::vector<std::string> multiline_split(const std::string &multiline)
{
    std::stringstream input(multiline);
    return split_stream(input);
}

//
// Read FILE* stream contents until EOF and return it as a string.
//
std::string stream_contents(FILE *input)
{
    std::stringstream contents;
    char line[256];
    while (fgets(line, sizeof(line), input)) {
        contents << line;
    }
    return contents.str();
}

//
// Create file with given contents.
//
void create_file(const std::string &filename, const std::string &contents)
{
    std::ofstream output(filename);
    output << contents;
}

//
// Create file with given contents.
//
void create_file(const std::string &dest_filename, const std::string &prolog,
                 const std::string &src_filename, const std::string &epilog)
{
    std::ofstream output(dest_filename);
    EXPECT_TRUE(output.is_open()) << dest_filename;

    std::ifstream input(src_filename);
    EXPECT_TRUE(input.is_open()) << src_filename;

    output << prolog;
    output << input.rdbuf();
    output << epilog;
}

//
// Check whether string starts with given prefix.
//
bool starts_with(const std::string &str, const char *prefix)
{
    auto prefix_size = strlen(prefix);
    return str.size() >= prefix_size && memcmp(str.c_str(), prefix, prefix_size) == 0;
}
