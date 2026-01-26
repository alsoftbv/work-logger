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

static void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *)
{
    throw std::runtime_error("PDF error: " + std::to_string(error_no) + ", detail: " + std::to_string(detail_no));
}

PDFBuilder::PDFBuilder(const InvoiceData &data) : data_(data)
{
    pdf_ = HPDF_New(error_handler, nullptr);
    if (!pdf_)
        throw std::runtime_error("Could not create PDF document");

    HPDF_SetCompressionMode(pdf_, HPDF_COMP_ALL);
    page_ = HPDF_AddPage(pdf_);
    HPDF_Page_SetSize(page_, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    font_ = HPDF_GetFont(pdf_, "Helvetica", "WinAnsiEncoding");
    font_bold_ = HPDF_GetFont(pdf_, "Helvetica-Bold", "WinAnsiEncoding");
}

PDFBuilder::~PDFBuilder()
{
    if (pdf_)
        HPDF_Free(pdf_);
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

void PDFBuilder::save(const std::string &path)
{
    HPDF_SaveToFile(pdf_, path.c_str());
}

void PDFBuilder::text(float x, float y, const std::string &s)
{
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, x, y, s.c_str());
    HPDF_Page_EndText(page_);
}

void PDFBuilder::set_font(bool bold, float size)
{
    HPDF_Page_SetFontAndSize(page_, bold ? font_bold_ : font_, size);
}

void PDFBuilder::set_color(float gray)
{
    HPDF_Page_SetRGBFill(page_, gray, gray, gray);
}

void PDFBuilder::set_color(float r, float g, float b)
{
    HPDF_Page_SetRGBFill(page_, r, g, b);
}

void PDFBuilder::draw_header()
{
    draw_logo();
    draw_invoice_title();
}

void PDFBuilder::draw_logo()
{
    if (data_.company_logo.empty() || !std::filesystem::exists(data_.company_logo))
        return;

    std::string ext = std::filesystem::path(data_.company_logo).extension().string();
    HPDF_Image logo = nullptr;

    HPDF_SetErrorHandler(pdf_, nullptr);
    if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG" || ext == ".JPEG")
        logo = HPDF_LoadJpegImageFromFile(pdf_, data_.company_logo.c_str());
    HPDF_SetErrorHandler(pdf_, error_handler);
    HPDF_ResetError(pdf_);

    if (logo)
    {
        float w = HPDF_Image_GetWidth(logo);
        float h = HPDF_Image_GetHeight(logo);
        float scale = std::min(150.0f / w, 80.0f / h);
        float logo_top = PAGE_HEIGHT - MARGIN + 20.0f;
        HPDF_Page_DrawImage(page_, logo, MARGIN, logo_top - h * scale, w * scale, h * scale);
    }
    else
    {
        std::cerr << "Warning: Could not load logo. Only JPEG format is supported." << std::endl;
    }
}

void PDFBuilder::draw_invoice_title()
{
    float y = PAGE_HEIGHT - MARGIN;
    set_font(true, 28);
    text(PAGE_WIDTH - MARGIN - 100, y, "INVOICE");

    set_font(false, 10);
    set_color(0.5f);
    text(PAGE_WIDTH - MARGIN - 85, y - 22, "# " + data_.invoice_number);
}

void PDFBuilder::draw_company_info()
{
    float y = PAGE_HEIGHT - MARGIN - 100;

    set_color(0);
    set_font(true, 11);
    text(MARGIN, y, data_.company_name);

    set_font(false, 10);
    text(MARGIN, y - 15, data_.company_address1);
    text(MARGIN, y - 29, data_.company_address2);
}

void PDFBuilder::draw_date_info()
{
    float lx = 360, vx = 480;
    float y = PAGE_HEIGHT - MARGIN - 100;

    auto row = [&](const std::string &label, const std::string &value) {
        set_color(0.4f);
        text(lx, y, label);
        set_color(0);
        text(vx, y, value);
        y -= 18;
    };

    set_font(false, 10);
    row("Date:", format_date(data_.date));
    row("Payment Terms:", std::to_string(data_.payment_term_days) + " Days");
    row("Due Date:", format_date(data_.due_date));

    draw_balance_due_box();
}

void PDFBuilder::draw_balance_due_box()
{
    float x = 360, vx = 480;
    float y = PAGE_HEIGHT - MARGIN - 100 - 54 - 7;

    set_color(0.98f, 0.85f, 0.5f);
    draw_rounded_rect(x - 10, y - 9, PAGE_WIDTH - MARGIN - x + 10, 30, 5);

    set_color(0);
    set_font(true, 12);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, x, y, "Balance Due:");
    HPDF_Page_TextOut(page_, vx, y, format_currency(data_.total).c_str());
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_billed_to()
{
    float y = PAGE_HEIGHT - MARGIN - 169;

    set_font(false, 9);
    set_color(0.5f);
    text(MARGIN, y, "Billed To:");

    set_color(0);
    set_font(true, 10);
    text(MARGIN, y - 15, data_.client_name);

    set_font(false, 10);
    text(MARGIN, y - 29, data_.client_address1);
    text(MARGIN, y - 43, data_.client_address2);
}

float PDFBuilder::draw_table()
{
    float y = PAGE_HEIGHT - MARGIN - 252;
    draw_table_header(y);
    draw_table_row(y - 28);
    return draw_totals(y - 73);
}

void PDFBuilder::draw_table_header(float y)
{
    set_color(0.95f, 0.6f, 0.1f);
    draw_rounded_rect(MARGIN, y - 8, PAGE_WIDTH - 2 * MARGIN, 28, 5);

    set_color(1, 1, 1);
    set_font(true, 10);
    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, MARGIN + 15, y + 2, "Item");
    HPDF_Page_TextOut(page_, 280, y + 2, "Quantity");
    HPDF_Page_TextOut(page_, 380, y + 2, "Rate");
    HPDF_Page_TextOut(page_, 480, y + 2, "Amount");
    HPDF_Page_EndText(page_);
}

void PDFBuilder::draw_table_row(float y)
{
    set_color(0);
    set_font(true, 10);
    text(MARGIN + 15, y, "Hours");

    set_font(false, 10);
    std::ostringstream hours;
    hours << std::fixed << std::setprecision(0) << data_.total_hours;

    HPDF_Page_BeginText(page_);
    HPDF_Page_TextOut(page_, 280, y, hours.str().c_str());
    HPDF_Page_TextOut(page_, 380, y, format_currency(data_.hourly_rate).c_str());
    HPDF_Page_TextOut(page_, 480, y, format_currency(data_.subtotal).c_str());
    HPDF_Page_EndText(page_);
}

float PDFBuilder::draw_totals(float y)
{
    set_font(false, 10);

    auto row = [&](const std::string &label, double amount, bool bold = false) {
        if (bold) set_font(true, 10);
        HPDF_Page_BeginText(page_);
        HPDF_Page_TextOut(page_, 380, y, label.c_str());
        HPDF_Page_TextOut(page_, 480, y, format_currency(amount).c_str());
        HPDF_Page_EndText(page_);
        y -= 18;
    };

    row("Subtotal:", data_.subtotal);
    row("VAT (21%):", data_.vat);
    row("Total:", data_.total, true);

    return y + 18;
}

void PDFBuilder::draw_footer(float y)
{
    set_font(false, 9);
    set_color(0.5f);
    text(MARGIN, y, "Details:");

    set_color(0);
    set_font(false, 10);
    text(MARGIN, y - 15, "KvK: " + data_.company_kvk);
    text(MARGIN, y - 29, "BTW: " + data_.company_btw);
    text(MARGIN, y - 43, "Bank Account: " + data_.company_bank);

    set_font(false, 9);
    set_color(0.5f);
    text(MARGIN, y - 68, "Terms:");

    set_color(0);
    set_font(false, 10);
    std::ostringstream terms;
    terms << "Please pay the total amount within " << data_.payment_term_days
          << " days to the IBAN bank account number, stating the invoice number.";
    text(MARGIN, y - 83, terms.str());
}

void PDFBuilder::draw_rounded_rect(float x, float y, float w, float h, float r)
{
    HPDF_Page_MoveTo(page_, x + r, y);
    HPDF_Page_LineTo(page_, x + w - r, y);
    HPDF_Page_CurveTo(page_, x + w, y, x + w, y, x + w, y + r);
    HPDF_Page_LineTo(page_, x + w, y + h - r);
    HPDF_Page_CurveTo(page_, x + w, y + h, x + w, y + h, x + w - r, y + h);
    HPDF_Page_LineTo(page_, x + r, y + h);
    HPDF_Page_CurveTo(page_, x, y + h, x, y + h, x, y + h - r);
    HPDF_Page_LineTo(page_, x, y + r);
    HPDF_Page_CurveTo(page_, x, y, x, y, x + r, y);
    HPDF_Page_Fill(page_);
}

std::string PDFBuilder::format_currency(double amount)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (data_.currency == "EUR")
        oss << "\x80 " << amount;
    else if (data_.currency == "USD")
        oss << "$ " << amount;
    else if (data_.currency == "GBP")
        oss << "\xA3 " << amount;
    else
        oss << data_.currency << " " << amount;

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

InvoiceData InvoiceGenerator::prepare_data(const std::string &client_id, const std::string &month)
{
    AppConfig config = ConfigManager::load();
    ClientData client = ClientManager::load(client_id);

    std::string month_key = month.empty() ? ClientManager::get_previous_month_key() : month;
    double total_hours = ClientManager::get_month_total_hours(client, month_key);

    if (total_hours <= 0)
        throw std::runtime_error("No hours logged for " + month_key);

    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);

    std::ostringstream date_ss;
    date_ss << std::put_time(&tm, "%Y-%m-%d");

    tm.tm_mday += client.payment_term_days;
    std::mktime(&tm);

    std::ostringstream due_ss;
    due_ss << std::put_time(&tm, "%Y-%m-%d");

    double subtotal = total_hours * client.hourly_rate;
    double vat = subtotal * VAT_RATE;

    InvoiceData data;
    data.invoice_number = config.company.tag + "-" + client.tag + "-" + month_key;
    data.date = date_ss.str();
    data.due_date = due_ss.str();
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
    data.total = subtotal + vat;

    return data;
}

std::string InvoiceGenerator::generate(const std::string &client_id, const std::string &month)
{
    if (!ClientManager::client_exists(client_id))
        throw std::runtime_error("Client not found: " + client_id);

    InvoiceData data = prepare_data(client_id, month);
    std::string output_path = data.invoice_number + ".pdf";

    PDFBuilder builder(data);
    builder.build();
    builder.save(output_path);

    return output_path;
}
