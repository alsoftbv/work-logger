#include "setup.hpp"

void add_setup_command(CLI::App &app)
{
    auto setup_command = app.add_subcommand("setup", "Setup user information");
}