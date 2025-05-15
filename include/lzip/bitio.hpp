#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdlib>

namespace lzw
{

	struct BitPacker
	{
		std::vector<uint8_t> output{};
		uint64_t bitBuf{};
		int bitCount{};

		inline void writeCode(int code, int codeSize)
		{
			code &= (1 << codeSize) - 1;
			bitBuf |= (uint64_t(code) << bitCount);
			bitCount += codeSize;
			while (bitCount >= 8)
			{
				output.push_back(uint8_t(bitBuf & 0xFF));
				bitBuf >>= 8;
				bitCount -= 8;
			}
		}

		inline int readCode(int codeSize)
		{
			if (bitCount < codeSize)
				return -1;
			int code = 0;
			for (int i = 0; i < codeSize; ++i)
			{
				code |= int(bitBuf & 1) << i;
				bitBuf >>= 1;
				--bitCount;
			}
			return code;
		}

		inline void flushRemaining()
		{
			if (bitCount > 0)
			{
				output.push_back(uint8_t(bitBuf & 0xFF));
				bitBuf = 0;
				bitCount = 0;
			}
		}

		inline void writeOutputToFile(const std::string &outFile)
		{
			if (std::getenv("CLI") && std::string(std::getenv("CLI")) == "1")
			{
				std::cout.write(reinterpret_cast<const char *>(output.data()), output.size());
			}
			else
			{
				std::ofstream ofs(outFile, std::ios::binary);
				ofs.write(reinterpret_cast<const char *>(output.data()), output.size());
			}
		}
	};

	inline auto newBitPacker()
	{
		return BitPacker{};
	}

} // namespace lzw
