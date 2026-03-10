#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/Enums.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(EnumTests)
    {
    public:
        // Verifies that a known valid command enum value passes validation.
        TEST_METHOD(CommandType_ValidValue_ReturnsTrue)
        {
            Assert::IsTrue(AeroStock::Common::IsValidCommandType(
                AeroStock::Common::CommandType::Verify));
        }

        // Verifies that an out-of-range command enum value is rejected.
        TEST_METHOD(CommandType_InvalidValue_ReturnsFalse)
        {
            const auto invalidCommand =
                static_cast<AeroStock::Common::CommandType>(99);

            Assert::IsFalse(AeroStock::Common::IsValidCommandType(invalidCommand));
        }

        // Verifies that a known valid status enum value passes validation.
        TEST_METHOD(StatusCode_ValidValue_ReturnsTrue)
        {
            Assert::IsTrue(AeroStock::Common::IsValidStatusCode(
                AeroStock::Common::StatusCode::Success));
        }

        // Verifies that an out-of-range status enum value is rejected.
        TEST_METHOD(StatusCode_InvalidValue_ReturnsFalse)
        {
            const auto invalidStatus =
                static_cast<AeroStock::Common::StatusCode>(99);

            Assert::IsFalse(AeroStock::Common::IsValidStatusCode(invalidStatus));
        }

        // Verifies that a known valid server state passes validation.
        TEST_METHOD(ServerState_ValidValue_ReturnsTrue)
        {
            Assert::IsTrue(AeroStock::Common::IsValidServerState(
                AeroStock::Common::ServerState::Verified));
        }

        // Verifies that a known valid error code passes validation.
        TEST_METHOD(ErrorCode_ValidValue_ReturnsTrue)
        {
            Assert::IsTrue(AeroStock::Common::IsValidErrorCode(
                AeroStock::Common::ErrorCode::ChecksumMismatch));
        }

        // Verifies that command enum values are converted to the expected text.
        TEST_METHOD(ToString_CommandType_ReturnsExpectedText)
        {
            const auto text = AeroStock::Common::ToString(
                AeroStock::Common::CommandType::RequestFile);

            Assert::AreEqual("RequestFile", text.data());
        }

        // Verifies that status enum values are converted to the expected text.
        TEST_METHOD(ToString_StatusCode_ReturnsExpectedText)
        {
            const auto text = AeroStock::Common::ToString(
                AeroStock::Common::StatusCode::VerificationFailed);

            Assert::AreEqual("VerificationFailed", text.data());
        }

        // Verifies that an invalid command enum value maps to the fallback text.
        TEST_METHOD(ToString_InvalidCommandType_ReturnsUnknownText)
        {
            const auto invalidCommand =
                static_cast<AeroStock::Common::CommandType>(250);

            const auto text = AeroStock::Common::ToString(invalidCommand);

            Assert::AreEqual("UnknownCommandType", text.data());
        }
    };
}