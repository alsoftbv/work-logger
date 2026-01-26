#include "command/log.hpp"
#include "storage/config.hpp"
#include "storage/client.hpp"
#include "flow/setup.hpp"
#include "flow/client.hpp"
#include "invoice/generator.hpp"
#include "report/work_log.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
#include <vector>
#include <algorithm>

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

void run_setup()
{
    SetupFlow::start();
}

void run_client_setup(const std::string &client)
{
    ClientFlow::start(client);
}

void run_log(const WlogOptions &opts)
{
    std::string date = opts.day.empty() ? get_today() : opts.day;

    if (!is_valid_date(date))
    {
        std::cerr << "Invalid date format. Use YYYY-MM-DD." << std::endl;
        return;
    }

    ClientManager::add_work_log(opts.client, date, opts.hours, opts.message);

    ClientData client = ClientManager::load(opts.client);
    std::cout << std::endl << "Logged " << opts.hours << " hours for " << client.name
              << " on " << date;
    if (!opts.message.empty())
        std::cout << ": " << opts.message;
    std::cout << std::endl;
}

void run_show(const WlogOptions &opts)
{
    ClientData client = ClientManager::load(opts.client);
    std::string today_date = get_today();

    std::string month_key;
    std::string month_display;

    if (opts.today_only)
    {
        month_key = today_date.substr(0, 7);
        month_display = "Today";
    }
    else if (opts.month.empty())
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
        month_key = opts.month;
        std::tm tm = {};
        std::istringstream ss(opts.month + "-01");
        ss >> std::get_time(&tm, "%Y-%m-%d");
        std::ostringstream name_oss;
        name_oss << std::put_time(&tm, "%B %Y");
        month_display = name_oss.str();
    }

    std::cout << client.name << " - " << month_display << std::endl;
    std::cout << std::string(40, '-') << std::endl;

    if (client.logs.count(month_key) == 0 || client.logs.at(month_key).empty())
    {
        std::cout << "No logs for this " << (opts.today_only ? "day" : "month") << "." << std::endl;
        return;
    }

    std::vector<std::pair<std::string, WorkLog>> sorted_logs;
    for (const auto &[date, log] : client.logs.at(month_key))
    {
        if (opts.today_only && date != today_date)
            continue;
        sorted_logs.push_back({date, log});
    }

    if (sorted_logs.empty())
    {
        std::cout << "No logs for today." << std::endl;
        return;
    }

    std::sort(sorted_logs.begin(), sorted_logs.end(),
              [](const auto &a, const auto &b) { return a.first < b.first; });

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
                  << log.message << std::endl;
        total += log.hours;
    }

    std::cout << std::string(40, '-') << std::endl;
    std::cout << "Total: " << std::fixed << std::setprecision(1) << total << " hours" << std::endl;
}

void run_invoice(const WlogOptions &opts)
{
    std::string output = InvoiceGenerator::generate(opts.client, opts.month);
    std::cout << "Invoice generated: " << output << std::endl;
}

void run_report(const WlogOptions &opts)
{
    std::string output = WorkLogReport::generate(opts.client, opts.month);
    std::cout << "Work log report generated: " << output << std::endl;
}
