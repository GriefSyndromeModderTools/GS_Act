#pragma once

#include <string>
#include <map>
#include <cctype>

namespace Act
{
	class CommandLine
	{
	public:
		CommandLine(int argc, char *argv[]) :
			argc(argc), argv(argv)
		{
		}

		bool Process()
		{
			for (int i = 0; i < argc; ++i)
			{
				char* str = argv[i];
				if (str[0] == '-')
				{
					if (!isalpha(str[1]) || str[2] != 0)
					{
						return false;
					}
					++i;
					if (i == argc)
					{
						return false;
					}
					data[str[1]] = std::string(argv[i]);
				}
			}
			return true;
		}

		bool HasOption(char c)
		{
			return data.find(c) != data.end();
		}

		std::string GetOption(char c)
		{
			if (!HasOption(c))
			{
				return "";
			}
			return data[c];
		}

	private:
		int argc;
		char** argv;
		std::map<char, std::string> data;
	};
}
