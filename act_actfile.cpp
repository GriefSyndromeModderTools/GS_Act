#include "act_actfile.h"
#include "act_utils.h"

std::unique_ptr<Act::Object> Act::ReadActFromFile(AbstractInputStream s)
{
	static const std::uint32_t magic_act1 = 0x31544341; //"ACT1"
	try
	{
		Task task("ReadActFromFile");
		
		if (s->StrictMode())
		{
			std::uint32_t magic = (std::uint32_t) s->ReadInt32();
			if (magic != magic_act1)
			{
				task.Error("incorrect magic number");
			}

			std::int32_t unknown = s->ReadInt32();
			if (unknown != 1)
			{
				task.Error("incorrect file type");
			}

			std::int32_t offset = s->ReadInt32();
			if (offset != 0)
			{
				s->SkipBytes(offset);
			}
		}

		return Act::Object::ReadObject(s, ".?AVCAct@@");
	}
	catch (std::exception&)
	{
		return nullptr;
	}
}

std::unique_ptr<Act::McdFile> Act::ReadMcdFromFile(AbstractInputStream s)
{
	static const std::uint32_t magic_2dmc = 0x434D4432; //"2DMC"
	try
	{
		Task task("ReadMcdFromFile");

		if (s->StrictMode())
		{
			std::uint32_t magic = (std::uint32_t) s->ReadInt32();
			if (magic != magic_2dmc)
			{
				task.Error("incorrect magic number");
			}

			std::int32_t unknown = s->ReadInt32();
			if (unknown != 2)
			{
				task.Error("incorrect file type");
			}

			std::int32_t offset = s->ReadInt32();
			if (offset != 0)
			{
				task.Error("non-zero offset is not supported");
			}
		}

		std::unique_ptr<McdFile> ret = std::make_unique<McdFile>();

		ret->Read(s);
		return ret;
	}
	catch (int&)
	{
		return nullptr;
	}
}

void Act::WriteMcdToFile(std::shared_ptr<ISerializable> obj, AbstractOutputStream s)
{
	static const std::uint32_t magic_2dmc = 0x434D4432; //"2DMC"

	//header
	if (s->StrictMode())
	{
		s->WriteInt32("magic", magic_2dmc);
		s->WriteInt32("type", 2);
		s->WriteInt32("offset", 0);
	}

	obj->Write(s);
}

void Act::WriteActToFile(std::shared_ptr<ISerializable> obj, AbstractOutputStream s)
{
	static const std::uint32_t magic_act1 = 0x31544341; //"ACT1"

	//header
	if (s->StrictMode())
	{
		s->WriteInt32("magic", magic_act1);
		s->WriteInt32("type", 1);
		s->WriteInt32("offset", 0);
	}

	obj->Write(s);
}

void Act::McdFile::Read(Act::AbstractInputStream s)
{
	Task task("ReadMcd");
	
	std::int32_t count_data;
	std::int32_t size_data;
	if (s->StrictMode())
	{
		count_data = s->ReadInt32();
		size_data = s->ReadInt32();
		UnknownData.reserve(count_data);
	}
	else
	{
		size_data = 56;
	}

	if (size_data != 56)
	{
		task.Error("incorrect segment length");
	}
	s->ReadArray(std::string(), [&](){
		if (s->StrictMode())
		{
			for (int i = 0; i < count_data; ++i)
			{
				s->ReadObject(std::string(), [&](){
					McdFile::Segment seg;
					seg.Read(s);
					UnknownData.push_back(seg);
					s->ReadInt32();
				});
			}
		}
		else
		{
			while (!s->EndOfBlock())
			{
				s->ReadObject(std::string(), [&](){
					McdFile::Segment seg;
					seg.Read(s);
					UnknownData.push_back(seg);
				});
			}
		}
	});

	std::int32_t count_res;
	if (s->StrictMode())
	{
		count_res = s->ReadInt32();
		Resources.reserve(count_res);
	}
	s->ReadArray(std::string(), [&](){
		if (s->StrictMode())
		{
			for (int i = 0; i < count_res; ++i)
			{
				s->ReadObject(std::string(), [&](){
					Resources.emplace_back(Object::ReadObject(s));
				});
			}
		}
		else
		{
			while (!s->EndOfBlock())
			{
				s->ReadObject(std::string(), [&](){
					Resources.emplace_back(Object::ReadObject(s));
				});
			}
		}
	});
}

void Act::McdFile::Write(Act::AbstractOutputStream s)
{
	if (s->StrictMode())
	{
		s->WriteInt32("unknown_count", UnknownData.size());
		s->WriteInt32("unknown_size", 56);
	}
	s->WriteArray("chips", [&](){
		for (auto&& seg : UnknownData)
		{
			s->WriteObject("", [&](){
				seg.Write(s);
			});
			if (s->StrictMode())
			{
				s->WriteNextPosition();
			}
		}
	});

	if (s->StrictMode())
	{
		s->WriteInt32("resouce_count", Resources.size());
	}
	s->WriteArray("resources", [&](){
		for (auto&& res : Resources)
		{
			s->WriteObject("", [&](){
				s->WriteInt32("classid", res->GetClassID());
				res->Write(s);
			});
		}
	});
}
