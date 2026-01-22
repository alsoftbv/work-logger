#include "report/work_log.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <stdexcept>

static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    throw std::runtime_error("PDF error: " + std::to_string(error_no) + ", detail: " + std::to_string(detail_no));
}

WorkLogPDFBuilder::WorkLogPDFBuilder(const WorkLogReportData &data) : data_(data)
{
    pdf_ = HPDF_New(error_handler, nullptr);
    if (!pdf_)
    {
        throw std::runtime_error("Could not create PDF document");
    }

    HPDF_SetCompressionMode(pdf_, HPDF_COMP_ALL);

    page_ = HPDF_AddPage(pdf_);
    HPDF_Page_SetSize(page_, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    font_ = HPDF_GetFont(pdf_, "Helvetica", "WinAnsiEncoding");
    font_bold_ = HPDF_GetFont(pdf_, "Helvetica-Bold", "WinAnsiEncoding");
}

WorkLogPDFBuilder::~WorkLogPDFBuilder()
{
    if (pdf_)
    {
        HPDF_Free(pdf_);
    }
}

void WorkLogPDFBuilder::build()
{
    draw_header();
    float y = PAGE_HEIGHT - MARGIN - 90;
    draw_table_header(y);
    float end_y = draw_table_rows(y - 28);
    draw_summary(end_y - 20);
}

void WorkLogPDFBuilder::save(const std::string &output_path)
{
    HPDF_SaveToFile(pdf_, output_path.c_str());
}

void WorkLogPDFBuilder::draw_header()
{
    float y = PAGE_HEIGHT - MARGIN;

    HPDF_Page_SetFontAndSize(page_, font_bold_, 24);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, "Work Log Report");
    HPDF_Page_EndText(page_);

    y -= 35;
    HPDF_Page_SetFontAndSize(page_, font_, 12);
    HPDF_Page_SetRGBFill(page_, 0.3f, 0.3f, 0.3f);
    HPDF_Page_BeginText(page_);
    std::string client_line = "Client: " + data_.client_name;
    HPDF_Page_TextOut(page_, MARGIN, y, client_line.c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_BeginText(page_);
    std::string month_line = "Period: " + data_.month;
    HPDF_Page_TextOut(page_, MARGIN, y, month_line.c_str());
    HPDF_Page_EndText(page_);

    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
}

void WorkLogPDFBuilder::draw_table_header(float y)
{
    float col1 = MARGIN;
    float col2 = 180;
    float col3 = 250;

    HPDF_Page_SetRGBFill(page_, 0.95f, 0.6f, 0.1f);
    draw_rounded_rect(MARGIN, y - 8, PAGE_WIDTH - 2 * MARGIN, 28, 5);

    HPDF_Page_SetRGBFill(page_, 1, 1, 1);
    HPDF_Page_SetFontAndSize(page_, font_bold_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col1 + 15, y + 2, "Date");
    HPDF_Page_TextOut(page_, col2, y + 2, "Hours");
    HPDF_Page_TextOut(page_, col3, y + 2, "Description");
    HPDF_Page_EndText(page_);
}

float WorkLogPDFBuilder::add_new_page()
{
    page_ = HPDF_AddPage(pdf_);
    HPDF_Page_SetSize(page_, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    return PAGE_HEIGHT - MARGIN;
}

float WorkLogPDFBuilder::draw_table_rows(float y)
{
    float col1 = MARGIN;
    float col2 = 180;
    float col3 = 250;
    float desc_max_width = PAGE_WIDTH - MARGIN - col3;
    float line_height = 14;
    float row_padding = 16;
    float min_y = MARGIN + 20;

    HPDF_Page_SetFontAndSize(page_, font_, 10);

    bool alternate = false;
    for (const auto &entry : data_.entries)
    {
        std::vector<std::string> lines = wrap_text(entry.message, desc_max_width);
        int num_lines = std::max(1, static_cast<int>(lines.size()));
        float row_height = num_lines * line_height + row_padding;

        if (y - row_height < min_y)
        {
            y = add_new_page();
            draw_table_header(y);
            y -= 28;
        }

        if (alternate)
        {
            HPDF_Page_SetRGBFill(page_, 0.95f, 0.95f, 0.95f);
            draw_rounded_rect(MARGIN, y - row_height + line_height + 6, PAGE_WIDTH - 2 * MARGIN, row_height, 4);
        }
        alternate = !alternate;

        HPDF_Page_SetRGBFill(page_, 0, 0, 0);
        HPDF_Page_SetFontAndSize(page_, font_, 10);
        HPDF_Page_BeginText(page_);
        HPDF_Page_TextOut(page_, col1 + 15, y, format_date(entry.date).c_str());

        std::ostringstream hours_ss;
        hours_ss << std::fixed << std::setprecision(1) << entry.hours;
        HPDF_Page_TextOut(page_, col2, y, hours_ss.str().c_str());

        float text_y = y;
        for (const auto &line : lines)
        {
            HPDF_Page_TextOut(page_, col3, text_y, line.c_str());
            text_y -= line_height;
        }
        HPDF_Page_EndText(page_);

        y -= row_height;
    }

    return y;
}

void WorkLogPDFBuilder::draw_summary(float y)
{
    float col2 = 380;
    float col3 = 480;
    float summary_height = 100;

    if (y - summary_height < MARGIN)
    {
        y = add_new_page() - 30;
    }

    HPDF_Page_SetRGBFill(page_, 0.95f, 0.95f, 0.95f);
    draw_rounded_rect(col2 - 20, y - 60, PAGE_WIDTH - MARGIN - col2 + 20, 80, 5);

    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col2, y, "Total Hours:");
    std::ostringstream hours_ss;
    hours_ss << std::fixed << std::setprecision(1) << data_.total_hours;
    HPDF_Page_TextOut(page_, col3, y, hours_ss.str().c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col2, y, "Hourly Rate:");
    HPDF_Page_TextOut(page_, col3, y, format_currency(data_.hourly_rate).c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_SetFontAndSize(page_, font_bold_, 11);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col2, y, "Total Amount:");
    HPDF_Page_TextOut(page_, col3, y, format_currency(data_.total_amount).c_str());
    HPDF_Page_EndText(page_);
}

void WorkLogPDFBuilder::draw_rounded_rect(float x, float y, float width, float height, float radius)
{
    HPDF_Page_MoveTo(page_, x + radius, y);
    HPDF_Page_LineTo(page_, x + width - radius, y);
    HPDF_Page_CurveTo(page_, x + width, y, x + width, y, x + width, y + radius);
    HPDF_Page_LineTo(page_, x + width, y + height - radius);
    HPDF_Page_CurveTo(page_, x + width, y + height, x + width, y + height, x + width - radius, y + height);
    HPDF_Page_LineTo(page_, x + radius, y + height);
    HPDF_Page_CurveTo(page_, x, y + height, x, y + height, x, y + height - radius);
    HPDF_Page_LineTo(page_, x, y + radius);
    HPDF_Page_CurveTo(page_, x, y, x, y, x + radius, y);
    HPDF_Page_Fill(page_);
}

std::string WorkLogPDFBuilder::format_currency(double amount)
{
    std::ostringstream oss;
    if (data_.currency == "EUR")
    {
        oss << "\x80 " << std::fixed << std::setprecision(2) << amount;
    }
    else if (data_.currency == "USD")
    {
        oss << "$ " << std::fixed << std::setprecision(2) << amount;
    }
    else if (data_.currency == "GBP")
    {
        oss << "\xA3 " << std::fixed << std::setprecision(2) << amount;
    }
    else
    {
        oss << data_.currency << " " << std::fixed << std::setprecision(2) << amount;
    }
    return oss.str();
}

std::vector<std::string> WorkLogPDFBuilder::wrap_text(const std::string &text, float max_width)
{
    std::vector<std::string> lines;
    std::string current_line;
    std::istringstream words_stream(text);
    std::string word;

    while (words_stream >> word)
    {
        std::string test_line = current_line.empty() ? word : current_line + " " + word;
        float width = HPDF_Page_TextWidth(page_, test_line.c_str());

        if (width <= max_width)
        {
            current_line = test_line;
        }
        else
        {
            if (!current_line.empty())
            {
                lines.push_back(current_line);
            }
            current_line = word;
        }
    }

    if (!current_line.empty())
    {
        lines.push_back(current_line);
    }

    return lines;
}

std::string WorkLogPDFBuilder::format_date(const std::string &date)
{
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    std::ostringstream oss;
    oss << std::put_time(&tm, "%b %d, %Y");
    return oss.str();
}

// WorkLogReport implementation

WorkLogReportData WorkLogReport::prepare_data(const std::string &client_id, const std::string &month)
{
    AppConfig config = ConfigManager::load();
    ClientData client = ClientManager::load(client_id);

    std::string month_key = month.empty() ? ClientManager::get_previous_month_key() : month;

    WorkLogReportData data;
    data.client_name = client.name;
    data.month = month_key;
    data.currency = config.company.currency;
    data.hourly_rate = client.hourly_rate;
    data.total_hours = 0;

    // Get log entries for the month
    if (client.logs.count(month_key))
    {
        std::vector<std::pair<std::string, WorkLog>> sorted_logs;
        for (const auto &[date, log] : client.logs.at(month_key))
        {
            sorted_logs.push_back({date, log});
        }

        std::sort(sorted_logs.begin(), sorted_logs.end(),
                  [](const auto &a, const auto &b)
                  { return a.first < b.first; });

        for (const auto &[date, log] : sorted_logs)
        {
            WorkLogEntry entry;
            entry.date = date;
            entry.hours = log.hours;
            entry.message = log.message;
            data.entries.push_back(entry);
            data.total_hours += log.hours;
        }
    }

    data.total_amount = data.total_hours * data.hourly_rate;

    return data;
}

std::string WorkLogReport::generate(const std::string &client_id, const std::string &month)
{
    if (!ClientManager::client_exists(client_id))
    {
        throw std::runtime_error("Client not found: " + client_id);
    }

    WorkLogReportData data = prepare_data(client_id, month);

    if (data.entries.empty())
    {
        throw std::runtime_error("No work logs found for " + data.month);
    }

    std::string output_path = "worklog-" + client_id + "-" + data.month + ".pdf";

    WorkLogPDFBuilder builder(data);
    builder.build();
    builder.save(output_path);

    return output_path;
}
