#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "storage/config.hpp"

namespace fs = std::filesystem;

class ConfigTest : public ::testing::Test
{
protected:
    std::string test_dir;

    void SetUp() override
    {
        test_dir = fs::temp_directory_path() / "wlog_test";
        fs::create_directories(test_dir);

        // Set HOME to test directory for isolation
        setenv("HOME", test_dir.c_str(), 1);
    }

    void TearDown() override
    {
        fs::remove_all(test_dir);
    }
};

TEST_F(ConfigTest, GetConfigDir)
{
    std::string config_dir = ConfigManager::get_config_dir();
    EXPECT_EQ(config_dir, test_dir + "/.wlog");
}

TEST_F(ConfigTest, ConfigNotExistsInitially)
{
    EXPECT_FALSE(ConfigManager::config_exists());
}

TEST_F(ConfigTest, SaveAndLoadConfig)
{
    AppConfig config;
    config.company.name = "Test Company";
    config.company.address_line1 = "123 Test Street";
    config.company.address_line2 = "Test City";
    config.company.kvk = "12345678";
    config.company.btw = "NL123456789B01";
    config.company.bank_account = "NL00TEST0123456789";
    config.company.tag = "TST";
    config.company.currency = "EUR";

    ConfigManager::save(config);

    EXPECT_TRUE(ConfigManager::config_exists());

    AppConfig loaded = ConfigManager::load();
    EXPECT_EQ(loaded.company.name, "Test Company");
    EXPECT_EQ(loaded.company.address_line1, "123 Test Street");
    EXPECT_EQ(loaded.company.kvk, "12345678");
    EXPECT_EQ(loaded.company.tag, "TST");
    EXPECT_EQ(loaded.company.currency, "EUR");
}

TEST_F(ConfigTest, EnsureDirectoriesCreated)
{
    ConfigManager::ensure_directories();

    EXPECT_TRUE(fs::exists(ConfigManager::get_config_dir()));
    EXPECT_TRUE(fs::exists(ConfigManager::get_clients_dir()));
    EXPECT_TRUE(fs::exists(ConfigManager::get_logos_dir()));
}

TEST_F(ConfigTest, LoadReturnsEmptyIfNoConfig)
{
    AppConfig config = ConfigManager::load();
    EXPECT_TRUE(config.company.name.empty());
    EXPECT_TRUE(config.company.tag.empty());
}
