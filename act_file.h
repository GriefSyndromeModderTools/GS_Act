#pragma once
#include <vector>
#include <cinttypes>
#include <functional>
#include <memory>

namespace Act
{
	template <typename T>
	struct DataConvert
	{
		static const std::size_t Size = sizeof(T);

		static T FromBytes(std::uint8_t* data)
		{
			return *(T*)data;
		}

		static void ToBytes(std::uint8_t* dest, T val)
		{
			T* dest_cv = (T*)dest;
			*dest = val;
		}

	};

	template <>
	struct DataConvert<bool>
	{
		static const std::size_t Size = 1;

		template <typename T>
		static bool FromBytes(T* data)
		{
			return *data != 0;
		}

		template <typename T>
		static void ToBytes(T* dest, bool val)
		{
			*dest = val ? 1 : 0;
		}
	};

	template <std::size_t Size>
	struct ByteBlock
	{
		std::uint8_t data[Size];

		template <typename Itor>
		void Fill(Itor&& itor)
		{
			std::remove_reference_t<Itor> itor_cp = itor;
			for (int i = 0; i < Size; ++i)
			{
				data[i] = *itor_cp;
				++itor_cp;
			}
		}

		template <typename T>
		T GetValue()
		{
			static_assert(Size == DataConvert<T>::Size, "Data size incorrect.");
			return DataConvert<T>::FromBytes(data);
		}
	};

	typedef std::function<void()> Writer;

	class AbstractOutputFile
	{
	public:
		virtual ~AbstractOutputFile() {}

		virtual void WriteInt32(std::string name, std::int32_t val) = 0;
		virtual void WriteInt16(std::string name, std::int16_t val) = 0;
		virtual void WriteFloat(std::string name, float val) = 0;
		virtual void WriteBool(std::string name, bool val) = 0;
		virtual void WriteString(std::string name, std::string val) = 0;

		virtual void WriteObject(std::string name, Writer writer) = 0;
		virtual void WriteArray(std::string name, Writer writer) = 0;

		virtual void WriteNextPosition() = 0;

		virtual bool StrictMode() = 0;

		virtual void Save(std::string filename) = 0;
	};

	typedef std::shared_ptr<AbstractOutputFile> AbstractOutputStream;

	typedef std::function<void()> Reader;

	class AbstractInputFile
	{
	public:
		virtual ~AbstractInputFile() {}
		virtual std::int32_t ReadInt32(std::string& name = std::string()) = 0;
		virtual std::int16_t ReadInt16(std::string& name = std::string()) = 0;
		virtual float ReadFloat(std::string& name = std::string()) = 0;
		virtual bool ReadBool(std::string& name = std::string()) = 0;
		virtual std::string ReadString(std::string& name = std::string()) = 0;

		virtual void ReadObject(std::string& name, Reader r) = 0;
		virtual void ReadArray(std::string& name, Reader r) = 0;

		virtual bool EndOfBlock() = 0;
		virtual void SetBlockSize(std::int32_t size) = 0;

		virtual void SkipBytes(size_t size) = 0;

		virtual bool StrictMode() = 0;
	};

	typedef std::shared_ptr<AbstractInputFile> AbstractInputStream;

	inline std::vector<std::uint8_t> OpenFile(std::string filename)
	{
		FILE* file_in = fopen(filename.c_str(), "rb");

		std::vector<std::uint8_t> ret;

		fseek(file_in, 0, SEEK_END);
		std::size_t len = ftell(file_in);
		ret.resize(len);
		fseek(file_in, 0, SEEK_SET);

		fread(ret.data(), 1, len, file_in);
		fclose(file_in);

		return ret;
	}
}
