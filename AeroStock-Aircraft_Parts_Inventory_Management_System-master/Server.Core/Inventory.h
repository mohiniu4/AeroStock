#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

#include "PartRecord.h"

namespace AeroStock::Server
{
    // inventory store with search, lookup, and stock update operations
    class Inventory final
    {
    public:
        Inventory();

        [[nodiscard]] std::optional<AeroStock::Common::PartRecord>
            FindByPartNumber(std::string_view partNumber) const;

        [[nodiscard]] std::vector<AeroStock::Common::PartRecord>
            FindByPartName(std::string_view partName) const;

        [[nodiscard]] std::optional<AeroStock::Common::PartRecord>
            GetPartDetails(std::string_view partNumber) const;

        [[nodiscard]] bool UpdateStock(
            std::string_view partNumber,
            std::uint32_t newQuantity);

        [[nodiscard]] const std::vector<AeroStock::Common::PartRecord>&
            GetAllRecords() const noexcept;

    private:
        void LoadSampleData();

        std::vector<AeroStock::Common::PartRecord> records_;
    };
}