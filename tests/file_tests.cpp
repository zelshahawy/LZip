#include "lzip/dictionary.hpp"
#include "lzip/bitio.hpp"
#include "lzip/encoder.hpp"
#include "lzip/decoder.hpp"
#include "lzip/cli.hpp"
#include "catch2/catch.hpp"
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace lzw;

TEST_CASE("Real-world file patterns", "[files]")
{
	SECTION("Text file simulation")
	{
		// Simulate a typical text file with repeated words
		std::string textContent =
				"The quick brown fox jumps over the lazy dog. "
				"The dog was sleeping under the tree. "
				"The fox was quick and brown, jumping over the dog again. "
				"This is a test of the LZW compression algorithm. "
				"The algorithm should compress repeated patterns efficiently. "
				"Words like 'the', 'fox', 'dog', and 'quick' appear multiple times. "
				"This repetition should lead to good compression ratios. ";

		// Repeat the content to simulate a larger file
		std::string fullContent;
		for (int i = 0; i < 50; ++i)
		{
			fullContent += textContent;
		}

		std::stringstream input(fullContent);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "text_file");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		double compressionRatio = static_cast<double>(compressedData.size()) / fullContent.size();

		// Should achieve good compression on text
		REQUIRE(compressionRatio < 0.7); // At least 30% compression
		REQUIRE(compressedData.size() > 0);

		unsetenv("CLI");
	}

	SECTION("Code file simulation")
	{
		// Simulate a C++ source file with repeated keywords
		std::string codeContent =
				"#include <iostream>\n"
				"#include <vector>\n"
				"#include <string>\n"
				"\n"
				"int main() {\n"
				"    std::vector<int> numbers;\n"
				"    std::string text = \"Hello World\";\n"
				"    \n"
				"    for (int i = 0; i < 100; ++i) {\n"
				"        numbers.push_back(i);\n"
				"        std::cout << \"Number: \" << i << std::endl;\n"
				"    }\n"
				"    \n"
				"    return 0;\n"
				"}\n";

		// Simulate multiple similar functions
		std::string fullCode;
		for (int i = 0; i < 20; ++i)
		{
			fullCode += codeContent;
		}

		std::stringstream input(fullCode);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "code_file");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		double compressionRatio = static_cast<double>(compressedData.size()) / fullCode.size();

		// Code should compress well due to repeated keywords
		REQUIRE(compressionRatio < 0.6); // At least 40% compression

		unsetenv("CLI");
	}

	SECTION("Log file simulation")
	{
		// Simulate a log file with timestamps and repeated messages
		std::stringstream logContent;
		std::vector<std::string> logMessages = {
				"INFO: Application started successfully",
				"DEBUG: Processing user request",
				"WARNING: Low memory detected",
				"ERROR: Connection timeout",
				"INFO: User authenticated",
				"DEBUG: Database query executed"};

		for (int day = 1; day <= 30; ++day)
		{
			for (int hour = 0; hour < 24; ++hour)
			{
				for (int minute = 0; minute < 60; minute += 15)
				{
					logContent << "2024-01-" << (day < 10 ? "0" : "") << day
										 << " " << (hour < 10 ? "0" : "") << hour
										 << ":" << (minute < 10 ? "0" : "") << minute << ":00 "
										 << logMessages[minute % logMessages.size()] << "\n";
				}
			}
		}

		std::string fullLog = logContent.str();
		std::stringstream input(fullLog);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "log_file");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		double compressionRatio = static_cast<double>(compressedData.size()) / fullLog.size();

		// Log files should compress very well due to repeated patterns
		REQUIRE(compressionRatio < 0.5); // At least 50% compression

		unsetenv("CLI");
	}
}

TEST_CASE("Edge case inputs", "[edge_cases]")
{
	SECTION("Single character file")
	{
		std::string singleChar = "A";
		std::stringstream input(singleChar);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "single");
		REQUIRE(true); // If we get here, no exception was thrown

		std::cout.rdbuf(orig);
		REQUIRE(compressed.str().size() > 0);
		unsetenv("CLI");
	}

	SECTION("Two character repetition")
	{
		std::string twoChars = "AB";
		std::stringstream input(twoChars);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "two_chars");
		REQUIRE(true); // If we get here, no exception was thrown

		std::cout.rdbuf(orig);
		REQUIRE(compressed.str().size() > 0);
		unsetenv("CLI");
	}

	SECTION("Highly repetitive single character")
	{
		std::string repeated(1000, 'X');
		std::stringstream input(repeated);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "repeated");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		// Should achieve excellent compression on single character repetition
		REQUIRE(compressedData.size() < repeated.size() / 10); // At least 90% compression

		unsetenv("CLI");
	}

	SECTION("Alternating pattern")
	{
		std::string pattern;
		for (int i = 0; i < 500; ++i)
		{
			pattern += (i % 2 == 0) ? "A" : "B";
		}

		std::stringstream input(pattern);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "alternating");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		// Alternating pattern should compress reasonably well
		REQUIRE(compressedData.size() < pattern.size());

		unsetenv("CLI");
	}
}

TEST_CASE("File extension handling", "[cli]")
{
	SECTION("Complex file extensions")
	{
		REQUIRE(getStem("archive.tar.gz") == "archive.tar");
		REQUIRE(getStem("backup.sql.bz2") == "backup.sql");
		REQUIRE(getStem("data.json.lzw") == "data.json");
	}

	SECTION("No extension files")
	{
		REQUIRE(getStem("README") == "README");
		REQUIRE(getStem("Makefile") == "Makefile");
		REQUIRE(getStem("LICENSE") == "LICENSE");
	}

	SECTION("Hidden files")
	{
		REQUIRE(getStem(".gitignore") == "");
		REQUIRE(getStem(".bashrc") == "");
		REQUIRE(getStem(".config.json") == ".config");
	}
}

TEST_CASE("Memory efficiency", "[memory]")
{
	SECTION("Large buffer handling")
	{
		// Test with data larger than the internal buffer (4096 bytes)
		std::string largeData(10000, 'X');
		// Add some variety to make it interesting
		for (size_t i = 1000; i < largeData.size(); i += 1000)
		{
			largeData[i] = 'Y';
		}

		std::stringstream input(largeData);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "large_buffer");
		REQUIRE(true); // If we get here, no exception was thrown

		std::cout.rdbuf(orig);
		REQUIRE(compressed.str().size() > 0);
		unsetenv("CLI");
	}

	SECTION("Multiple small chunks")
	{
		// Simulate reading in small chunks
		std::string data = "ABCDEFGHIJ";
		std::string repeatedData;
		for (int i = 0; i < 100; ++i)
		{
			repeatedData += data;
		}

		std::stringstream input(repeatedData);

		setenv("CLI", "1", 1);
		std::streambuf *orig = std::cout.rdbuf();
		std::stringstream compressed;
		std::cout.rdbuf(compressed.rdbuf());

		startEncoding(input, "small_chunks");

		std::cout.rdbuf(orig);

		std::string compressedData = compressed.str();
		REQUIRE(compressedData.size() > 0);
		REQUIRE(compressedData.size() < repeatedData.size());

		unsetenv("CLI");
	}
}
