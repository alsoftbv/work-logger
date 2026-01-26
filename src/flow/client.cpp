#include "flow/client.hpp"
#include "flow/flow_utils.hpp"
#include "storage/client.hpp"
#include <iostream>

void ClientFlow::start(const std::string &client_id)
{
    FlowUtils::print_header("Client Setup: " + client_id);

    ClientData data;
    if (ClientManager::client_exists(client_id))
    {
        data = ClientManager::load(client_id);
        std::cout << "Existing client found. Press Enter to keep current values." << std::endl << std::endl;
    }
    else
    {
        std::cout << "Setting up new client: " << client_id << std::endl << std::endl;
    }

    data.name = FlowUtils::prompt_required("Client business name", data.name);
    data.address_line1 = FlowUtils::prompt_required("Address line 1", data.address_line1);
    data.address_line2 = FlowUtils::prompt_required("Address line 2 (city)", data.address_line2);
    data.hourly_rate = FlowUtils::prompt_double("Hourly rate", data.hourly_rate);
    data.payment_term_days = FlowUtils::prompt_int("Payment term (days)", data.payment_term_days > 0 ? data.payment_term_days : 14);
    data.tag = FlowUtils::prompt_required("Client tag (for invoice numbers)", data.tag);

    ClientManager::save(client_id, data);

    FlowUtils::print_success("Client configuration saved!");
}
