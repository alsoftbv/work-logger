#include <gtest/gtest.h>
#include <filesystem>
#include "storage/client.hpp"
#include "storage/config.hpp"

namespace fs = std::filesystem;

class ClientTest : public ::testing::Test
{
protected:
    std::string test_dir;

    void SetUp() override
    {
        test_dir = fs::temp_directory_path() / "wlog_test_client";
        fs::create_directories(test_dir);
        setenv("HOME", test_dir.c_str(), 1);
        ConfigManager::ensure_directories();
    }

    void TearDown() override
    {
        fs::remove_all(test_dir);
    }
};

TEST_F(ClientTest, ClientNotExistsInitially)
{
    EXPECT_FALSE(ClientManager::client_exists("testclient"));
}

TEST_F(ClientTest, SaveAndLoadClient)
{
    ClientData client;
    client.name = "Test Client Corp";
    client.address_line1 = "456 Client Ave";
    client.address_line2 = "Client City";
    client.hourly_rate = 100.0;
    client.payment_term_days = 30;
    client.tag = "TCC";
    client.next_invoice_number = 1;

    ClientManager::save("testclient", client);

    EXPECT_TRUE(ClientManager::client_exists("testclient"));

    ClientData loaded = ClientManager::load("testclient");
    EXPECT_EQ(loaded.name, "Test Client Corp");
    EXPECT_EQ(loaded.hourly_rate, 100.0);
    EXPECT_EQ(loaded.payment_term_days, 30);
    EXPECT_EQ(loaded.tag, "TCC");
}

TEST_F(ClientTest, AddWorkLog)
{
    ClientData client;
    client.name = "Log Test Client";
    client.hourly_rate = 75.0;
    client.tag = "LTC";
    ClientManager::save("logclient", client);

    ClientManager::add_work_log("logclient", "2026-01-15", 8.0, "Development work");
    ClientManager::add_work_log("logclient", "2026-01-16", 6.5, "Bug fixes");

    ClientData loaded = ClientManager::load("logclient");

    ASSERT_TRUE(loaded.logs.count("2026-01") > 0);
    ASSERT_TRUE(loaded.logs["2026-01"].count("2026-01-15") > 0);
    ASSERT_TRUE(loaded.logs["2026-01"].count("2026-01-16") > 0);

    EXPECT_EQ(loaded.logs["2026-01"]["2026-01-15"].hours, 8.0);
    EXPECT_EQ(loaded.logs["2026-01"]["2026-01-15"].message, "Development work");
    EXPECT_EQ(loaded.logs["2026-01"]["2026-01-16"].hours, 6.5);
}

TEST_F(ClientTest, OverwriteSameDayLog)
{
    ClientData client;
    client.name = "Overwrite Test";
    client.tag = "OVR";
    ClientManager::save("overwriteclient", client);

    ClientManager::add_work_log("overwriteclient", "2026-01-20", 4.0, "Morning work");
    ClientManager::add_work_log("overwriteclient", "2026-01-20", 8.0, "Full day work");

    ClientData loaded = ClientManager::load("overwriteclient");
    EXPECT_EQ(loaded.logs["2026-01"]["2026-01-20"].hours, 8.0);
    EXPECT_EQ(loaded.logs["2026-01"]["2026-01-20"].message, "Full day work");
}

TEST_F(ClientTest, GetMonthTotalHours)
{
    ClientData client;
    client.name = "Hours Test";
    client.tag = "HRS";
    client.logs["2026-01"]["2026-01-10"] = {8.0, "Work 1"};
    client.logs["2026-01"]["2026-01-11"] = {6.0, "Work 2"};
    client.logs["2026-01"]["2026-01-12"] = {7.5, "Work 3"};
    client.logs["2026-02"]["2026-02-01"] = {4.0, "Feb work"};

    double jan_total = ClientManager::get_month_total_hours(client, "2026-01");
    double feb_total = ClientManager::get_month_total_hours(client, "2026-02");
    double mar_total = ClientManager::get_month_total_hours(client, "2026-03");

    EXPECT_DOUBLE_EQ(jan_total, 21.5);
    EXPECT_DOUBLE_EQ(feb_total, 4.0);
    EXPECT_DOUBLE_EQ(mar_total, 0.0);
}

TEST_F(ClientTest, IncrementInvoiceNumber)
{
    ClientData client;
    client.name = "Invoice Test";
    client.tag = "INV";
    client.next_invoice_number = 5;
    ClientManager::save("invclient", client);

    int num1 = ClientManager::increment_invoice_number("invclient");
    int num2 = ClientManager::increment_invoice_number("invclient");
    int num3 = ClientManager::increment_invoice_number("invclient");

    EXPECT_EQ(num1, 5);
    EXPECT_EQ(num2, 6);
    EXPECT_EQ(num3, 7);

    ClientData loaded = ClientManager::load("invclient");
    EXPECT_EQ(loaded.next_invoice_number, 8);
}
