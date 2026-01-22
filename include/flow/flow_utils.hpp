#pragma once

#include <string>
#include <functional>

namespace FlowUtils
{
    std::string prompt(const std::string &label, const std::string &current_value = "");

    std::string prompt_required(const std::string &label, const std::string &current_value = "");

    double prompt_double(const std::string &label, double current_value = 0.0);

    int prompt_int(const std::string &label, int current_value = 0);

    void print_header(const std::string &title);

    void print_success(const std::string &message);
}
