#pragma once

#include <string>

namespace Act
{
	namespace Platform
	{
		void ErrorMessageBox(std::string msg);
	};

	namespace StringFormat
	{
		template <typename T, typename... TR>
		inline std::string Join(T first, TR... rest)
		{
			std::string ret;
			ret += first;
			ret += Join(rest...);
			return ret;
		}

		inline std::string Join()
		{
			return std::string();
		}

		template <typename... T>
		inline std::string Line(T... values)
		{
			return Join(values...) + "\n";
		}

		template <typename... T>
		inline std::string IntentLine(T... values)
		{
			return std::string("  ") + Line(values...);
		}

		template <typename... T>
		inline std::string JoinLines(T... values)
		{
			std::string ret = Join(values...);
			if (ret.back() == '\n')
			{
				ret.resize(ret.size() - 1);
			}
			return ret;
		}
	}

	class Task
	{
	public:
		Task() :
			Task(std::string())
		{
		}

		Task(std::string msg)
		{
			this->msg = msg;
			this->parent = GetLastTask();

			PushTask();
		}

		void Error(std::string msg)
		{
			using namespace StringFormat;
			Platform::ErrorMessageBox(JoinLines(
				Line("Error: ", msg),
				Line("In task: ", this->msg)
			));

			throw std::exception();
		}

		static void SendError(std::string msg)
		{
			GetLastTask()->Error(msg);
		}

	private:
		std::string msg;
		Task* parent;

	private:
		void PushTask()
		{
			GetStatic() = this;
		}

		void PopTask()
		{
			GetStatic() = parent;
		}

		static Task* GetLastTask()
		{
			return GetStatic();
		}

		static inline Task*& GetStatic()
		{
			static Task* val = nullptr;
			return val;
		}
	};
}
