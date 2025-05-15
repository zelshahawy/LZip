#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace lzw
{

	struct DictionaryEntry
	{
		int prefix;
		uint8_t ch;
	};

	struct DictKey
	{
		int prefix;
		int ch;
	};

	struct DictKeyHash
	{
		size_t operator()(const DictKey &k) const noexcept
		{
			return std::hash<int>()(k.prefix) ^ (std::hash<int>()(k.ch) << 1);
		}
	};

	struct DictKeyEq
	{
		bool operator()(const DictKey &a, const DictKey &b) const noexcept
		{
			return a.prefix == b.prefix && a.ch == b.ch;
		}
	};

	inline auto initDictionary()
	{
		std::vector<DictionaryEntry> dict(256);
		std::unordered_map<DictKey, int, DictKeyHash, DictKeyEq> lookup;
		for (int i = 0; i < 256; ++i)
		{
			dict[i] = DictionaryEntry{-1, static_cast<uint8_t>(i)};
			lookup[DictKey{-1, i}] = i;
		}
		return std::make_pair(dict, lookup);
	}

} // namespace lzw
