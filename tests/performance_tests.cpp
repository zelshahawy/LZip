#include "lzip/dictionary.hpp"
#include "lzip/bitio.hpp"
#include "lzip/encoder.hpp"
#include "lzip/decoder.hpp"
#include "lzip/cli.hpp"
#include "catch2/catch.hpp"
#include <sstream>
#include <random>
#include <algorithm>
#include <chrono>
#include <numeric>

using namespace lzw;

TEST_CASE("Performance tests", "[performance]")
{
	SECTION("Large file simulation")
	{
		// Generate large test data
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> dis(0, 255);

		std::vector<uint8_t> testData(10000);
		std::generate(testData.begin(), testData.end(), [&]()
									{ return dis(gen); });

		std::string dataStr(testData.begin(), testData.end());
		std::stringstream input(dataStr);

		// Measure encoding performance
		auto start = std::chrono::high_resolution_clock::now();

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream output;
		std::cout.rdbuf(output.rdbuf());

		startEncoding(input, "performance_test");

		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout.rdbuf(orig);
		unsetenv("CLI");

		// Should complete in reasonable time (adjust threshold as needed)
		REQUIRE(duration.count() < 5000); // 5 seconds max
		REQUIRE(output.str().size() > 0);
	}

	SECTION("Repetitive pattern compression efficiency")
	{
		// Create highly repetitive data
		std::string pattern = "ABCDEFGH";
		std::string testData;
		for (int i = 0; i < 1000; ++i)
		{
			testData += pattern;
		}

		std::stringstream input(testData);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "repetitive");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		double compressionRatio = static_cast<double>(compressedData.size()) / testData.size();

		// Should achieve good compression ratio on repetitive data
		REQUIRE(compressionRatio < 0.5); // At least 50% compression

		unsetenv("CLI");
	}
}

TEST_CASE("Stress tests", "[stress]")
{
	SECTION("Maximum dictionary growth")
	{
		auto [dict, lookup] = initDictionary();
		int nextCode = 256;
		constexpr int MAXCODE = (1 << 20) - 1; // 20-bit maximum

		// Fill dictionary to near maximum
		for (int prefix = 0; prefix < 256 && nextCode < MAXCODE - 1000; ++prefix)
		{
			for (int ch = 0; ch < 256 && nextCode < MAXCODE - 1000; ++ch)
			{
				DictKey key{prefix, ch};
				if (lookup.find(key) == lookup.end())
				{
					dict.push_back(DictionaryEntry{prefix, static_cast<uint8_t>(ch)});
					lookup[key] = nextCode;
					++nextCode;
				}
			}
		}

		REQUIRE(dict.size() > 10000);
		REQUIRE(nextCode < MAXCODE);
	}

	SECTION("Edge case bit widths")
	{
		BitPacker bp;

		// Test all possible bit widths
		for (int bitWidth = 1; bitWidth <= 20; ++bitWidth)
		{
			int maxVal = (1 << bitWidth) - 1;
			bp.writeCode(maxVal, bitWidth);
			bp.writeCode(0, bitWidth);
			bp.writeCode(maxVal / 2, bitWidth);
		}

		bp.flushRemaining();
		REQUIRE(bp.output.size() > 0);
	}
}

TEST_CASE("Binary data handling", "[binary]")
{
	SECTION("All byte values")
	{
		// Test with all possible byte values
		std::vector<uint8_t> allBytes(256);
		std::iota(allBytes.begin(), allBytes.end(), 0);

		std::string testData(allBytes.begin(), allBytes.end());
		std::stringstream input(testData);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "binary");
		REQUIRE(true); // If we get here, no exception was thrown

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		REQUIRE(compressedData.size() > 0);

		// Test decompression
		std::stringstream compressedInput(compressedData);
		std::stringstream decompressed;
		std::cout.rdbuf(decompressed.rdbuf());

		startDecoding(compressedInput, "binary");
		REQUIRE(true); // If we get here, no exception was thrown

		std::cout.rdbuf(orig);

		std::string decompressedData = decompressed.str();
		REQUIRE(decompressedData == testData);

		unsetenv("CLI");
	}

	SECTION("Random binary data")
	{
		std::random_device rd;
		std::mt19937 gen(42); // Fixed seed for reproducibility
		std::uniform_int_distribution<> dis(0, 255);

		std::vector<uint8_t> randomData(1000);
		std::generate(randomData.begin(), randomData.end(), [&]()
									{ return dis(gen); });

		std::string testData(randomData.begin(), randomData.end());
		std::stringstream input(testData);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "random");
		REQUIRE(true); // If we get here, no exception was thrown

		std::cout.rdbuf(orig);
		unsetenv("CLI");

		REQUIRE(compressed.str().size() > 0);
	}
}
