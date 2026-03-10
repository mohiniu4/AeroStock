#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/PartRecord.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(PartRecordTests)
    {
    public:
        // Verifies that a fully populated part record with a valid stock quantity
        // passes validation.
        TEST_METHOD(ValidPartRecord_IsValid)
        {
            AeroStock::Common::PartRecord record{};
            record.partNumber = "PN-1001";
            record.partName = "Hydraulic Pump";
            record.category = "Hydraulics";
            record.storageLocation = "A1-SHELF-3";
            record.supplier = "Boeing Supply";
            record.stockQuantity = 10U;

            Assert::IsTrue(record.IsValid());
        }

        // Verifies that a missing part number causes the part record to fail validation.
        TEST_METHOD(PartRecord_WithEmptyPartNumber_IsInvalid)
        {
            AeroStock::Common::PartRecord record{};
            record.partName = "Hydraulic Pump";
            record.category = "Hydraulics";
            record.storageLocation = "A1-SHELF-3";
            record.supplier = "Boeing Supply";
            record.stockQuantity = 10U;

            Assert::IsFalse(record.IsValid());
        }

        // Verifies that part records reject stock quantities that exceed the supported limit.
        TEST_METHOD(PartRecord_WithExcessiveQuantity_IsInvalid)
        {
            AeroStock::Common::PartRecord record{};
            record.partNumber = "PN-1001";
            record.partName = "Hydraulic Pump";
            record.category = "Hydraulics";
            record.storageLocation = "A1-SHELF-3";
            record.supplier = "Boeing Supply";
            record.stockQuantity =
                AeroStock::Common::PartRecord::MaxSupportedStockQuantity + 1U;

            Assert::IsFalse(record.IsValid());
        }

        // Verifies that a default-constructed record is recognized as empty.
        TEST_METHOD(EmptyPartRecord_IsEmpty)
        {
            AeroStock::Common::PartRecord record{};

            Assert::IsTrue(record.IsEmpty());
        }

        // Verifies that exact part-number matching succeeds for the expected identifier.
        TEST_METHOD(MatchesPartNumber_ExactMatch_ReturnsTrue)
        {
            AeroStock::Common::PartRecord record{};
            record.partNumber = "PN-1001";

            Assert::IsTrue(
                AeroStock::Common::MatchesPartNumber(record, "PN-1001"));
        }

        // Verifies that exact part-name matching succeeds for the expected display name.
        TEST_METHOD(MatchesPartName_ExactMatch_ReturnsTrue)
        {
            AeroStock::Common::PartRecord record{};
            record.partName = "Hydraulic Pump";

            Assert::IsTrue(
                AeroStock::Common::MatchesPartName(record, "Hydraulic Pump"));
        }
    };
}