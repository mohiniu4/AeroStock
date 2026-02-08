#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace AeroStock::Common
{
    // Represents one inventory item shared between the client and server.
    struct PartRecord final
    {
        std::string partNumber{};      // Unique part identifier.
        std::string partName{};        // Human-readable part name.
        std::string category{};        // Inventory grouping/category.
        std::string storageLocation{}; // Storage location for the part.
        std::string supplier{};        // Supplier name.
        std::uint32_t stockQuantity{ 0U }; // Current available quantity.

        // Returns true if the part number is present.
        [[nodiscard]] bool HasValidPartNumber() const noexcept
        {
            return !partNumber.empty();
        }

        // Returns true if the part name is present.
        [[nodiscard]] bool HasValidPartName() const noexcept
        {
            return !partName.empty();
        }

        // Returns true if the category is present.
        [[nodiscard]] bool HasValidCategory() const noexcept
        {
            return !category.empty();
        }

        // Returns true if the storage location is present.
        [[nodiscard]] bool HasValidStorageLocation() const noexcept
        {
            return !storageLocation.empty();
        }

        // Returns true if the supplier is present.
        [[nodiscard]] bool HasValidSupplier() const noexcept
        {
            return !supplier.empty();
        }

        // Returns true if the quantity is within the supported range.
        [[nodiscard]] constexpr bool HasValidStockQuantity() const noexcept
        {
            return stockQuantity <= MaxSupportedStockQuantity;
        }

        // Returns true if all required fields contain acceptable values.
        [[nodiscard]] bool IsValid() const noexcept
        {
            return HasValidPartNumber() &&
                HasValidPartName() &&
                HasValidCategory() &&
                HasValidStorageLocation() &&
                HasValidSupplier() &&
                HasValidStockQuantity();
        }

        // Returns true if this record still contains only default/empty values.
        [[nodiscard]] bool IsEmpty() const noexcept
        {
            return partNumber.empty() &&
                partName.empty() &&
                category.empty() &&
                storageLocation.empty() &&
                supplier.empty() &&
                stockQuantity == 0U;
        }

        // Maximum stock quantity allowed by the shared model.
        static inline constexpr std::uint32_t MaxSupportedStockQuantity = 1'000'000U;
    };

    // Returns true if the record's part number matches the expected value exactly.
    [[nodiscard]] inline bool MatchesPartNumber(
        const PartRecord& record,
        const std::string_view expectedPartNumber) noexcept
    {
        return record.partNumber == expectedPartNumber;
    }

    // Returns true if the record's part name matches the expected value exactly.
    [[nodiscard]] inline bool MatchesPartName(
        const PartRecord& record,
        const std::string_view expectedPartName) noexcept
    {
        return record.partName == expectedPartName;
    }
}