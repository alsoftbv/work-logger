#include "flow/flow_utils.hpp"
#include <iostream>
#include <sstream>
#include <limits>

namespace FlowUtils
{
    std::string prompt(const std::string &label, const std::string &current_value)
    {
        if (!current_value.empty())
        {
            std::cout << label << " [" << current_value << "]: ";
        }
        else
        {
            std::cout << label << ": ";
        }

        std::string input;
        std::getline(std::cin, input);

        if (input.empty() && !current_value.empty())
        {
            return current_value;
        }

        return input;
    }

    std::string prompt_required(const std::string &label, const std::string &current_value)
    {
        while (true)
        {
            std::string result = prompt(label, current_value);
            if (!result.empty())
            {
                return result;
            }
            std::cout << "This field is required.\n";
        }
    }

    double prompt_double(const std::string &label, double current_value)
    {
        while (true)
        {
            std::string current_str = (current_value > 0) ? std::to_string(current_value) : "";
            if (!current_str.empty())
            {
                size_t dot_pos = current_str.find('.');
                if (dot_pos != std::string::npos)
                {
                    current_str = current_str.substr(0, dot_pos + 3);
                }
            }

            std::string input = prompt(label, current_str);

            if (input.empty() && current_value > 0)
            {
                return current_value;
            }

            try
            {
                double value = std::stod(input);
                if (value > 0)
                {
                    return value;
                }
                std::cout << "Please enter a positive number.\n";
            }
            catch (...)
            {
                std::cout << "Invalid number. Please try again.\n";
            }
        }
    }

    int prompt_int(const std::string &label, int current_value)
    {
        while (true)
        {
            std::string current_str = (current_value > 0) ? std::to_string(current_value) : "";
            std::string input = prompt(label, current_str);

            if (input.empty() && current_value > 0)
            {
                return current_value;
            }

            try
            {
                int value = std::stoi(input);
                if (value > 0)
                {
                    return value;
                }
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
