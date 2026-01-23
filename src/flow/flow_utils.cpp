#include "flow/flow_utils.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace FlowUtils
{
    std::string prompt(const std::string &label, const std::string &current)
    {
        if (!current.empty())
            std::cout << label << " [" << current << "]: ";
        else
            std::cout << label << ": ";

        std::string input;
        std::getline(std::cin, input);
        return input.empty() && !current.empty() ? current : input;
    }

    std::string prompt_required(const std::string &label, const std::string &current)
    {
        while (true)
        {
            std::string result = prompt(label, current);
            if (!result.empty())
                return result;
            std::cout << "This field is required.\n";
        }
    }

    double prompt_double(const std::string &label, double current)
    {
        while (true)
        {
            std::string current_str;
            if (current > 0)
            {
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << current;
                current_str = oss.str();
            }

            std::string input = prompt(label, current_str);
            if (input.empty() && current > 0)
                return current;

            try
            {
                double value = std::stod(input);
                if (value > 0)
                    return value;
                std::cout << "Please enter a positive number.\n";
            }
            catch (...)
            {
                std::cout << "Invalid number. Please try again.\n";
            }
        }
    }

    int prompt_int(const std::string &label, int current)
    {
        while (true)
        {
            std::string current_str = current > 0 ? std::to_string(current) : "";
            std::string input = prompt(label, current_str);

            if (input.empty() && current > 0)
                return current;

            try
            {
                int value = std::stoi(input);
                if (value > 0)
                    return value;
                std::cout << "Please enter a positive number.\n";
            }
            catch (...)
            {
                std::cout << "Invalid number. Please try again.\n";
            }
        }
    }

    void print_header(const std::string &title)
    {
        std::cout << "\n=== " << title << " ===\n\n";
    }

    void print_success(const std::string &message)
    {
        std::cout << "\n" << message << "\n";
    }
}
