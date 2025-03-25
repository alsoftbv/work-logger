#include "command/setup.hpp"
#include "flow/setup.hpp"

void add_setup_command(CLI::App &app)
{
    auto setup_command = app.add_subcommand("setup", "Setup user information");
    setup_command->callback(SetupFlow::start);
}
