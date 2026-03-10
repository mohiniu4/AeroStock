#include "pch.h"
#include "CppUnitTest.h"

#include <vector>

#include "../Common/Checksum.h"
#include "../Common/PayloadUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(ChecksumTests)
    {
    public:
        // Verifies that checksum generation is deterministic for identical input data.
        TEST_METHOD(Calculate_SameInput_ReturnsSameChecksum)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("AEROSTOCK_VERIFY");

            const auto checksum1 = AeroStock::Common::Checksum::Calculate(payload);
            const auto checksum2 = AeroStock::Common::Checksum::Calculate(payload);

            Assert::AreEqual(checksum1, checksum2);
        }

        // Verifies that different payload values do not produce the same checksum.
        TEST_METHOD(Calculate_DifferentInput_ReturnsDifferentChecksum)
        {
            const auto payload1 =
                AeroStock::Common::PayloadUtils::StringToPayload("ABC");
            const auto payload2 =
                AeroStock::Common::PayloadUtils::StringToPayload("ABD");

            const auto checksum1 = AeroStock::Common::Checksum::Calculate(payload1);
            const auto checksum2 = AeroStock::Common::Checksum::Calculate(payload2);

            Assert::AreNotEqual(checksum1, checksum2);
        }

        // Verifies that checksum validation succeeds when the expected checksum
        // matches the payload content exactly.
        TEST_METHOD(Validate_MatchingChecksum_ReturnsTrue)
        {
            const auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("VERIFIED");

            const auto checksum = AeroStock::Common::Checksum::Calculate(payload);

            Assert::IsTrue(AeroStock::Common::Checksum::Validate(payload, checksum));
        }

        // Verifies that checksum validation fails when the payload is modified
        // after the checksum has already been calculated.
        TEST_METHOD(Validate_ModifiedPayload_ReturnsFalse)
        {
            auto payload =
                AeroStock::Common::PayloadUtils::StringToPayload("VERIFIED");

            const auto checksum = AeroStock::Common::Checksum::Calculate(payload);
            payload[0] = static_cast<std::uint8_t>('X');

            Assert::IsFalse(AeroStock::Common::Checksum::Validate(payload, checksum));
        }

        // Verifies the expected checksum behavior for an empty payload buffer.
        TEST_METHOD(Calculate_EmptyPayload_ReturnsInitialSeedValue)
        {
            const std::vector<std::uint8_t> emptyPayload{};

            const auto checksum = AeroStock::Common::Checksum::Calculate(emptyPayload);

            Assert::AreEqual(2166136261U, checksum);
        }
    };
}