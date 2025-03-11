#include <CLI/CLI.hpp>

int main(int argc, char** argv) {
    CLI::App app{"MyApp"};
    // Here your flags / options
    CLI11_PARSE(app, argc, argv);
}