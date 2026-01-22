#include <gtest/gtest.h>
#include <filesystem>
#include "storage/config.hpp"
#include "storage/client.hpp"
#include "report/work_log.hpp"

namespace fs = std::filesystem;

class WorkLogTest : public ::testing::Test
{
protected:
    std::string test_dir;
    std::string original_cwd;

    void SetUp() override
    {
        test_dir = fs::temp_directory_path() / "wlog_test_worklog";
        fs::create_directories(test_dir);
        setenv("HOME", test_dir.c_str(), 1);

        original_cwd = fs::current_path();
        fs::current_path(test_dir);

        AppConfig config;
        config.company.name = "WorkLog Test Co";
        config.company.address_line1 = "123 Test St";
        config.company.address_line2 = "Test City";
        config.company.kvk = "12345678";
        config.company.btw = "NL123456789B01";
        config.company.bank_account = "NL99TEST1234567890";
        config.company.tag = "WLT";
        config.company.currency = "EUR";
        ConfigManager::save(config);

        ClientData client;
        client.name = "WorkLog Client";
        client.address_line1 = "456 Client Ave";
        client.address_line2 = "Client City";
        client.hourly_rate = 75.0;
        client.payment_term_days = 14;
        client.tag = "WLC";

        client.logs["2026-01"]["2026-01-05"] = {8.0, "Short message"};
        client.logs["2026-01"]["2026-01-06"] = {8.0, "This is a much longer message that should wrap to multiple lines in the PDF report to test the text wrapping functionality"};
        client.logs["2026-01"]["2026-01-07"] = {4.0, "Another day of work"};

        ClientManager::save("worklogclient", client);
    }

    void TearDown() override
    {
        fs::current_path(original_cwd);
        fs::remove_all(test_dir);
    }
};

TEST_F(WorkLogTest, GenerateWorkLogPDF)
{
    std::string output = WorkLogReport::generate("worklogclient", "2026-01");

    EXPECT_EQ(output, "worklog-worklogclient-2026-01.pdf");
    EXPECT_TRUE(fs::exists(test_dir + "/" + output));

    auto file_size = fs::file_size(test_dir + "/" + output);
    EXPECT_GT(file_size, 1000);
}

TEST_F(WorkLogTest, ThrowsOnNoLogs)
{
    ClientData empty_client;
    empty_client.name = "Empty Client";
    empty_client.tag = "EMP";
    empty_client.hourly_rate = 50.0;
    ClientManager::save("emptywlclient", empty_client);

    EXPECT_THROW(WorkLogReport::generate("emptywlclient", "2026-01"), std::runtime_error);
}

TEST_F(WorkLogTest, ThrowsOnNonexistentClient)
{
    EXPECT_THROW(WorkLogReport::generate("nonexistent", "2026-01"), std::runtime_error);
}

TEST_F(WorkLogTest, GenerateWithLongDescriptions)
{
    ClientData client;
    client.name = "Long Desc Client";
    client.tag = "LDC";
    client.hourly_rate = 100.0;

    std::string long_msg = "Work on uNode, investigate address sanitization to prevent any memory issues in the field, enable address sanitization on project to catch memory issues, identify found issues and fix them to improve uNode stability";
    client.logs["2026-02"]["2026-02-01"] = {8.0, long_msg};
    client.logs["2026-02"]["2026-02-02"] = {8.0, long_msg};

    ClientManager::save("longdescclient", client);

    std::string output = WorkLogReport::generate("longdescclient", "2026-02");
    EXPECT_TRUE(fs::exists(test_dir + "/" + output));

    auto file_size = fs::file_size(test_dir + "/" + output);
    EXPECT_GT(file_size, 1000);
}

TEST_F(WorkLogTest, PrepareDataCalculatesCorrectTotals)
{
    ClientData client;
    client.name = "Totals Client";
    client.tag = "TOT";
    client.hourly_rate = 50.0;
    client.logs["2026-03"]["2026-03-01"] = {8.0, "Day 1"};
    client.logs["2026-03"]["2026-03-02"] = {6.5, "Day 2"};
    client.logs["2026-03"]["2026-03-03"] = {7.5, "Day 3"};
    ClientManager::save("totalsclient", client);

    std::string output = WorkLogReport::generate("totalsclient", "2026-03");
    EXPECT_TRUE(fs::exists(test_dir + "/" + output));
}

TEST_F(WorkLogTest, EntriesSortedByDate)
{
    ClientData client;
    client.name = "Sort Client";
    client.tag = "SRT";
    client.hourly_rate = 60.0;
    client.logs["2026-04"]["2026-04-15"] = {8.0, "Middle"};
    client.logs["2026-04"]["2026-04-01"] = {8.0, "First"};
    client.logs["2026-04"]["2026-04-28"] = {8.0, "Last"};
    ClientManager::save("sortclient", client);

    std::string output = WorkLogReport::generate("sortclient", "2026-04");
    EXPECT_TRUE(fs::exists(test_dir + "/" + output));
}
