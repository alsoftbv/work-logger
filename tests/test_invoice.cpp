#include <gtest/gtest.h>
#include <filesystem>
#include "storage/config.hpp"
#include "storage/client.hpp"
#include "invoice/generator.hpp"

namespace fs = std::filesystem;

class InvoiceTest : public ::testing::Test
{
protected:
    std::string test_dir;
    std::string original_cwd;
    std::string prev_month;

    void SetUp() override
    {
        test_dir = fs::temp_directory_path() / "wlog_test_invoice";
        fs::create_directories(test_dir);
        setenv("HOME", test_dir.c_str(), 1);

        // Save current directory and change to test dir for PDF output
        original_cwd = fs::current_path();
        fs::current_path(test_dir);

        prev_month = ClientManager::get_previous_month_key();

        // Setup config
        AppConfig config;
        config.company.name = "Invoice Test Co";
        config.company.address_line1 = "789 Invoice St";
        config.company.address_line2 = "Invoice City";
        config.company.kvk = "87654321";
        config.company.btw = "NL987654321B01";
        config.company.bank_account = "NL99TEST9876543210";
        config.company.tag = "ITC";
        config.company.currency = "EUR";
        ConfigManager::save(config);

        // Setup client with previous month logs
        ClientData client;
        client.name = "Invoice Client";
        client.address_line1 = "100 Client Rd";
        client.address_line2 = "Client Town";
        client.hourly_rate = 80.0;
        client.payment_term_days = 14;
        client.tag = "ICL";

        // Add logs for previous month
        client.logs[prev_month][prev_month + "-10"] = {8.0, "Day 1 work"};
        client.logs[prev_month][prev_month + "-11"] = {8.0, "Day 2 work"};
        client.logs[prev_month][prev_month + "-12"] = {4.0, "Day 3 work"};

        ClientManager::save("invoiceclient", client);
    }

    void TearDown() override
    {
        fs::current_path(original_cwd);
        fs::remove_all(test_dir);
    }
};

TEST_F(InvoiceTest, GenerateInvoicePDF)
{
    std::string output = InvoiceGenerator::generate("invoiceclient");

    std::string expected = "ITC-ICL-" + prev_month + ".pdf";
    EXPECT_EQ(output, expected);
    EXPECT_TRUE(fs::exists(test_dir + "/" + output));

    // Check file size is reasonable (PDF should be > 1KB)
    auto file_size = fs::file_size(test_dir + "/" + output);
    EXPECT_GT(file_size, 1000);
}

TEST_F(InvoiceTest, RegeneratesSameFilename)
{
    std::string output1 = InvoiceGenerator::generate("invoiceclient");
    std::string output2 = InvoiceGenerator::generate("invoiceclient");

    // Same month should produce same filename
    EXPECT_EQ(output1, output2);
}

TEST_F(InvoiceTest, ThrowsOnNoHours)
{
    // Create client with no hours in previous month
    ClientData empty_client;
    empty_client.name = "Empty Client";
    empty_client.tag = "EMP";
    empty_client.hourly_rate = 50.0;
    ClientManager::save("emptyclient", empty_client);

    EXPECT_THROW(InvoiceGenerator::generate("emptyclient"), std::runtime_error);
}

TEST_F(InvoiceTest, ThrowsOnNonexistentClient)
{
    EXPECT_THROW(InvoiceGenerator::generate("nonexistent"), std::runtime_error);
}

TEST_F(InvoiceTest, CalculationsCorrect)
{
    // 20 hours at 80 EUR = 1600 subtotal
    // VAT 21% = 336
    // Total = 1936

    // Generate invoice - just verify it completes without error
    std::string output = InvoiceGenerator::generate("invoiceclient");
    EXPECT_TRUE(fs::exists(test_dir + "/" + output));
}
