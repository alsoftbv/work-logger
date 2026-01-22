#include <CLI/CLI.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <vector>
#include <algorithm>

#include "storage/config.hpp"
#include "storage/client.hpp"
#include "flow/setup.hpp"
#include "flow/client.hpp"
#include "invoice/generator.hpp"
#include "report/work_log.hpp"

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

int main(int argc, char **argv)
{
    CLI::App app{"Work logger - log hours and generate invoices"};

    bool run_setup = false;
    std::string client;
    double hours = 0.0;
    std::string message;
    std::string day;
    bool invoice = false;
    bool report = false;
    std::string report_month;
    bool show = false;

    app.add_flag("--setup,-s", run_setup, "Run business setup");
    app.add_option("client", client, "Client identifier");
    app.add_option("hours", hours, "Hours worked");
    app.add_option("message", message, "Work description");
    app.add_option("day", day, "Date (YYYY-MM-DD), defaults to today");
    app.add_flag("--invoice,-i", invoice, "Generate invoice for previous month");
    app.add_flag("--report,-r", report, "Generate work log report");
    app.add_option("--month,-m", report_month, "Month for report (YYYY-MM), defaults to previous month");
    app.add_flag("--show", show, "Show current month's work logs");

    CLI11_PARSE(app, argc, argv);

    if (!report_month.empty() && report_month.length() <= 2)
    {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&now_time);
        int month_num = std::stoi(report_month);
        std::ostringstream oss;
        oss << (1900 + tm.tm_year) << "-" << std::setfill('0') << std::setw(2) << month_num;
        report_month = oss.str();
    }

    if (run_setup)
    {
        SetupFlow::start();
        return 0;
    }

    if (client.empty())
    {
        if (!ConfigManager::config_exists())
        {
            std::cout << "Welcome to wlog! Let's set up your business first.\n";
            SetupFlow::start();
        }
        else
        {
            std::cout << app.help() << std::endl;
        }
        return 0;
    }

    if (!ConfigManager::config_exists())
    {
        std::cout << "No business configuration found. Let's set it up first.\n";
        SetupFlow::start();
    }

    if (!ClientManager::client_exists(client))
    {
        std::cout << "Client '" << client << "' not found. Let's set it up.\n";
        ClientFlow::start(client);

        if (hours <= 0 && !invoice)
        {
            return 0;
        }
    }

    if (invoice || report)
    {
        if (invoice)
        {
            try
            {
                std::string output = InvoiceGenerator::generate(client, report_month);
                std::cout << "Invoice generated: " << output << "\n";
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            }
        }

        if (report)
        {
            try
            {
                std::string output = WorkLogReport::generate(client, report_month);
                std::cout << "Work log report generated: " << output << "\n";
            }
            catch (const std::exception &e)
            {
                std::cerr << "Error: " << e.what() << "\n";
                return 1;
            }
        }

        return 0;
    }

    if (show)
    {
        ClientData client_data = ClientManager::load(client);

        std::string month_key;
        std::string month_display;

        if (report_month.empty())
        {
            auto now = std::chrono::system_clock::now();
            auto now_time = std::chrono::system_clock::to_time_t(now);
            std::tm tm = *std::localtime(&now_time);
            std::ostringstream month_oss;
            month_oss << std::put_time(&tm, "%Y-%m");
            month_key = month_oss.str();
            std::ostringstream name_oss;
            name_oss << std::put_time(&tm, "%B %Y");
            month_display = name_oss.str();
        }
        else
        {
            month_key = report_month;
            std::tm tm = {};
            std::istringstream ss(report_month + "-01");
            ss >> std::get_time(&tm, "%Y-%m-%d");
            std::ostringstream name_oss;
            name_oss << std::put_time(&tm, "%B %Y");
            month_display = name_oss.str();
        }

        std::cout << client_data.name << " - " << month_display << "\n";
        std::cout << std::string(40, '-') << "\n";

        if (client_data.logs.count(month_key) == 0 || client_data.logs.at(month_key).empty())
        {
            std::cout << "No logs for this month.\n";
            return 0;
        }

        std::vector<std::pair<std::string, WorkLog>> sorted_logs;
        for (const auto &[date, log] : client_data.logs.at(month_key))
        {
            sorted_logs.push_back({date, log});
        }
        std::sort(sorted_logs.begin(), sorted_logs.end(),
                  [](const auto &a, const auto &b)
                  { return a.first < b.first; });

        double total = 0.0;
        for (const auto &[date, log] : sorted_logs)
        {
            std::tm dtm = {};
            std::istringstream dss(date);
            dss >> std::get_time(&dtm, "%Y-%m-%d");
            std::ostringstream doss;
            doss << std::put_time(&dtm, "%b %d");

            std::cout << doss.str() << "   "
                      << std::fixed << std::setprecision(1) << log.hours << "h   "
                      << log.message << "\n";
            total += log.hours;
        }

        std::cout << std::string(40, '-') << "\n";
        std::cout << "Total: " << std::fixed << std::setprecision(1) << total << " hours\n";
        return 0;
    }

    if (hours <= 0)
    {
        ClientFlow::start(client);
        return 0;
    }

    std::string date = day.empty() ? get_today() : day;

    if (!is_valid_date(date))
    {
        std::cerr << "Invalid date format. Use YYYY-MM-DD.\n";
        return 1;
    }

    ClientManager::add_work_log(client, date, hours, message);

    ClientData client_data = ClientManager::load(client);
    std::cout << "Logged " << hours << " hours for " << client_data.name
              << " on " << date;
    if (!message.empty())
    {
        std::cout << ": " << message;
    }
    std::cout << "\n";

    return 0;
}
