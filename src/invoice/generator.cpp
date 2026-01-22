#include "invoice/generator.hpp"
#include "storage/config.hpp"
#include "storage/client.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <filesystem>
#include <iostream>

static const float VAT_RATE = 0.21f;

static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    throw std::runtime_error("PDF error: " + std::to_string(error_no) + ", detail: " + std::to_string(detail_no));
}

// PDFBuilder implementation

PDFBuilder::PDFBuilder(const InvoiceData &data) : data_(data)
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

PDFBuilder::~PDFBuilder()
{
    if (pdf_)
    {
        HPDF_Free(pdf_);
    }
}

void PDFBuilder::build()
{
    draw_header();
    draw_company_info();
    draw_date_info();
    draw_billed_to();
    float table_end_y = draw_table();
    draw_footer(table_end_y - 40);
}

void PDFBuilder::save(const std::string &output_path)
{
    HPDF_SaveToFile(pdf_, output_path.c_str());
}

void PDFBuilder::draw_header()
{
    draw_logo();
    draw_invoice_title();
}

void PDFBuilder::draw_logo()
{
    if (data_.company_logo.empty() || !std::filesystem::exists(data_.company_logo))
    {
        return;
    }

    std::string ext = std::filesystem::path(data_.company_logo).extension().string();
    HPDF_Image logo = nullptr;

    // Temporarily disable error handler for logo loading
    HPDF_SetErrorHandler(pdf_, nullptr);

    if (ext == ".png" || ext == ".PNG")
    {
        logo = HPDF_LoadPngImageFromFile(pdf_, data_.company_logo.c_str());
    }
    else if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG")
    {
        logo = HPDF_LoadJpegImageFromFile(pdf_, data_.company_logo.c_str());
    }

    // Restore error handler
    HPDF_SetErrorHandler(pdf_, error_handler);
    HPDF_ResetError(pdf_);

    if (logo)
    {
        float logo_width = HPDF_Image_GetWidth(logo);
        float logo_height = HPDF_Image_GetHeight(logo);
        float max_logo_height = 80.0f;
        float max_logo_width = 150.0f;

        float scale = std::min(max_logo_width / logo_width, max_logo_height / logo_height);
        float scaled_width = logo_width * scale;
        float scaled_height = logo_height * scale;

        // Align logo top with INVOICE text top (baseline + cap height)
        float text_baseline = PAGE_HEIGHT - MARGIN;
        float cap_height = 20.0f;  // Approximate cap height for 28pt Helvetica
        float logo_top = text_baseline + cap_height;
        HPDF_Page_DrawImage(page_, logo, MARGIN, logo_top - scaled_height, scaled_width, scaled_height);
    }
    else
    {
        std::cerr << "Warning: Could not load logo (unsupported format). Try converting to JPEG.\n";
    }
}

void PDFBuilder::draw_invoice_title()
{
    float y = PAGE_HEIGHT - MARGIN;

    HPDF_Page_SetFontAndSize(page_, font_bold_, 28);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, PAGE_WIDTH - MARGIN - 100, y, "INVOICE");
    HPDF_Page_EndText(page_);

    y -= 22;
    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_SetRGBFill(page_, 0.5f, 0.5f, 0.5f);
    HPDF_Page_BeginText(page_);
    std::string inv_label = "# " + data_.invoice_number;
    HPDF_Page_TextOut(page_, PAGE_WIDTH - MARGIN - 85, y, inv_label.c_str());
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_company_info()
{
    float y = PAGE_HEIGHT - MARGIN - 100;

    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_bold_, 11);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, data_.company_name.c_str());
    HPDF_Page_EndText(page_);

    y -= 15;
    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, data_.company_address1.c_str());
    HPDF_Page_EndText(page_);

    y -= 14;
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, data_.company_address2.c_str());
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_date_info()
{
    float info_x = 360;
    float info_val_x = 480;
    float y = PAGE_HEIGHT - MARGIN - 100;

    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_SetRGBFill(page_, 0.4f, 0.4f, 0.4f);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_x, y, "Date:");
    HPDF_Page_EndText(page_);
    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_val_x, y, format_date(data_.date).c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_SetRGBFill(page_, 0.4f, 0.4f, 0.4f);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_x, y, "Payment Terms:");
    HPDF_Page_EndText(page_);
    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    std::string terms = std::to_string(data_.payment_term_days) + " Days";
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_val_x, y, terms.c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_SetRGBFill(page_, 0.4f, 0.4f, 0.4f);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_x, y, "Due Date:");
    HPDF_Page_EndText(page_);
    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_val_x, y, format_date(data_.due_date).c_str());
    HPDF_Page_EndText(page_);

    draw_balance_due_box();
}

void PDFBuilder::draw_balance_due_box()
{
    float info_x = 360;
    float info_val_x = 480;
    float y = PAGE_HEIGHT - MARGIN - 100 - 18 - 18 - 25;

    HPDF_Page_SetRGBFill(page_, 0.98f, 0.85f, 0.5f);
    draw_rounded_rect(info_x - 10, y - 9, PAGE_WIDTH - MARGIN - info_x + 10, 30, 5);

    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_bold_, 12);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, info_x, y, "Balance Due:");
    std::string balance = format_currency(data_.total);
    HPDF_Page_TextOut(page_, info_val_x, y, balance.c_str());
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_billed_to()
{
    float y = PAGE_HEIGHT - MARGIN - 100 - 14 - 55;

    HPDF_Page_SetFontAndSize(page_, font_, 9);
    HPDF_Page_SetRGBFill(page_, 0.5f, 0.5f, 0.5f);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, "Billed To:");
    HPDF_Page_EndText(page_);

    y -= 15;
    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_bold_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, data_.client_name.c_str());
    HPDF_Page_EndText(page_);

    y -= 14;
    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, data_.client_address1.c_str());
    HPDF_Page_EndText(page_);

    y -= 14;
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, data_.client_address2.c_str());
    HPDF_Page_EndText(page_);
}

float PDFBuilder::draw_table()
{
    float y = PAGE_HEIGHT - MARGIN - 100 - 14 - 55 - 15 - 14 - 14 - 40;

    draw_table_header(y);
    draw_table_row(y - 28);
    return draw_totals(y - 28 - 45);
}

void PDFBuilder::draw_table_header(float y)
{
    float col1 = MARGIN;
    float col2 = 280;
    float col3 = 380;
    float col4 = 480;

    HPDF_Page_SetRGBFill(page_, 0.95f, 0.6f, 0.1f);
    draw_rounded_rect(MARGIN, y - 8, PAGE_WIDTH - 2 * MARGIN, 28, 5);

    HPDF_Page_SetRGBFill(page_, 1, 1, 1);
    HPDF_Page_SetFontAndSize(page_, font_bold_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col1 + 15, y + 2, "Item");
    HPDF_Page_TextOut(page_, col2, y + 2, "Quantity");
    HPDF_Page_TextOut(page_, col3, y + 2, "Rate");
    HPDF_Page_TextOut(page_, col4, y + 2, "Amount");
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_table_row(float y)
{
    float col1 = MARGIN;
    float col2 = 280;
    float col3 = 380;
    float col4 = 480;

    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_bold_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col1 + 15, y, "Hours");
    HPDF_Page_EndText(page_);

    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    std::ostringstream hours_ss;
    hours_ss << std::fixed << std::setprecision(0) << data_.total_hours;
    HPDF_Page_TextOut(page_, col2, y, hours_ss.str().c_str());
    HPDF_Page_TextOut(page_, col3, y, format_currency(data_.hourly_rate).c_str());
    HPDF_Page_TextOut(page_, col4, y, format_currency(data_.subtotal).c_str());
    HPDF_Page_EndText(page_);
}

float PDFBuilder::draw_totals(float y)
{
    float col3 = 380;
    float col4 = 480;

    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col3, y, "Subtotal:");
    HPDF_Page_TextOut(page_, col4, y, format_currency(data_.subtotal).c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col3, y, "VAT (21%):");
    HPDF_Page_TextOut(page_, col4, y, format_currency(data_.vat).c_str());
    HPDF_Page_EndText(page_);

    y -= 18;
    HPDF_Page_SetFontAndSize(page_, font_bold_, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, col3, y, "Total:");
    HPDF_Page_TextOut(page_, col4, y, format_currency(data_.total).c_str());
    HPDF_Page_EndText(page_);

    return y;
}

void PDFBuilder::draw_footer(float y)
{
    HPDF_Page_SetFontAndSize(page_, font_, 9);
    HPDF_Page_SetRGBFill(page_, 0.5f, 0.5f, 0.5f);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, "Details:");
    HPDF_Page_EndText(page_);

    y -= 15;
    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    std::string kvk_line = "KvK: " + data_.company_kvk;
    HPDF_Page_TextOut(page_, MARGIN, y, kvk_line.c_str());
    HPDF_Page_EndText(page_);

    y -= 14;
    HPDF_Page_BeginText(page_);
    std::string btw_line = "BTW: " + data_.company_btw;
    HPDF_Page_TextOut(page_, MARGIN, y, btw_line.c_str());
    HPDF_Page_EndText(page_);

    y -= 14;
    HPDF_Page_BeginText(page_);
    std::string bank_line = "Bank Account: " + data_.company_bank;
    HPDF_Page_TextOut(page_, MARGIN, y, bank_line.c_str());
    HPDF_Page_EndText(page_);

    y -= 25;
    HPDF_Page_SetFontAndSize(page_, font_, 9);
    HPDF_Page_SetRGBFill(page_, 0.5f, 0.5f, 0.5f);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN, y, "Terms:");
    HPDF_Page_EndText(page_);

    y -= 15;
    HPDF_Page_SetRGBFill(page_, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page_, font_, 10);
    HPDF_Page_BeginText(page_);
    std::ostringstream terms_ss;
    terms_ss << "Please pay the total amount within " << data_.payment_term_days
             << " days to the IBAN bank account number, stating the invoice number.";
    HPDF_Page_TextOut(page_, MARGIN, y, terms_ss.str().c_str());
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_rounded_rect(float x, float y, float width, float height, float radius)
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

std::string PDFBuilder::format_currency(double amount)
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

std::string PDFBuilder::format_date(const std::string &date)
{
    std::tm tm = {};
    std::istringstream ss(date);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    std::ostringstream oss;
    oss << std::put_time(&tm, "%b %d, %Y");
    return oss.str();
}

// InvoiceGenerator implementation

InvoiceData InvoiceGenerator::prepare_data(const std::string &client_id, const std::string &month)
{
    AppConfig config = ConfigManager::load();
    ClientData client = ClientManager::load(client_id);

    std::string month_key = month.empty() ? ClientManager::get_previous_month_key() : month;
    double total_hours = ClientManager::get_month_total_hours(client, month_key);

    if (total_hours <= 0)
    {
        throw std::runtime_error("No hours logged for " + month_key);
    }

    // Invoice number: TAG-CLIENT_TAG-YEAR-MONTH
    std::ostringstream inv_num_ss;
    inv_num_ss << config.company.tag << "-" << client.tag << "-" << month_key;

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);

    std::ostringstream date_ss;
    date_ss << std::put_time(&tm, "%Y-%m-%d");
    std::string today = date_ss.str();

    tm.tm_mday += client.payment_term_days;
    std::mktime(&tm);

    std::ostringstream due_ss;
    due_ss << std::put_time(&tm, "%Y-%m-%d");
    std::string due_date = due_ss.str();

    double subtotal = total_hours * client.hourly_rate;
    double vat = subtotal * VAT_RATE;
    double total = subtotal + vat;

    InvoiceData data;
    data.invoice_number = inv_num_ss.str();
    data.date = today;
    data.due_date = due_date;
    data.payment_term_days = client.payment_term_days;

    data.company_name = config.company.name;
    data.company_address1 = config.company.address_line1;
    data.company_address2 = config.company.address_line2;
    data.company_kvk = config.company.kvk;
    data.company_btw = config.company.btw;
    data.company_bank = config.company.bank_account;
    data.company_logo = config.company.logo_path;
    data.currency = config.company.currency;

    data.client_name = client.name;
    data.client_address1 = client.address_line1;
    data.client_address2 = client.address_line2;

    data.total_hours = total_hours;
    data.hourly_rate = client.hourly_rate;
    data.subtotal = subtotal;
    data.vat = vat;
    data.total = total;

    return data;
}

std::string InvoiceGenerator::generate(const std::string &client_id, const std::string &month)
{
    if (!ClientManager::client_exists(client_id))
    {
        throw std::runtime_error("Client not found: " + client_id);
    }

    InvoiceData data = prepare_data(client_id, month);
    std::string output_path = data.invoice_number + ".pdf";

    PDFBuilder builder(data);
    builder.build();
    builder.save(output_path);

    return output_path;
}
