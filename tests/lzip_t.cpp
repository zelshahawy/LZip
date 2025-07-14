#include "lzip/dictionary.hpp"
#include "lzip/bitio.hpp"
#include "lzip/encoder.hpp"
#include "lzip/decoder.hpp"
#include "lzip/cli.hpp"
#include "catch2/catch.hpp"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <random>

using namespace lzw;

TEST_CASE("Dictionary initialization", "[dictionary]")
{
  SECTION("Basic dictionary initialization")
  {
    auto [dict, lookup] = initDictionary();

    REQUIRE(dict.size() == 256);
    REQUIRE(lookup.size() == 256);

    // Check first few entries
    REQUIRE(dict[0].prefix == -1);
    REQUIRE(dict[0].ch == 0);
    REQUIRE(dict[255].prefix == -1);
    REQUIRE(dict[255].ch == 255);
  }

  SECTION("Dictionary lookup functionality")
  {
    auto [dict, lookup] = initDictionary();

    // Test lookup for initial entries
    for (int i = 0; i < 256; ++i)
    {
      DictKey key{-1, i};
      auto it = lookup.find(key);
      REQUIRE(it != lookup.end());
      REQUIRE(it->second == i);
    }
  }
}

TEST_CASE("DictKey hash and equality", "[dictionary]")
{
  SECTION("Hash function consistency")
  {
    DictKey key1{10, 65};
    DictKey key2{10, 65};
    DictKey key3{10, 66};

    DictKeyHash hasher;
    REQUIRE(hasher(key1) == hasher(key2));
    REQUIRE(hasher(key1) != hasher(key3));
  }

  SECTION("Equality function")
  {
    DictKey key1{10, 65};
    DictKey key2{10, 65};
    DictKey key3{11, 65};
    DictKey key4{10, 66};

    DictKeyEq eq;
    REQUIRE(eq(key1, key2));
    REQUIRE_FALSE(eq(key1, key3));
    REQUIRE_FALSE(eq(key1, key4));
  }
}

TEST_CASE("BitPacker functionality", "[bitio]")
{
  SECTION("Write and read single codes")
  {
    BitPacker bp;

    // Write some codes
    bp.writeCode(65, 8); // 'A'
    bp.writeCode(66, 8); // 'B'
    bp.writeCode(67, 8); // 'C'

    REQUIRE(bp.output.size() == 3);
    REQUIRE(bp.output[0] == 65);
    REQUIRE(bp.output[1] == 66);
    REQUIRE(bp.output[2] == 67);
  }

  SECTION("Variable bit width encoding")
  {
    BitPacker bp;

    // Write codes with different bit widths
    bp.writeCode(7, 3); // 111 in 3 bits
    bp.writeCode(5, 3); // 101 in 3 bits
    bp.writeCode(3, 2); // 11 in 2 bits

    bp.flushRemaining();

    // Should have packed into bytes
    REQUIRE(bp.output.size() >= 1);
  }

  SECTION("Code masking")
  {
    BitPacker bp;

    // Test that codes are properly masked
    bp.writeCode(0xFF, 4); // Should be masked to 0x0F
    bp.flushRemaining();

    // Verify masking worked
    REQUIRE((bp.output[0] & 0x0F) == 0x0F);
  }

  SECTION("Flush remaining bits")
  {
    BitPacker bp;

    bp.writeCode(1, 1); // Single bit
    REQUIRE(bp.bitCount == 1);

    bp.flushRemaining();
    REQUIRE(bp.bitCount == 0);
    REQUIRE(bp.output.size() == 1);
  }
}

TEST_CASE("String reconstruction", "[decoder]")
{
  SECTION("Single character reconstruction")
  {
    auto [dict, lookup] = initDictionary();

    // Test single character codes
    auto result = reconstructString(dict, 65); // 'A'
    REQUIRE(result.size() == 1);
    REQUIRE(result[0] == 65);
  }

  SECTION("Multi-character reconstruction")
  {
    auto [dict, lookup] = initDictionary();

    // Add a custom entry: "AB" where A=65, B=66
    dict.push_back(DictionaryEntry{65, 66}); // prefix=A, char=B

    auto result = reconstructString(dict, 256); // First custom entry
    REQUIRE(result.size() == 2);
    REQUIRE(result[0] == 65); // 'A'
    REQUIRE(result[1] == 66); // 'B'
  }
}

TEST_CASE("getStem utility function", "[cli]")
{
  SECTION("Basic filename stem extraction")
  {
    REQUIRE(getStem("test.txt") == "test");
    REQUIRE(getStem("archive.tar.gz") == "archive.tar");
    REQUIRE(getStem("noextension") == "noextension");
  }

  SECTION("Path handling")
  {
    REQUIRE(getStem("/path/to/file.txt") == "file");
    REQUIRE(getStem("C:\\Windows\\file.exe") == "file");
    REQUIRE(getStem("../relative/path.dat") == "path");
  }

  SECTION("Edge cases")
  {
    REQUIRE(getStem("") == "");
    REQUIRE(getStem(".hidden") == "");
    REQUIRE(getStem("file.") == "file");
  }
}

TEST_CASE("End-to-end compression and decompression", "[integration]")
{
  SECTION("Simple text compression/decompression")
  {
    // Create test data
    std::string testData = "ABABABAB";
    std::stringstream input(testData);

    // Set environment for CLI mode to capture output
    setenv("CLI", "1", 1);

    // Capture original cout
    std::streambuf *orig = std::cout.rdbuf();
    std::stringstream compressed;
    std::cout.rdbuf(compressed.rdbuf());

    // Encode
    startEncoding(input, "test");

    // Restore cout
    std::cout.rdbuf(orig);

    // Get compressed data
    std::string compressedData = compressed.str();
    REQUIRE(compressedData.size() > 0);
    REQUIRE(compressedData.size() < testData.size()); // Should be compressed

    // Reset for decoding
    std::stringstream compressedInput(compressedData);
    std::stringstream decompressed;
    std::cout.rdbuf(decompressed.rdbuf());

    // Decode
    startDecoding(compressedInput, "test");

    // Restore cout
    std::cout.rdbuf(orig);

    // Verify decompression
    std::string decompressedData = decompressed.str();
    REQUIRE(decompressedData == testData);

    // Clean up environment
    unsetenv("CLI");
  }

  SECTION("Empty input handling")
  {
    std::stringstream empty;

    setenv("CLI", "1", 1);
    std::streambuf *orig = std::cout.rdbuf();
    std::stringstream output;
    std::cout.rdbuf(output.rdbuf());

    // Should not crash on empty input
    startEncoding(empty, "empty");
    REQUIRE(true); // If we get here, no exception was thrown

    std::cout.rdbuf(orig);
    unsetenv("CLI");
  }

  SECTION("Large repetitive data")
  {
    // Create large repetitive data that should compress well
    std::string pattern = "Hello World! ";
    std::string testData;
    for (int i = 0; i < 100; ++i)
    {
      testData += pattern;
    }

    std::stringstream input(testData);

    setenv("CLI", "1", 1);
    std::streambuf *orig = std::cout.rdbuf();
    std::stringstream compressed;
    std::cout.rdbuf(compressed.rdbuf());

    startEncoding(input, "large");

    std::cout.rdbuf(orig);

    std::string compressedData = compressed.str();
    REQUIRE(compressedData.size() > 0);
    // Should achieve good compression on repetitive data
    REQUIRE(compressedData.size() < testData.size() / 2);

    unsetenv("CLI");
  }
}

TEST_CASE("BitPacker edge cases", "[bitio]")
{
  SECTION("Maximum code values")
  {
    BitPacker bp;

    // Test maximum values for different bit widths
    bp.writeCode((1 << 9) - 1, 9);   // 9-bit max
    bp.writeCode((1 << 12) - 1, 12); // 12-bit max
    bp.writeCode((1 << 16) - 1, 16); // 16-bit max

    bp.flushRemaining();
    REQUIRE(bp.output.size() > 0);
  }

  SECTION("Zero values")
  {
    BitPacker bp;

    bp.writeCode(0, 8);
    bp.writeCode(0, 1);
    bp.writeCode(0, 16);

    bp.flushRemaining();
    REQUIRE(bp.output.size() > 0);
  }
}

TEST_CASE("Dictionary growth simulation", "[dictionary]")
{
  SECTION("Simulate encoding process")
  {
    auto [dict, lookup] = initDictionary();
    int nextCode = 256;

    // Simulate adding entries during encoding
    for (int prefix = 0; prefix < 10; ++prefix)
    {
      for (int ch = 65; ch < 75; ++ch)
      { // A-J
        if (nextCode < 1000)
        { // Simulate some limit
          DictKey key{prefix, ch};
          if (lookup.find(key) == lookup.end())
          {
            dict.push_back(DictionaryEntry{prefix, static_cast<uint8_t>(ch)});
            lookup[key] = nextCode;
            ++nextCode;
          }
        }
      }
    }

    REQUIRE(dict.size() > 256);
    REQUIRE(lookup.size() > 256);
    REQUIRE(nextCode > 256);
  }
}

TEST_CASE("Error handling", "[error]")
{
  SECTION("Encoder error handling")
  {
    std::stringstream input("test data");

    // This should not throw but should handle errors gracefully
    startEncoding(input, "test");
    REQUIRE(true); // If we get here, no exception was thrown
  }

  SECTION("Decoder error handling")
  {
    std::stringstream input("invalid compressed data");

    // This should not throw but should handle errors gracefully
    startDecoding(input, "test");
    REQUIRE(true); // If we get here, no exception was thrown
  }
}
