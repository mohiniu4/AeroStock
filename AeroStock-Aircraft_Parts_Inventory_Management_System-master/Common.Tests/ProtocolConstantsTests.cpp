#include "pch.h"
#include "CppUnitTest.h"

#include "../Common/ProtocolConstants.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CommonTests
{
    TEST_CLASS(ProtocolConstantsTests)
    {
    public:
        // Verifies that the fixed serialized header size matches the intended packet design.
        TEST_METHOD(FixedHeaderSize_IsExpectedValue)
        {
            Assert::AreEqual(
                static_cast<std::size_t>(15U),
                AeroStock::Common::ProtocolConstants::FixedHeaderSizeBytes);
        }

        // Verifies that verification payload constants are initialized and distinct.
        TEST_METHOD(VerificationConstants_AreNotEmptyOrDuplicated)
        {
            using namespace AeroStock::Common::ProtocolConstants;

            Assert::AreEqual("AEROSTOCK_VERIFY", VerificationRequestText);
            Assert::AreEqual("VERIFIED", VerificationSuccessText);
            Assert::AreEqual("VERIFICATION_FAILED", VerificationFailureText);

            Assert::AreNotEqual(VerificationRequestText, VerificationSuccessText);
            Assert::AreNotEqual(VerificationRequestText, VerificationFailureText);
            Assert::AreNotEqual(VerificationSuccessText, VerificationFailureText);
        }
    };
}