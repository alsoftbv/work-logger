#pragma once

#include <string>
#include <nlohmann/json.hpp>

struct CompanyConfig
{
    std::string name;
    std::string address_line1;
    std::string address_line2;
    std::string kvk;
    std::string btw;
    std::string bank_account;
    std::string tag;
    std::string logo_path;
    std::string currency = "EUR";

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        CompanyConfig,
        name, address_line1, address_line2, kvk, btw,
        bank_account, tag, logo_path, currency
    )
};

struct AppConfig
{
    CompanyConfig company;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(AppConfig, company)
};

class ConfigManager
{
public:
    static std::string get_config_dir();
    static std::string get_config_path();
    static std::string get_clients_dir();
    static std::string get_logos_dir();

    static bool config_exists();
    static AppConfig load();
    static void save(const AppConfig &config);
    static void ensure_directories();
};
