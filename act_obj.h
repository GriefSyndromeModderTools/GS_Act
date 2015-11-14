#pragma once

#include "act_binfile.h"
#include "act_utils.h"

#include <map>

namespace Act
{
	struct ISerializable
	{
		virtual ~ISerializable() {}
		virtual void Read(AbstractInputStream s) = 0;
		virtual void Write(AbstractOutputStream s) = 0;
		//TODO write
	};

	struct PropertyTable : ISerializable
	{
		enum class ValueType
		{
			Integer = 0,
			Float = 1,
			Bool = 2,
			String = 3,
			Integer_Mcd = 4,
		};

		struct Value
		{
			std::string StringValue;
			union {
				std::int32_t IntegerValue;
				bool BoolValue;
				float FloatValue;
			};
		};

		struct Entry
		{
			std::string name;
			ValueType type;
			Value value;
		};

		virtual void Read(AbstractInputStream s) override
		{
			Task task("ReadPropertyTable");
			std::vector<Entry> entries;

			if (!s->StrictMode())
			{
				while (!s->EndOfBlock())
				{
					Entry entry;
					s->ReadObject(entry.name, [&](){
						entry.type = (ValueType) s->ReadInt32();
						switch (entry.type)
						{
						case ValueType::Integer:
						case ValueType::Integer_Mcd:
							entry.value.IntegerValue = s->ReadInt32();
							break;
						case ValueType::Float:
							entry.value.FloatValue = s->ReadFloat();
							break;
						case ValueType::Bool:
							entry.value.BoolValue = s->ReadBool();
							break;
						case ValueType::String:
							entry.value.StringValue = s->ReadString();
							break;
						}
					});

					entries.push_back(entry);
				}
			}
			else
			{
				if (!IsShortVer)
				{
					bool has_desc = s->ReadBool();
					if (!has_desc)
					{
						task.Error("property table without description");
					}
				}

				int size = s->ReadInt32();
				entries.reserve(size);

				s->ReadArray(std::string(), [&](){
					for (int i = 0; i < size; ++i)
					{
						Entry entry;
						s->ReadObject(std::string(), [&](){
							entry.name = s->ReadString();
							entry.type = (ValueType)s->ReadInt32();
						});
						entries.push_back(entry);
					}
				});

				s->ReadArray(std::string(), [&](){
					for (int i = 0; i < size; ++i)
					{
						s->ReadObject(std::string(), [&](){
							switch (entries[i].type)
							{
							case ValueType::Integer:
							case ValueType::Integer_Mcd:
								entries[i].value.IntegerValue = s->ReadInt32();
								break;
							case ValueType::Float:
								entries[i].value.FloatValue = s->ReadFloat();
								break;
							case ValueType::Bool:
								entries[i].value.BoolValue = s->ReadBool();
								break;
							case ValueType::String:
								entries[i].value.StringValue = s->ReadString();
								break;
							default:
								task.Error("unknown value type in property table");
							}
						});
					}
				});
			}

			this->Entries = std::move(entries);
		}
		
		virtual void Write(AbstractOutputStream s) override
		{
			if (!s->StrictMode())
			{
				for (auto&& entry : Entries)
				{
					s->WriteObject(entry.name, [&]() {
						s->WriteInt32("type", (int) entry.type);
						switch (entry.type)
						{
						case ValueType::Integer:
						case ValueType::Integer_Mcd:
							s->WriteInt32("value", entry.value.IntegerValue);
							break;
						case ValueType::Float:
							s->WriteFloat("value", entry.value.FloatValue);
							break;
						case ValueType::Bool:
							s->WriteBool("value", entry.value.BoolValue);
							break;
						case ValueType::String:
							s->WriteString("value", entry.value.StringValue);
							break;
						}
					});
				}
			}
			else
			{
				if (!IsShortVer)
				{
					s->WriteBool("has_header", true);
				}
				s->WriteInt32("count", Entries.size());
				s->WriteArray("header", [&](){
					for (auto&& entry : Entries)
					{
						s->WriteObject("", [&](){
							s->WriteString("name", entry.name);
							s->WriteInt32("type", (int) entry.type);
						});
					}
				});
				s->WriteArray("values", [&](){
					for (auto&& entry : Entries)
					{
						s->WriteObject("", [&](){
							switch (entry.type)
							{
							case ValueType::Integer:
							case ValueType::Integer_Mcd:
								s->WriteInt32("value", entry.value.IntegerValue);
								break;
							case ValueType::Float:
								s->WriteFloat("value", entry.value.FloatValue);
								break;
							case ValueType::Bool:
								s->WriteBool("value", entry.value.BoolValue);
								break;
							case ValueType::String:
								s->WriteString("value", entry.value.StringValue);
								break;
							}
						});
					}
				});
			}
		}

		void UseShortVer()
		{
			IsShortVer = true;
		}

		std::vector<Entry> Entries;

	private:
		bool IsShortVer = false;
	};

	struct Code : ISerializable
	{
		virtual void Read(AbstractInputStream s) override
		{
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
			CodeStr = s->ReadString();
		}

		virtual void Write(AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});

			std::string write_code = CodeStr;
			if (s->StrictMode())
			{
				write_code += '\0';
			}
			s->WriteString("code", write_code);
		}

		PropertyTable Properties;
		std::string CodeStr;
	};

	class Object : public ISerializable
	{
	public:
		Object(const char* classid) :
			ClassID(classid)
		{
		}

		virtual ~Object() {}

		std::uint32_t GetClassID()
		{
			return CalcHash(ClassID);
		}

	public:
		static std::unique_ptr<Object> ReadObject(AbstractInputStream s, std::uint32_t type);

		static std::unique_ptr<Object> ReadObject(AbstractInputStream s, std::string type)
		{
			return ReadObject(s, CalcHash(type.c_str()));
		}

		static std::unique_ptr<Object> ReadObject(AbstractInputStream s)
		{
			std::uint32_t type = (std::uint32_t) s->ReadInt32();
			return ReadObject(s, type);
		}

	public:
		static std::uint32_t CalcHash(const char* str)
		{
			const char* p = str;
			std::uint32_t hash = 0;
			while (*p)
			{
				hash ^= (*(p++) + (hash << 6) + (hash >> 2) + 0x9E3779B9);
			}
			return hash;
		}

	public:
		PropertyTable Properties;

	private:
		const char* ClassID;
	};
}
