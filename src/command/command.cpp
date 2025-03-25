#include "command/command.hpp"
#include "command/setup.hpp"

#define MIN_SUBCOMMAND 0
#define MAX_SUBCOMMAND 1

void add_commands(CLI::App &app);

void setup_commands(CLI::App &app)
{
    app.require_subcommand(MIN_SUBCOMMAND, MAX_SUBCOMMAND);

    add_commands(app);
}

void add_commands(CLI::App &app)
{
    add_setup_command(app);
}
