#include "command/log.hpp"
#include "storage/config.hpp"
#include "storage/client.hpp"
#include "flow/setup.hpp"
#include "flow/client.hpp"
#include "invoice/generator.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>

static std::string get_today()
{
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

static bool is_valid_date(const std::string &date)
{
    std::regex date_regex("^\\d{4}-\\d{2}-\\d{2}$");
    return std::regex_match(date, date_regex);
}

struct LogOptions
{
    std::string client;
    double hours = 0.0;
    std::string message;
    std::string day;
    bool invoice = false;
};

static void run_log_command(LogOptions &opts)
{
    if (!ConfigManager::config_exists())
    {
        std::cout << "No business configuration found. Let's set it up first.\n";
        SetupFlow::start();
    }

    if (opts.invoice)
    {
        std::string output = InvoiceGenerator::generate(opts.client);
        std::cout << "Invoice generated: " << output << "\n";
        return;
    }

    if (!ClientManager::client_exists(opts.client))
    {
        std::cout << "Client '" << opts.client << "' not found. Let's set it up.\n";
        ClientFlow::start(opts.client);
    }

    if (opts.hours <= 0)
    {
        return;
    }

    std::string date = opts.day.empty() ? get_today() : opts.day;

    if (!is_valid_date(date))
    {
        std::cerr << "Invalid date format. Use YYYY-MM-DD.\n";
        return;
    }

    ClientManager::add_work_log(opts.client, date, opts.hours, opts.message);

    ClientData client = ClientManager::load(opts.client);
    std::cout << "Logged " << opts.hours << " hours for " << client.name
              << " on " << date << ": " << opts.message << "\n";
}

void add_log_command(CLI::App &app)
{
    static LogOptions opts;

    auto log_cmd = app.add_subcommand("log", "Log work hours for a client");

    log_cmd->add_option("client", opts.client, "Client identifier")
        ->required();

    log_cmd->add_option("hours", opts.hours, "Number of hours worked");

    log_cmd->add_option("message", opts.message, "Work description");

    log_cmd->add_option("day", opts.day, "Date (YYYY-MM-DD), defaults to today");

    log_cmd->add_flag("--invoice,-i", opts.invoice, "Generate invoice for previous month");

    log_cmd->callback([&]()
                      { run_log_command(opts); });
}
