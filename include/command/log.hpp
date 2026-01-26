#pragma once

#include <string>

struct WlogOptions
{
    std::string client;
    double hours = 0.0;
    std::string message;
    std::string day;
    std::string month;
    bool setup = false;
    bool invoice = false;
    bool report = false;
    bool show = false;
    bool today_only = false;
};

void run_setup();
void run_client_setup(const std::string &client);
void run_log(const WlogOptions &opts);
void run_show(const WlogOptions &opts);
void run_invoice(const WlogOptions &opts);
void run_report(const WlogOptions &opts);
