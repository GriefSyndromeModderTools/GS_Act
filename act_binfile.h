#pragma once
#include "act_file.h"
#include "act_utils.h"

#include <vector>
#include <cinttypes>
#include <string>
#include <memory>

namespace Act
{
	class BinaryInputFile final : public AbstractInputFile
	{
	public:
		BinaryInputFile(std::vector<std::uint8_t>& data) :
			data(std::move(data)),
			pos(0)
		{
		}

		BinaryInputFile(std::string filename) :
			BinaryInputFile(OpenFile(filename))
		{
		}

	private:
		std::vector<std::uint8_t> ReadBytes(std::size_t count)
		{
			std::vector<std::uint8_t> ret(data.begin() + pos, data.begin() + pos + count);
			pos += count;
			return ret;
		}

		template <std::size_t Size>
		ByteBlock<Size> ReadBytes()
		{
			ByteBlock<Size> ret;
			ret.Fill(data.begin() + pos);
			pos += Size;
			return ret;
		}

		void ReadAny()
		{
			if (block_rest) --block_rest;
		}

	public:
		virtual std::int32_t ReadInt32(std::string& name = std::string()) override
		{
			ReadAny();
			return ReadBytes<4>().GetValue<std::int32_t>();
		}

		virtual std::int16_t ReadInt16(std::string& name = std::string()) override
		{
			ReadAny();
			return ReadBytes<2>().GetValue<std::int16_t>();
		}

		virtual bool ReadBool(std::string& name = std::string()) override
		{
			ReadAny();
			return ReadBytes<1>().GetValue<bool>();
		}

		virtual std::string ReadString(std::string& name = std::string()) override
		{
			ReadAny();
			int size = ReadBytes<4>().GetValue<std::int32_t>();
			std::vector<std::uint8_t> data = ReadBytes(size);
			char* raw = (char*) data.data();
			return std::string(raw, size);
		}

		virtual float ReadFloat(std::string& name = std::string()) override
		{
			ReadAny();
			return ReadBytes<4>().GetValue<float>();
		}

		virtual void ReadObject(std::string& name, Act::Reader r) override
		{
			ReadAny();
			auto last_rest = block_rest;
			block_rest = block_next_size;
			block_next_size = 0;
			r();
			block_rest = last_rest;
		}

		virtual void ReadArray(std::string& name, Reader r) override
		{
			ReadAny();
			auto last_rest = block_rest;
			block_rest = block_next_size;
			block_next_size = 0;
			r();
			block_rest = last_rest;
		}

		virtual bool EndOfBlock() override
		{
			//Task::SendError("try to get structure on binary file");
			//return false;
			return block_rest == 0;
		}

		virtual void SetBlockSize(std::int32_t size) override
		{
			block_next_size = size;
		}

		virtual void SkipBytes(size_t size) override
		{
			pos += size;
		}

		virtual bool StrictMode() override
		{
			return true;
		}

	private:
		std::vector<std::uint8_t> data;
		std::size_t pos;
		std::int32_t block_rest = 0;
		std::int32_t block_next_size = 0;
	};

	struct BinaryInputStream
	{
		template <typename... T>
		static AbstractInputStream Create(T&&... args)
		{
			return std::make_shared<BinaryInputFile>(std::forward<T>(args)...);
		}
	};

	class BinaryOutputFile final : public AbstractOutputFile
	{
	public:
		virtual void WriteInt32(std::string name, std::int32_t val) override
		{
			WriteBytes(&val, 4);
		}

		virtual void WriteInt16(std::string name, std::int16_t val) override
		{
			WriteBytes(&val, 2);
		}

		virtual void WriteFloat(std::string name, float val) override
		{
			WriteBytes(&val, 4);
		}

		virtual void WriteBool(std::string name, bool val) override
		{
			char data = val ? 1 : 0;
			WriteBytes(&data, 1);
		}

		virtual void WriteString(std::string name, std::string val) override
		{
			WriteInt32("len", val.size());
			WriteBytes(val.c_str(), val.size());
		}

		virtual void WriteObject(std::string name, Writer writer) override
		{
			writer();
		}

		virtual void WriteArray(std::string name, Writer writer) override
		{
			writer();
		}

		virtual void WriteNextPosition() override
		{
			WriteInt32("pos", data.size() + 4);
		}

		virtual bool StrictMode() override
		{
			return true;
		}

		virtual void Save(std::string filename) override
		{
			FILE* file_out = fopen(filename.c_str(), "wb");
			fwrite(data.data(), 1, data.size(), file_out);
			fclose(file_out);
		}

	private:
		void WriteBytes(const void* p, int size)
		{
			int pos = data.size();
			data.resize(pos + size);
			memcpy(data.data() + pos, p, size);
		}

	private:
		std::vector<std::uint8_t> data;
	};

	struct BinaryOutputStream
	{
		static AbstractOutputStream Create()
		{
			return AbstractOutputStream(new BinaryOutputFile());
		}
	};
}
