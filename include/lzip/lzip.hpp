#pragma once

// Main LZip header file - includes all components
#include "dictionary.hpp"
#include "bitio.hpp"
#include "encoder.hpp"
#include "decoder.hpp"
#include "cli.hpp"

namespace lzip
{
	// Export the main namespace
	using namespace lzw;

	// Convenience function for testing
	inline int hash(int value)
	{
		return value + 1;
	}
}
