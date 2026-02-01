#include <CLI/CLI.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "command/log.hpp"
#include "storage/config.hpp"
#include "storage/client.hpp"

static std::string normalize_month(const std::string &month)
{
    if (month.empty() || month.length() > 2)
        return month;

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    int month_num = std::stoi(month);
    std::ostringstream oss;
    oss << (1900 + tm.tm_year) << "-" << std::setfill('0') << std::setw(2) << month_num;
    return oss.str();
}

int main(int argc, char **argv)
{
    CLI::App app{"Work logger - log hours and generate invoices"};
    app.usage("wlog <client> <hours> <message> [date]\n"
              "       wlog <client> [OPTIONS]\n"
              "       wlog --setup [client]");

    WlogOptions opts;

    app.add_flag("--setup", opts.setup, "Run business or client setup");
    app.add_option("client", opts.client, "Client identifier");
    app.add_option("hours", opts.hours, "Hours worked");
    app.add_option("message", opts.message, "Work description");
    app.add_option("date", opts.day, "Date (YYYY-MM-DD), defaults to today");
    app.add_flag("--invoice,-i", opts.invoice, "Generate invoice for previous month");
    app.add_flag("--report,-r", opts.report, "Generate work log report");
    app.add_option("--month,-m", opts.month, "Month for report (YYYY-MM), defaults to previous month");
    app.add_flag("--show,-s", opts.show, "Show current month's work logs");
    app.add_flag("--today,-t", opts.today_only, "Show only today's log (use with -s)");

    CLI11_PARSE(app, argc, argv);

    opts.month = normalize_month(opts.month);

    if (opts.setup)
    {
        if (opts.client.empty())
        {
            run_setup();
        }
        else
        {
            if (!ConfigManager::config_exists())
            {
                std::cout << "No business configuration found. Let's set it up first." << std::endl;
                run_setup();
            }
            run_client_setup(opts.client);
        }
        return 0;
    }

    if (opts.client.empty())
    {
        if (!ConfigManager::config_exists())
        {
            std::cout << "Welcome to wlog! Let's set up your business first." << std::endl;
            run_setup();
        }
        else
        {
            std::cout << app.help() << std::endl;
        }
        return 0;
    }

    if (!ConfigManager::config_exists())
    {
        std::cout << "No business configuration found. Let's set it up first." << std::endl;
        run_setup();
    }

    if (!ClientManager::client_exists(opts.client))
    {
        std::cout << "Client '" << opts.client << "' not found. Let's set it up." << std::endl;
        run_client_setup(opts.client);

        if (opts.hours <= 0 && !opts.invoice && !opts.report)
            return 0;
    }

    if (opts.invoice)
    {
        try
        {
            run_invoice(opts);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    if (opts.report)
    {
        try
        {
            run_report(opts);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    if (opts.invoice || opts.report)
        return 0;

    if (opts.show)
    {
        run_show(opts);
        return 0;
    }

    if (opts.hours <= 0)
    {
        run_client_setup(opts.client);
        return 0;
    }

    run_log(opts);
    return 0;
}
