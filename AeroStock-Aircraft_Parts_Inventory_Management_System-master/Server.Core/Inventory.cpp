#include "pch.h"
#include "Inventory.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace AeroStock::Server
{
    namespace
    {
        // converts a string to lowercase for case insensitive comparison
        [[nodiscard]] std::string ToLowerCopy(const std::string_view text)
        {
            std::string normalizedText;
            normalizedText.reserve(text.size());

            for (const char character : text)
            {
                normalizedText.push_back(static_cast<char>(
                    std::tolower(static_cast<unsigned char>(character))));
            }

            return normalizedText;
        }
    }

    Inventory::Inventory()
    {
        LoadSampleData();
    }

    void Inventory::LoadSampleData()
    {
        records_ = {
            { "PN-1001", "Hydraulic Pump",            "Hydraulics",    "A1-SHELF-3", "Boeing Supply",      25 },
            { "PN-1002", "Landing Gear Actuator",    "Landing Gear",  "B2-SHELF-1", "Airbus Parts Co",    12 },
            { "PN-1003", "Avionics Display Unit",    "Avionics",      "C3-SHELF-7", "Honeywell Aero",      8 },
            { "PN-1004", "Fuel Control Valve",       "Fuel System",   "A1-SHELF-5", "Parker Hannifin",    30 },
            { "PN-1005", "Engine Turbine Blade",     "Engine",        "D4-SHELF-2", "GE Aviation",        50 },
            { "PN-1006", "Cabin Pressure Sensor",    "Environmental", "C3-SHELF-9", "Collins Aerospace",  15 },
            { "PN-1007", "Wing Flap Motor",          "Flight Ctrl",   "B2-SHELF-4", "Moog Inc",           10 },
            { "PN-1008", "Navigation Antenna",       "Avionics",      "C3-SHELF-2", "L3Harris",           20 },
            { "PN-1009", "Brake Assembly",           "Landing Gear",  "B2-SHELF-6", "Safran",             18 },
            { "PN-1010", "APU Starter Motor",        "APU",           "D4-SHELF-8", "Honeywell Aero",      6 },
            { "PN-1011", "Oxygen Mask Assembly",     "Safety",        "E5-SHELF-1", "Zodiac Aerospace",   40 },
            { "PN-1012", "Pitot Tube Probe",         "Instruments",   "C3-SHELF-4", "Thales Group",       22 },
            { "PN-1013", "Hydraulic Filter Element", "Hydraulics",    "A1-SHELF-7", "Pall Corporation",   35 },
            { "PN-1014", "Generator Control Unit",   "Electrical",    "D4-SHELF-3", "Collins Aerospace",  11 },
            { "PN-1015", "Fire Extinguisher Bottle", "Safety",        "E5-SHELF-3", "Kidde Aerospace",    14 },
        };
    }

    // searches by part number
    std::optional<AeroStock::Common::PartRecord>
        Inventory::FindByPartNumber(const std::string_view partNumber) const
    {
        const std::string normalizedSearchPartNumber = ToLowerCopy(partNumber);

        const auto it = std::find_if(
            records_.begin(),
            records_.end(),
            [&normalizedSearchPartNumber](const AeroStock::Common::PartRecord& record)
            {
                return ToLowerCopy(record.partNumber) == normalizedSearchPartNumber;
            });

        if (it != records_.end())
        {
            return *it;
        }

        return std::nullopt;
    }

    // searches by part name
    std::vector<AeroStock::Common::PartRecord>
        Inventory::FindByPartName(const std::string_view partName) const
    {
        std::vector<AeroStock::Common::PartRecord> results;
        const std::string normalizedSearchPartName = ToLowerCopy(partName);

        for (const auto& record : records_)
        {
            const std::string normalizedRecordPartName = ToLowerCopy(record.partName);

            if (normalizedRecordPartName.find(normalizedSearchPartName) != std::string::npos)
            {
                results.push_back(record);
            }
        }

        return results;
    }

    std::optional<AeroStock::Common::PartRecord>
        Inventory::GetPartDetails(const std::string_view partNumber) const
    {
        return FindByPartNumber(partNumber);
    }

    // updates quantity if the part exists and within range
    bool Inventory::UpdateStock(
        const std::string_view partNumber,
        const std::uint32_t newQuantity)
    {
        if (newQuantity > AeroStock::Common::PartRecord::MaxSupportedStockQuantity)
        {
            return false;
        }

        const std::string normalizedSearchPartNumber = ToLowerCopy(partNumber);

        const auto it = std::find_if(
            records_.begin(),
            records_.end(),
            [&normalizedSearchPartNumber](const AeroStock::Common::PartRecord& record)
            {
                return ToLowerCopy(record.partNumber) == normalizedSearchPartNumber;
            });

        if (it == records_.end())
        {
            return false;
        }

        it->stockQuantity = newQuantity;
        return true;
    }

    const std::vector<AeroStock::Common::PartRecord>&
        Inventory::GetAllRecords() const noexcept
    {
        return records_;
    }
}