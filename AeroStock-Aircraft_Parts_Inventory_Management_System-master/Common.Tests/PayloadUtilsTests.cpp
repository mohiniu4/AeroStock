#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/PayloadUtils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(PayloadUtilsTests)
    {
    public:
        // Verifies that text converted into a payload buffer can be converted
        // back into the original text without data loss.
        TEST_METHOD(StringToPayload_AndBack_RetainsOriginalText)
        {
            constexpr std::string_view text = "AEROSTOCK_VERIFY";

            const auto payload = AeroStock::Common::PayloadUtils::StringToPayload(text);
            const auto convertedText = AeroStock::Common::PayloadUtils::PayloadToString(payload);

            Assert::AreEqual(text.data(), convertedText.c_str());
        }

        // Verifies that payload comparison succeeds when the payload content
        // matches the expected text exactly.
        TEST_METHOD(PayloadEqualsText_MatchingText_ReturnsTrue)
        {
            const auto payload = AeroStock::Common::PayloadUtils::StringToPayload("VERIFIED");

            Assert::IsTrue(
                AeroStock::Common::PayloadUtils::PayloadEqualsText(payload, "VERIFIED"));
        }

        // Verifies that payload comparison fails when the expected text differs
        // from the actual payload content.
        TEST_METHOD(PayloadEqualsText_DifferentText_ReturnsFalse)
        {
            const auto payload = AeroStock::Common::PayloadUtils::StringToPayload("VERIFIED");

            Assert::IsFalse(
                AeroStock::Common::PayloadUtils::PayloadEqualsText(payload, "FAILED"));
        }

        // Verifies that converting an empty payload buffer produces an empty string.
        TEST_METHOD(PayloadToString_EmptyPayload_ReturnsEmptyString)
        {
            AeroStock::Common::Packet::PayloadBuffer emptyPayload{};

            const auto text = AeroStock::Common::PayloadUtils::PayloadToString(emptyPayload);

            Assert::IsTrue(text.empty());
        }

        // Verifies that an empty string converts into an empty payload buffer.
        TEST_METHOD(StringToPayload_EmptyString_ReturnsEmptyPayload)
        {
            const auto payload = AeroStock::Common::PayloadUtils::StringToPayload("");

            Assert::IsTrue(payload.empty());
        }

        // Verifies that an empty payload matches an empty expected text value.
        TEST_METHOD(PayloadEqualsText_EmptyPayloadAndEmptyText_ReturnsTrue)
        {
            AeroStock::Common::Packet::PayloadBuffer emptyPayload{};

            Assert::IsTrue(
                AeroStock::Common::PayloadUtils::PayloadEqualsText(emptyPayload, ""));
        }
    };
}