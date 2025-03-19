#include <CLI/CLI.hpp>

#include "command/setup.hpp"

int main(int argc, char **argv)
{
    CLI::App app{"Work logger helps you log your daily work"};

    setup_commands(app);

    CLI11_PARSE(app, argc, argv);

    return 0;
}