#include "storage/config.hpp"
#include <fstream>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

std::string ConfigManager::get_config_dir()
{
    const char *home = std::getenv("HOME");
    if (!home)
    {
        throw std::runtime_error("HOME environment variable not set");
    }
    return std::string(home) + "/.wlog";
}

std::string ConfigManager::get_config_path()
{
    return get_config_dir() + "/config.json";
}

std::string ConfigManager::get_clients_dir()
{
    return get_config_dir() + "/clients";
}

std::string ConfigManager::get_logos_dir()
{
    return get_config_dir() + "/logos";
}

bool ConfigManager::config_exists()
{
    return fs::exists(get_config_path());
}

void ConfigManager::ensure_directories()
{
    fs::create_directories(get_config_dir());
    fs::create_directories(get_clients_dir());
    fs::create_directories(get_logos_dir());
}

AppConfig ConfigManager::load()
{
    std::ifstream file(get_config_path());
    if (!file.is_open())
    {
        return AppConfig{};
    }

    nlohmann::json j;
    file >> j;
    return j.get<AppConfig>();
}

void ConfigManager::save(const AppConfig &config)
{
    ensure_directories();

    std::ofstream file(get_config_path());
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open config file for writing");
    }

    nlohmann::json j = config;
    file << j.dump(2);
}
