#pragma once

#include <string>
#include <vector>
#include <hpdf.h>
#include "storage/config.hpp"
#include "storage/client.hpp"

struct WorkLogEntry
{
    std::string date;
    double hours;
    std::string message;
};

struct WorkLogReportData
{
    std::string client_name;
    std::string month;
    std::string currency;
    double hourly_rate;
    double total_hours;
    double subtotal;
    double vat;
    double total;
    std::vector<WorkLogEntry> entries;
};

class WorkLogPDFBuilder
{
public:
    WorkLogPDFBuilder(const WorkLogReportData &data);
    ~WorkLogPDFBuilder();

    void build();
    void save(const std::string &output_path);

private:
    void draw_header();
    void draw_table_header(float y);
    float draw_table_rows(float y);
    void draw_summary(float y);

    void draw_rounded_rect(float x, float y, float width, float height, float radius);
    std::string format_currency(double amount);
    static std::string format_date(const std::string &date);
    std::vector<std::string> wrap_text(const std::string &text, float max_width);
    float add_new_page();

    const WorkLogReportData &data_;
    HPDF_Doc pdf_;
    HPDF_Page page_;
    HPDF_Font font_;
    HPDF_Font font_bold_;

    static constexpr float PAGE_WIDTH = 595.0f;
    static constexpr float PAGE_HEIGHT = 842.0f;
    static constexpr float MARGIN = 50.0f;
};

class WorkLogReport
{
public:
    static std::string generate(const std::string &client_id, const std::string &month = "");

private:
    static WorkLogReportData prepare_data(const std::string &client_id, const std::string &month);
};
