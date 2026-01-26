#include <iostream>
#include <filesystem>
#include "flow/setup.hpp"
#include "flow/flow_utils.hpp"
#include "storage/config.hpp"

namespace fs = std::filesystem;

static std::string trim(const std::string &s)
{
    size_t start = s.find_first_not_of(" \t");
    if (start == std::string::npos)
        return "";
    size_t end = s.find_last_not_of(" \t");
    return s.substr(start, end - start + 1);
}

static std::string strip_quotes(const std::string &s)
{
    if (s.size() >= 2 &&
        ((s.front() == '"' && s.back() == '"') ||
         (s.front() == '\'' && s.back() == '\'')))
    {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

static std::string clean_path(const std::string &path)
{
    return strip_quotes(trim(path));
}

static std::string prompt_logo_path(const std::string &current)
{
    while (true)
    {
        std::string input = clean_path(FlowUtils::prompt_required("Logo path (JPEG)", current));

        if (input == current && !current.empty() && fs::exists(current))
            return current;

        if (!fs::exists(input))
        {
            std::cout << "File not found: " << input << std::endl;
            continue;
        }

        std::string logos_dir = ConfigManager::get_logos_dir();
        fs::create_directories(logos_dir);

        fs::path source(input);
        fs::path dest = fs::path(logos_dir) / ("logo" + source.extension().string());

        try
        {
            fs::copy_file(source, dest, fs::copy_options::overwrite_existing);
            std::cout << "Logo copied to " << dest.string() << std::endl;
            return dest.string();
        }
        catch (const fs::filesystem_error &e)
        {
            std::cout << "Could not copy logo: " << e.what() << std::endl;
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
        std::cout << "Existing configuration found. Press Enter to keep current values." << std::endl << std::endl;
    }

    auto &c = config.company;
    c.name = FlowUtils::prompt_required("Business name", c.name);
    c.address_line1 = FlowUtils::prompt_required("Address line 1", c.address_line1);
    c.address_line2 = FlowUtils::prompt_required("Address line 2 (city)", c.address_line2);
    c.kvk = FlowUtils::prompt_required("KvK number", c.kvk);
    c.btw = FlowUtils::prompt_required("BTW number", c.btw);
    c.bank_account = FlowUtils::prompt_required("Bank account (IBAN)", c.bank_account);
    c.tag = FlowUtils::prompt_required("Company tag (for invoice numbers)", c.tag);
    c.logo_path = prompt_logo_path(c.logo_path);
    c.currency = FlowUtils::prompt("Currency", c.currency.empty() ? "EUR" : c.currency);

    ConfigManager::save(config);
    FlowUtils::print_success("Business configuration saved!");
}
