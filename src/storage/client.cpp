#include "storage/client.hpp"
#include "storage/config.hpp"
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

std::string ClientManager::get_client_path(const std::string &client_id)
{
    return ConfigManager::get_clients_dir() + "/" + client_id + ".json";
}

bool ClientManager::client_exists(const std::string &client_id)
{
    return fs::exists(get_client_path(client_id));
}

ClientData ClientManager::load(const std::string &client_id)
{
    std::ifstream file(get_client_path(client_id));
    if (!file.is_open())
    {
        return ClientData{};
    }

    nlohmann::json j;
    file >> j;
    return j.get<ClientData>();
}

void ClientManager::save(const std::string &client_id, const ClientData &data)
{
    ConfigManager::ensure_directories();

    std::ofstream file(get_client_path(client_id));
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open client file for writing");
    }

    nlohmann::json j = data;
    file << j.dump(2);
}

void ClientManager::add_work_log(const std::string &client_id,
                                  const std::string &date,
                                  double hours,
                                  const std::string &message)
{
    ClientData data = load(client_id);

    std::string month_key = date.substr(0, 7);

    data.logs[month_key][date] = WorkLog{hours, message};

    save(client_id, data);
}

double ClientManager::get_month_total_hours(const ClientData &client,
                                             const std::string &month_key)
{
    double total = 0.0;
    auto it = client.logs.find(month_key);
    if (it != client.logs.end())
    {
        for (const auto &[date, log] : it->second)
        {
            total += log.hours;
        }
    }
    return total;
}

std::string ClientManager::get_previous_month_key()
{
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);

    if (tm.tm_mon == 0)
    {
        tm.tm_year--;
        tm.tm_mon = 11;
    }
    else
    {
        tm.tm_mon--;
    }

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m");
    return oss.str();
}

int ClientManager::increment_invoice_number(const std::string &client_id)
{
    ClientData data = load(client_id);
    int invoice_num = data.next_invoice_number;
    data.next_invoice_number++;
    save(client_id, data);
    return invoice_num;
}
