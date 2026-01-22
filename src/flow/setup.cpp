#include <iostream>
#include <filesystem>
#include "flow/setup.hpp"
#include "flow/flow_utils.hpp"
#include "storage/config.hpp"

namespace fs = std::filesystem;

static std::string clean_path(const std::string &path)
{
    std::string result = path;

    // Trim whitespace
    size_t start = result.find_first_not_of(" \t");
    size_t end = result.find_last_not_of(" \t");
    if (start == std::string::npos)
        return "";
    result = result.substr(start, end - start + 1);

    // Strip surrounding quotes
    if (result.size() >= 2)
    {
        if ((result.front() == '"' && result.back() == '"') ||
            (result.front() == '\'' && result.back() == '\''))
        {
            result = result.substr(1, result.size() - 2);
        }
    }

    return result;
}

static std::string prompt_logo_path(const std::string &current_value)
{
    while (true)
    {
        std::string raw_input = FlowUtils::prompt_required("Logo path", current_value);
        std::string input = clean_path(raw_input);

        // If keeping existing value that's already in .wlog, accept it
        if (input == current_value && !current_value.empty() && fs::exists(current_value))
        {
            return current_value;
        }

        if (!fs::exists(input))
        {
            std::cout << "File not found: " << input << "\n";
            continue;
        }

        // Copy to .wlog/logos
        std::string logos_dir = ConfigManager::get_logos_dir();
        fs::create_directories(logos_dir);

        fs::path source(input);
        fs::path dest = fs::path(logos_dir) / ("logo" + source.extension().string());

        try
        {
            fs::copy_file(source, dest, fs::copy_options::overwrite_existing);
            std::cout << "Logo copied to " << dest.string() << "\n";
            return dest.string();
        }
        catch (const fs::filesystem_error &e)
        {
            std::cout << "Could not copy logo: " << e.what() << "\n";
            continue;
        }
    }
}

void SetupFlow::start()
{
    FlowUtils::print_header("Business Setup");

    AppConfig config;
    if (ConfigManager::config_exists())
    {
        config = ConfigManager::load();
        std::cout << "Existing configuration found. Press Enter to keep current values.\n\n";
    }

    auto &company = config.company;

    company.name = FlowUtils::prompt_required("Business name", company.name);
    company.address_line1 = FlowUtils::prompt_required("Address line 1", company.address_line1);
    company.address_line2 = FlowUtils::prompt_required("Address line 2 (city)", company.address_line2);
    company.kvk = FlowUtils::prompt_required("KvK number", company.kvk);
    company.btw = FlowUtils::prompt_required("BTW number", company.btw);
    company.bank_account = FlowUtils::prompt_required("Bank account (IBAN)", company.bank_account);
    company.tag = FlowUtils::prompt_required("Company tag (for invoice numbers)", company.tag);

    company.logo_path = prompt_logo_path(company.logo_path);

    company.currency = FlowUtils::prompt("Currency", company.currency.empty() ? "EUR" : company.currency);

    ConfigManager::save(config);

    FlowUtils::print_success("Business configuration saved successfully!");
}
