#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
namespace Flux
{
	namespace Common
	{
		template <class T>
		static std::vector<T> ReadFile(const std::string& filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);

			if (!file.is_open()) {
				throw std::runtime_error("failed to open file!" + filename);
			}

			size_t fileSize = (size_t)file.tellg();

			std::vector<char> bufferChar(fileSize);
			std::vector<T> buffer;

			file.seekg(0);
			file.read(bufferChar.data(), fileSize);

			file.close();


			buffer.assign(bufferChar.begin(), bufferChar.end());
			return buffer;
		}
	}
}