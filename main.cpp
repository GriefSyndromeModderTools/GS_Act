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
#if 0
	std::string path = "E:\\Games\\[game]GRIEFSYNDROME\\griefsyndrome\\gs00\\data\\stage\\";
	std::string file = "stage1A.act";
	std::string output_path = "E:\\stage\\";
	
	Act::TextFileOptions::SetStrictMode(false);

	Act::AbstractInputStream input = Act::TextInputStream::Create(output_path + file + ".modified.txt");
	//Act::AbstractInputStream input = Act::BinaryInputStream::Create(path + file);
	Act::AbstractOutputStream output = Act::BinaryOutputStream::Create();
	//Act::AbstractOutputStream output = Act::TextOutputStream::Create();
	std::shared_ptr<Act::ISerializable> obj = Act::ReadActFromFile(input);

#if 0
	std::stringstream ss;
	for (auto&& seg : obj->UnknownData)
	{
		for (auto&& num : seg.short_array)
		{
			ss << num;
			ss << ", ";
		}
		ss << "\n";
	}
	Act::AbstractOutputStream dump = Act::TextOutputStream::Create();
	dump->WriteString("data", ss.str());
	dump->Save(output_path + file + ".dump.txt");
	return 0;
#endif
#if 0
	Act::AbstractOutputStream dump = Act::TextOutputStream::Create();
	dump->WriteString("data", dump_str.str());
	dump->Save(output_path + file + ".dump.txt");
	return 0;
#endif
#if 0
	auto copy_unknown = obj->UnknownData;
	int c = obj->UnknownData.size();
	c = 0; //7 8
	copy_unknown.resize(c);
	copy_unknown.push_back(obj->UnknownData[7]);
	copy_unknown.push_back(obj->UnknownData[58]);
	obj->UnknownData = copy_unknown;
	file += ".modified";
#endif
	WriteActToFile(obj, output);

	//output->Save("E:\\stage\\" + file + ".modified.txt.bin");
	output->Save(std::string("E:\\Games\\[game]GRIEFSYNDROME\\griefsyndrome\\gs03chiptest\\data\\stage\\") + file);
	//output->Save("E:\\stage\\" + file + ".check.dat");

	//output->Save("E:\\Games\\[game]GRIEFSYNDROME\\griefsyndrome\\gs03chiptest\\data\\stage\\stage1.mcd");
	return 0;
#endif
}
