#include "pch.h"
#include "CppUnitTest.h"

#include "ClientFileManager.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ClientTests
{
    TEST_CLASS(ClientFileManagerTests)
    {
    public:
        TEST_METHOD(GetDefaultReceivedFileName_ReturnsExpectedName)
        {
            const auto fileName =
                AeroStock::Client::ClientFileManager::GetDefaultReceivedFileName();

            Assert::AreEqual("received_inventory_catalog.dat", fileName.c_str());
        }

        TEST_METHOD(SavePayloadToFile_ValidPayload_CreatesFileWithExpectedContents)
        {
            const std::string outputFileName = "client_file_manager_test_output.dat";
            const AeroStock::Common::Packet::PayloadBuffer payload{
                'A', 'e', 'r', 'o', 'S', 't', 'o', 'c', 'k'
            };

            AeroStock::Client::ClientFileManager::SavePayloadToFile(payload, outputFileName);

            Assert::IsTrue(std::filesystem::exists(outputFileName));

            std::ifstream inputFile(outputFileName, std::ios::binary);
            Assert::IsTrue(inputFile.is_open());

            const std::vector<char> fileContents{
                std::istreambuf_iterator<char>(inputFile),
                std::istreambuf_iterator<char>()
            };

            Assert::AreEqual(
                static_cast<unsigned __int64>(payload.size()),
                static_cast<unsigned __int64>(fileContents.size()));

            for (std::size_t index = 0; index < payload.size(); ++index)
            {
                Assert::AreEqual(
                    static_cast<int>(payload[index]),
                    static_cast<int>(static_cast<unsigned char>(fileContents[index])));
            }

            inputFile.close();

            const bool removed = std::filesystem::remove(outputFileName);
            Assert::IsTrue(removed);
        }

        TEST_METHOD(SavePayloadToFile_EmptyPayload_ThrowsRuntimeError)
        {
            const std::string outputFileName = "client_file_manager_empty_test.dat";
            const AeroStock::Common::Packet::PayloadBuffer emptyPayload{};

            Assert::ExpectException<std::runtime_error>(
                [&]()
                {
                    AeroStock::Client::ClientFileManager::SavePayloadToFile(
                        emptyPayload,
                        outputFileName);
                });

            if (std::filesystem::exists(outputFileName))
            {
                std::filesystem::remove(outputFileName);
            }
        }
    };
}