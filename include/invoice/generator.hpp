#pragma once

#include <string>
#include <hpdf.h>
#include "storage/config.hpp"
#include "storage/client.hpp"

struct InvoiceData
{
    std::string invoice_number;
    std::string date;
    std::string due_date;
    int payment_term_days;

    std::string company_name;
    std::string company_address1;
    std::string company_address2;
    std::string company_kvk;
    std::string company_btw;
    std::string company_bank;
    std::string company_logo;
    std::string currency;

    std::string client_name;
    std::string client_address1;
    std::string client_address2;

    double total_hours;
    double hourly_rate;
    double subtotal;
    double vat;
    double total;
};

class PDFBuilder
{
public:
    PDFBuilder(const InvoiceData &data);
    ~PDFBuilder();

    void build();
    void save(const std::string &output_path);

private:
    void draw_header();
    void draw_logo();
    void draw_invoice_title();
    void draw_company_info();
    void draw_date_info();
    void draw_balance_due_box();
    void draw_billed_to();
    float draw_table();
    void draw_table_header(float y);
    void draw_table_row(float y);
    float draw_totals(float y);
    void draw_footer(float y);

    void draw_rounded_rect(float x, float y, float width, float height, float radius);
    std::string format_currency(double amount);
    static std::string format_date(const std::string &date);

    const InvoiceData &data_;
    HPDF_Doc pdf_;
    HPDF_Page page_;
    HPDF_Font font_;
    HPDF_Font font_bold_;

    static constexpr float PAGE_WIDTH = 595.0f;
    static constexpr float PAGE_HEIGHT = 842.0f;
    static constexpr float MARGIN = 50.0f;
};

class InvoiceGenerator
{
public:
    static std::string generate(const std::string &client_id, const std::string &month = "");

private:
    static InvoiceData prepare_data(const std::string &client_id, const std::string &month);
};
