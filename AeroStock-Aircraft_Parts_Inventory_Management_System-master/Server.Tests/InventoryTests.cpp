#include "pch.h"
#include "CppUnitTest.h"

#include "Inventory.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ServerTests
{
    TEST_CLASS(InventoryTests)
    {
    public:
        // Verifies that the inventory constructor loads the sample data set.
        TEST_METHOD(Constructor_LoadsSampleData)
        {
            AeroStock::Server::Inventory inventory{};

            const auto& records = inventory.GetAllRecords();

            Assert::IsFalse(records.empty());
            Assert::AreEqual(static_cast<std::size_t>(15), records.size());
        }

        // Verifies that searching for a known part number returns the expected record.
        TEST_METHOD(FindByPartNumber_KnownPart_ReturnsRecord)
        {
            AeroStock::Server::Inventory inventory{};

            const auto result = inventory.FindByPartNumber("PN-1001");

            Assert::IsTrue(result.has_value());
            Assert::AreEqual("PN-1001", result->partNumber.c_str());
            Assert::AreEqual("Hydraulic Pump", result->partName.c_str());
            Assert::AreEqual(static_cast<std::uint32_t>(25), result->stockQuantity);
        }

        // Verifies that part-number search is case-insensitive.
        TEST_METHOD(FindByPartNumber_LowercaseInput_ReturnsRecord)
        {
            AeroStock::Server::Inventory inventory{};

            const auto result = inventory.FindByPartNumber("pn-1001");

            Assert::IsTrue(result.has_value());
            Assert::AreEqual("PN-1001", result->partNumber.c_str());
            Assert::AreEqual("Hydraulic Pump", result->partName.c_str());
        }

        // Verifies that searching for an unknown part number returns no result.
        TEST_METHOD(FindByPartNumber_UnknownPart_ReturnsNoResult)
        {
            AeroStock::Server::Inventory inventory{};

            const auto result = inventory.FindByPartNumber("PN-9999");

            Assert::IsFalse(result.has_value());
        }

        // Verifies that searching by part name substring returns matching records.
        TEST_METHOD(FindByPartName_SubstringMatch_ReturnsResults)
        {
            AeroStock::Server::Inventory inventory{};

            const auto results = inventory.FindByPartName("Hydraulic");

            Assert::IsFalse(results.empty());
            Assert::IsTrue(results.size() >= 2);

            bool foundPump = false;
            bool foundFilter = false;

            for (const auto& record : results)
            {
                if (record.partNumber == "PN-1001")
                {
                    foundPump = true;
                }

                if (record.partNumber == "PN-1013")
                {
                    foundFilter = true;
                }
            }

            Assert::IsTrue(foundPump);
            Assert::IsTrue(foundFilter);
        }

        // Verifies that part-name search is case-insensitive.
        TEST_METHOD(FindByPartName_LowercaseInput_ReturnsResults)
        {
            AeroStock::Server::Inventory inventory{};

            const auto results = inventory.FindByPartName("hydraulic");

            Assert::IsFalse(results.empty());
            Assert::IsTrue(results.size() >= 2);
        }

        // Verifies that searching by a name with no matches returns an empty result set.
        TEST_METHOD(FindByPartName_NoMatches_ReturnsEmptyResults)
        {
            AeroStock::Server::Inventory inventory{};

            const auto results = inventory.FindByPartName("Quantum Reactor");

            Assert::IsTrue(results.empty());
        }

        // Verifies that GetPartDetails returns the full record for a known part number.
        TEST_METHOD(GetPartDetails_KnownPart_ReturnsRecord)
        {
            AeroStock::Server::Inventory inventory{};

            const auto result = inventory.GetPartDetails("PN-1003");

            Assert::IsTrue(result.has_value());
            Assert::AreEqual("PN-1003", result->partNumber.c_str());
            Assert::AreEqual("Avionics Display Unit", result->partName.c_str());
        }

        // Verifies that updating stock for a known part succeeds and persists the new value.
        TEST_METHOD(UpdateStock_KnownPart_UpdatesQuantity)
        {
            AeroStock::Server::Inventory inventory{};

            const bool updateResult = inventory.UpdateStock("PN-1001", 99U);
            const auto updatedRecord = inventory.FindByPartNumber("PN-1001");

            Assert::IsTrue(updateResult);
            Assert::IsTrue(updatedRecord.has_value());
            Assert::AreEqual(static_cast<std::uint32_t>(99), updatedRecord->stockQuantity);
        }

        // Verifies that updating stock is case-insensitive for part number input.
        TEST_METHOD(UpdateStock_LowercasePartNumber_UpdatesQuantity)
        {
            AeroStock::Server::Inventory inventory{};

            const bool updateResult = inventory.UpdateStock("pn-1001", 88U);
            const auto updatedRecord = inventory.FindByPartNumber("PN-1001");

            Assert::IsTrue(updateResult);
            Assert::IsTrue(updatedRecord.has_value());
            Assert::AreEqual(static_cast<std::uint32_t>(88), updatedRecord->stockQuantity);
        }

        // Verifies that updating stock for an unknown part fails.
        TEST_METHOD(UpdateStock_UnknownPart_ReturnsFalse)
        {
            AeroStock::Server::Inventory inventory{};

            const bool updateResult = inventory.UpdateStock("PN-9999", 50U);

            Assert::IsFalse(updateResult);
        }

        // Verifies that stock updates beyond the supported maximum are rejected.
        TEST_METHOD(UpdateStock_ExcessiveQuantity_ReturnsFalse)
        {
            AeroStock::Server::Inventory inventory{};

            const bool updateResult = inventory.UpdateStock(
                "PN-1001",
                AeroStock::Common::PartRecord::MaxSupportedStockQuantity + 1U);

            Assert::IsFalse(updateResult);
        }
    };
}