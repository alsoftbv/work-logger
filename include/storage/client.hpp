#pragma once

#include <string>
#include <map>
#include <nlohmann/json.hpp>

struct WorkLog
{
    double hours;
    std::string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WorkLog, hours, message)
};

struct ClientData
{
    std::string name;
    std::string address_line1;
    std::string address_line2;
    double hourly_rate = 0.0;
    int payment_term_days = 14;
    std::string tag;
    int next_invoice_number = 1;
    std::map<std::string, std::map<std::string, WorkLog>> logs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        ClientData,
        name, address_line1, address_line2, hourly_rate,
        payment_term_days, tag, next_invoice_number, logs
    )
};

class ClientManager
{
public:
    static std::string get_client_path(const std::string &client_id);
    static bool client_exists(const std::string &client_id);
    static ClientData load(const std::string &client_id);
    static void save(const std::string &client_id, const ClientData &data);

    static void add_work_log(const std::string &client_id,
                             const std::string &date,
                             double hours,
                             const std::string &message);

    static double get_month_total_hours(const ClientData &client,
                                         const std::string &month_key);

    static std::string get_previous_month_key();
    static int increment_invoice_number(const std::string &client_id);
};
