#include "act_binfile.h"
#include "act_obj.h"
#include "act_actfile.h"
#include "act_textfile.h"
#include "act_cmd.h"
#include <sstream>
#include <iostream>

std::stringstream dump_str;

static void PrintHelpMessage()
{
	std::cout <<
		"Options: \n"
		"  -h         Show help message.\n"
		"  -a <file>  Binary act to text act.\n"
		"  -A <file>  Text act to binary act.\n"
		"  -m <file>  Binary mcd to text mcd.\n"
		"  -M <file>  Text mcd to binary act.\n"
		;
}

static void Error()
{
	std::cout << "Invalid argument." << std::endl;
	PrintHelpMessage();
}

int main(int argc, char *argv[])
{
	std::cout << "GS_Act, griefsyndrome act and mcd file converter. By acaly." << std::endl;
	Act::CommandLine cmd(argc - 1, argv + 1);
	if (!cmd.Process())
	{
		PrintHelpMessage();
		return 1;
	}

	int file_type = 0;

	if (cmd.HasOption('h'))
	{
		PrintHelpMessage();
		return 0;
	}
	if (cmd.HasOption('a'))
	{
		if (file_type) Error();
		Act::AbstractInputStream input = Act::BinaryInputStream::Create(cmd.GetOption('a'));
		std::shared_ptr<Act::ISerializable> obj = Act::ReadActFromFile(input);
		Act::AbstractOutputStream output = Act::TextOutputStream::Create();
		Act::WriteActToFile(obj, output);
		output->Save(cmd.GetOption('a') + ".txt");
		file_type = 1;
	}
	if (cmd.HasOption('A'))
	{
		if (file_type) Error();
		Act::AbstractInputStream input = Act::TextInputStream::Create(cmd.GetOption('A'));
		std::shared_ptr<Act::ISerializable> obj = Act::ReadActFromFile(input);
		Act::AbstractOutputStream output = Act::BinaryOutputStream::Create();
		Act::WriteActToFile(obj, output);
		output->Save(cmd.GetOption('A') + ".bin");
		file_type = 1;
	}
	if (cmd.HasOption('m'))
	{
		if (file_type) Error();
		Act::AbstractInputStream input = Act::BinaryInputStream::Create(cmd.GetOption('m'));
		std::shared_ptr<Act::ISerializable> obj = Act::ReadMcdFromFile(input);
		Act::AbstractOutputStream output = Act::TextOutputStream::Create();
		Act::WriteMcdToFile(obj, output);
		output->Save(cmd.GetOption('m') + ".txt");
		file_type = 2;
	}
	if (cmd.HasOption('M'))
	{
		if (file_type) Error();
		Act::AbstractInputStream input = Act::TextInputStream::Create(cmd.GetOption('M'));
		std::shared_ptr<Act::ISerializable> obj = Act::ReadMcdFromFile(input);
		Act::AbstractOutputStream output = Act::BinaryOutputStream::Create();
		Act::WriteMcdToFile(obj, output);
		output->Save(cmd.GetOption('M') + ".bin");
		file_type = 2;
	}
	if (file_type == 0)
	{
		PrintHelpMessage();
		return 0;
	}
	return 0;
}
