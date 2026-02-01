#pragma once

namespace Billing
{
    constexpr double VAT_RATE = 0.21;

    struct AmountBreakdown
    {
        double subtotal;
        double vat;
        double total;
    };

    constexpr inline AmountBreakdown calculate_amounts(double hours, double hourly_rate)
    {
        double subtotal = hours * hourly_rate;
        double vat = subtotal * VAT_RATE;
        return {
            .subtotal = subtotal,
            .vat = vat,
            .total = subtotal + vat};
    }
}
