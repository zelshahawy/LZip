#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "dictionary.hpp"
#include "bitio.hpp"

namespace lzw
{

	inline std::vector<uint8_t> reconstructString(const std::vector<DictionaryEntry> &dict, int code)
	{
		if (code < 256)
			return {uint8_t(code)};
		std::vector<uint8_t> result;
		while (code != -1)
		{
			auto &e = dict[code];
			result.push_back(e.ch);
			code = e.prefix;
		}
		std::reverse(result.begin(), result.end());
		return result;
	}

	inline void execDecoding(std::istream &input, const std::string &filename)
	{
		auto bp = newBitPacker();
		auto [dict, lookup] = initDictionary();
		int nextCode{256};
		int codeSize{9};
		constexpr int MAXBITS{20};
		constexpr int MAXCODE{(1 << MAXBITS) - 1};

		int oldCode{-1};
		std::vector<uint8_t> oldString;
		std::vector<uint8_t> buf(4096);

		auto decodeOne = [&](int newCode)
		{
			std::vector<uint8_t> out;
			if (oldCode == -1)
			{
				oldCode = newCode;
				oldString = reconstructString(dict, newCode);
				return oldString;
			}
			if (newCode >= int(dict.size()) && newCode == nextCode)
			{
				auto tmp = oldString;
				tmp.push_back(oldString[0]);
				out = tmp;
			}
			else
			{
				out = reconstructString(dict, newCode);
			}
			if (nextCode < MAXCODE)
			{
				dict.push_back(DictionaryEntry{oldCode, out[0]});
				++nextCode;
				if (nextCode + 1 == (1 << codeSize) && codeSize < MAXBITS)
					++codeSize;
			}
			oldCode = newCode;
			oldString = out;
			return out;
		};

		while (input)
		{
			input.read(reinterpret_cast<char *>(buf.data()), buf.size());
			std::streamsize n = input.gcount();
			for (std::streamsize i = 0; i < n; ++i)
			{
				bp.bitBuf |= (uint64_t(buf[i]) << bp.bitCount);
				bp.bitCount += 8;
				while (bp.bitCount >= codeSize)
				{
					int code = bp.readCode(codeSize);
					if (code < 0)
						break;
					auto decoded = decodeOne(code);
					bp.output.insert(bp.output.end(), decoded.begin(), decoded.end());
				}
			}
		}
		while (true)
		{
			int code = bp.readCode(codeSize);
			if (code < 0)
				break;
			auto decoded = decodeOne(code);
			bp.output.insert(bp.output.end(), decoded.begin(), decoded.end());
		}

		bp.writeOutputToFile(filename + ".out");
	}

	inline void startDecoding(std::istream &input, const std::string &filename)
	{
		try
		{
			execDecoding(input, filename);
		}
		catch (...)
		{
			std::cerr << "Error Decoding\n";
		}
	}

} // namespace lzw
