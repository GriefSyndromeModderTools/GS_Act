#pragma once
#include "act_file.h"
#include "act_utils.h"

#include <sstream>
#include <memory>

namespace Act
{
	struct TextFileOptions
	{
		static void SetStrictMode(bool value)
		{
			StrictModeValue() = value;
		}

		static bool GetStrictMode()
		{
			return StrictModeValue();
		}

	private:
		static inline bool& StrictModeValue()
		{
			static bool val = false;
			return val;
		}
	};

	class TextOutputFile final : public AbstractOutputFile
	{
		virtual void WriteInt32(std::string name, std::int32_t val) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			std::stringstream ss;
			ss << val;
			value += ss.str();
			SetComma();
		}

		virtual void WriteInt16(std::string name, std::int16_t val) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			std::stringstream ss;
			ss << val;
			value += ss.str();
			SetComma();
		}

		virtual void WriteFloat(std::string name, float val) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			std::stringstream ss;
			ss << val;
			value += ss.str();
			SetComma();
		}

		virtual void WriteBool(std::string name, bool val) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			value += (val ? "true" : "false");
			SetComma();
		}

		virtual void WriteString(std::string name, std::string val) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			value += "\"";
			value += MakeEscape(std::string(val.c_str())) + "\""; //remove zeros
			SetComma();
		}

		virtual void WriteObject(std::string name, Writer writer) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			value += "{";
			++indent;
			bool original_is_in_array = is_in_array;
			is_in_array = false;
			writer();
			is_in_array = original_is_in_array;
			--indent;
			NewLine();
			value += "}";
			SetComma();
		}

		virtual void WriteArray(std::string name, Writer writer) override
		{
			CheckComma();
			NewLine();
			WriteLable(name);
			value += "[";
			++indent;
			bool original_is_in_array = is_in_array;
			is_in_array = true;
			writer();
			is_in_array = original_is_in_array;
			--indent;
			NewLine();
			value += "]";
			SetComma();
		}

		virtual void WriteNextPosition() override
		{
		}

		virtual bool StrictMode() override
		{
			return TextFileOptions::GetStrictMode();
		}

		void Save(std::string filename) override
		{
			FILE* file_out = fopen(filename.c_str(), "wb");
			fwrite(value.c_str(), 1, value.length(), file_out);
			fclose(file_out);
		}

	private:
		static std::string MakeEscape(std::string str)
		{
			std::stringstream ss;
			for (char c : str)
			{
				switch (c)
				{
				case '\\':
					ss << "\\\\";
					break;
				case '"':
					ss << "\\\"";
					break;
				default:
					ss << c;
				}
			}
			return ss.str();
		}

	private:
		bool append_comma = false;
		int indent = 0;
		std::string value;
		bool is_in_array = false;

		void ClearComma()
		{
			append_comma = false;
		}

		void CheckComma()
		{
			if (append_comma) value += ",";
			ClearComma();
		}

		void SetComma()
		{
			append_comma = true;
		}

		void NewLine()
		{
			value += "\n";
			value += std::string(indent * 2, ' ');
		}

		void WriteLable(std::string str)
		{
			if (is_in_array) return;
			value += str + ": ";
		}
	};

	struct TextOutputStream
	{
		static AbstractOutputStream Create()
		{
			return AbstractOutputStream(new TextOutputFile());
		}
	};

	class TextInputFile final : public AbstractInputFile
	{
	public:
		TextInputFile(std::string filename)
		{
			auto vec = OpenFile(filename);
			data = (char*) OpenFile(filename).data();
			pos = 0;
			current_is_array = false;
		}

	public:
		virtual std::int32_t ReadInt32(std::string& name = std::string()) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			return (std::int32_t) ReadNumber();
		}

		virtual std::int16_t ReadInt16(std::string& name = std::string()) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			return (std::int16_t) ReadNumber();
		}

		virtual float ReadFloat(std::string& name = std::string()) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			return (float) ReadNumber();
		}

		virtual bool ReadBool(std::string& name = std::string()) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			return ReadWord() == "true";
		}

		virtual std::string ReadString(std::string& name = std::string()) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			return ReadQuoteString();
		}

		virtual void ReadObject(std::string& name, Reader r) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			Skip('{');
			bool last_is_array = current_is_array;
			current_is_array = false;
			r();
			current_is_array = last_is_array;
			Skip('}');
		}

		virtual void ReadArray(std::string& name, Reader r) override
		{
			CheckEndOfBlock();
			ReadLabel(name);
			Skip('[');
			bool last_is_array = current_is_array;
			current_is_array = true;
			r();
			current_is_array = last_is_array;
			Skip(']');
		}

		virtual bool EndOfBlock() override
		{
			char c = data[PeekChar()];
			if (c == 0 || c == ']' || c == '}')
			{
				return true;
			}
			return false;
		}

		virtual void SetBlockSize(std::int32_t size) override
		{
		}

		virtual void SkipBytes(size_t size) override
		{
			if (size != 0)
			{
				Task::SendError("invalid operation: skip bytes in text file");
			}
		}

		virtual bool StrictMode() override
		{
			return TextFileOptions::GetStrictMode();
		}

	private:
		void ReadLabel(std::string& name)
		{
			if (current_is_array) return;
			name = ReadWord();
			Skip(':');
		}

		void CheckEndOfBlock()
		{
			TrySkip(',');
			if (EndOfBlock())
			{
				std::string dump = data.substr(pos, 3);
				Task::SendError("end of block");
			}
		}

		double ReadNumber()
		{
			std::stringstream ss(ReadWord());
			double ret;
			ss >> ret;
			return ret;
		}

		std::string ReadWord()
		{
			int p = PeekChar();
			int p_start = p;
			if (!IsAlphaDig(data[p]))
			{
				std::string dump = data.substr(p, 100);
				Task::SendError("expecting word");
			}
			do
			{
				++p;
				if (CheckEndOfString(p))
				{
					break;
				}
			} while (IsAlphaDig(data[p]));
			pos = p;
			return data.substr(p_start, p - p_start);
		}

		std::string Dump(int i)
		{
			return data.substr(i,50);
		}
		std::string ReadQuoteString()
		{
			Skip('"');
			int p = pos;
			int p_start = p;
			std::stringstream ss;
			while (1) {
				switch (data[p])
				{
				case '"':
					pos = p + 1; //skip '"'
					return ss.str();
				case '\\':
					Dump(10);
					++p;
					if (CheckEndOfString(p))
					{
						Task::SendError("unfinished escape character");
					}
					ss << data[p];
					break;
				default:
					ss << data[p];
				}
				++p;
				if (CheckEndOfString(p))
				{
					Task::SendError("unfinished string value");
				}
			}
		}

		int PeekChar()
		{
			int p = pos;
			while (IsSpaceChar(data[p]))
			{
				++p;
			}
			return p;
		}

		bool CheckEndOfString(std::size_t p)
		{
			return p >= data.length();
		}

		bool TrySkip(char c)
		{
			int p = PeekChar();
			if (data[p] == c)
			{
				pos = p + 1;
				return true;
			}
			return false;
		}

		void Skip(char c)
		{
			if (!TrySkip(c))
			{
				std::string dump = data.substr(pos, 100);
				Task::SendError("expecting " + c);
			}
		}

	private:
		bool IsSpaceChar(char c)
		{
			return c == ' ' || c == '\t' || c == '\n' || c == '\r';
		}

		bool IsAlphaDig(char c)
		{
			return (c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z') ||
				(c >= '0' && c <= '9') ||
				c == '_' || //other chars in str
				c == '.' || c == '+' || c == '-'; //also allow number characters
		}
	private:
		std::string data;
		std::size_t pos;
		bool current_is_array;
	};

	struct TextInputStream
	{
		template <typename... T>
		static AbstractInputStream Create(T&&... args)
		{
			return std::make_shared<TextInputFile>(std::forward<T>(args)...);
		}
	};
}
