/* MIT License

Copyright (c) 2018 melindil

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "rapidjson/document.h"
#include "Logger.h"

class GenericPatcher
{
public:
	GenericPatcher(Logger*, std::string const& configname);
	~GenericPatcher();

	void apply();

private:

	struct ApplyType
	{
		off_t offset;
		std::vector<unsigned char> patch;
	};

	struct PatchType
	{
		std::string name;
		std::vector<ApplyType> changes;
	};

	void apply_impl(ApplyType const& p);
	std::vector<ApplyType> ParseChanges(std::string const& patchname, 
		rapidjson::Value const& val);

	std::vector<unsigned char> ConvertFromHex(std::string const& in);

	std::vector<PatchType> patches_;
	Logger* logger_;

};

