#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include "encoder.hpp"
#include "decoder.hpp"

namespace lzw
{

	inline std::string getStem(const std::string &path)
	{
		auto pos = path.find_last_of("/\\");
		std::string file = (pos == std::string::npos) ? path : path.substr(pos + 1);
		auto dot = file.find_last_of('.');
		return (dot == std::string::npos) ? file : file.substr(0, dot);
	}

	inline void runCLI(int argc, char *argv[])
	{
		std::string cmdName = getStem(argv[0]);
		if (argc > 9)
		{
			std::cerr << "Too many filenames provided, maximum is 8\n";
			std::exit(1);
		}

		std::vector<std::thread> threads;

		auto dispatch = [&](std::istream &in, const std::string &name)
		{
			if (cmdName == "encode")
			{
				startEncoding(in, name);
			}
			else if (cmdName == "decode")
			{
				startDecoding(in, name);
			}
			else
			{
				std::cerr << "Invalid command name\n";
				std::exit(1);
			}
		};

		if (argc < 2)
		{
			threads.emplace_back([&]()
													 {
            std::cout << "Enter text to " << cmdName << " (Ctrl+D to finish): ";
            dispatch(std::cin, "stdin"); });
		}
		else
		{
			for (int i = 1; i < argc; ++i)
			{
				threads.emplace_back([&, i]()
														 {
                std::string filename = argv[i];
                std::ifstream file(filename, std::ios::binary);
                if (!file) {
                    std::cerr << "Error opening file: " << filename << "\n";
                    return;
                }
                dispatch(file, getStem(filename)); });
			}
		}

		for (auto &t : threads)
		{
			if (t.joinable())
				t.join();
		}
	}

} // namespace lzw
