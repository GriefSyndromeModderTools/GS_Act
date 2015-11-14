#pragma once
#include "act_obj.h"
#include "act_binfile.h"

namespace Act
{
	struct McdFile : ISerializable
	{
		struct Segment : ISerializable
		{
			//union {
				//struct
				//{
				//	std::uint32_t unknown00;
				//	std::uint32_t unknown01;
				//	std::uint32_t unknown02;
				//	std::uint32_t unknown03;
				//	std::uint32_t unknown04;
				//	std::uint32_t unknown05;
				//	std::uint32_t unknown06;
				//	std::uint32_t unknown07;
				//	std::uint32_t unknown08;
				//	std::uint32_t unknown09;
				//	std::uint32_t unknown10;
				//	std::uint32_t unknown11;
				//	std::uint32_t unknown12;
				//	std::uint32_t unknown13;
				//};
				//std::int16_t short_array[28];
				std::uint32_t chip_id;
				std::uint32_t res_id;
				std::int16_t src_x;
				std::int16_t src_y;
				std::int16_t width;
				std::int16_t height;
				std::int16_t collision_flag;
				std::int16_t collision_x;
				std::int16_t collision_y;
				std::int16_t collision_w;
				std::int16_t collision_h;
			//};

			void ReadZero16(Act::AbstractInputStream s)
			{
				if (s->ReadInt16() != 0)
				{
					Task::SendError("expecting 16-bit 0");
				}
			}

			virtual void Read(Act::AbstractInputStream s) override
			{
				chip_id = s->ReadInt32();
				res_id = s->ReadInt32();
				src_x = s->ReadInt16();
				src_y = s->ReadInt16();
				width = s->ReadInt16();
				height = s->ReadInt16();
				if (s->StrictMode())
				{
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
				}
				collision_flag = s->ReadInt16();
				collision_x = s->ReadInt16();
				collision_y = s->ReadInt16();
				collision_w = s->ReadInt16();
				collision_h = s->ReadInt16();
				if (s->StrictMode())
				{
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
					ReadZero16(s);
				}
				//unknown00 = s->ReadInt32();
				//unknown01 = s->ReadInt32();
				//unknown02 = s->ReadInt32();
				//unknown03 = s->ReadInt32();
				//unknown04 = s->ReadInt32();
				//unknown05 = s->ReadInt32();
				//unknown06 = s->ReadInt32();
				//unknown07 = s->ReadInt32();
				//unknown08 = s->ReadInt32();
				//unknown09 = s->ReadInt32();
				//unknown10 = s->ReadInt32();
				//unknown11 = s->ReadInt32();
				//unknown12 = s->ReadInt32();
				//unknown13 = s->ReadInt32();
			}

			virtual void Write(Act::AbstractOutputStream s) override
			{
				s->WriteInt32("chip_id", chip_id);
				s->WriteInt32("res_id", res_id);
				s->WriteInt16("src_x", src_x);
				s->WriteInt16("src_y", src_y);
				s->WriteInt16("width", width);
				s->WriteInt16("height", height);
				if (s->StrictMode())
				{
					for (int i = 0; i < 9; ++i)
						s->WriteInt16("zero", 0);
				}
				s->WriteInt16("collision_flag", collision_flag);
				s->WriteInt16("collision_x", collision_x);
				s->WriteInt16("collision_y", collision_y);
				s->WriteInt16("collision_w", collision_w);
				s->WriteInt16("collision_h", collision_h);
				if (s->StrictMode())
				{
					for (int i = 0; i < 6; ++i)
						s->WriteInt16("zero", 0);
				}
				//s->WriteInt32("unknown00", unknown00);
				//s->WriteInt32("unknown01", unknown01);
				//s->WriteInt32("unknown02", unknown02);
				//s->WriteInt32("unknown03", unknown03);
				//s->WriteInt32("unknown04", unknown04);
				//s->WriteInt32("unknown05", unknown05);
				//s->WriteInt32("unknown06", unknown06);
				//s->WriteInt32("unknown07", unknown07);
				//s->WriteInt32("unknown08", unknown08);
				//s->WriteInt32("unknown09", unknown09);
				//s->WriteInt32("unknown10", unknown10);
				//s->WriteInt32("unknown11", unknown11);
				//s->WriteInt32("unknown12", unknown12);
				//s->WriteInt32("unknown13", unknown13);
			}
		};

		std::vector<Segment> UnknownData;
		std::vector<std::unique_ptr<Object>> Resources;
		virtual void Read(Act::AbstractInputStream s) override;
		virtual void Write(Act::AbstractOutputStream s) override;
	};

	std::unique_ptr<Object> ReadActFromFile(AbstractInputStream s);
	std::unique_ptr<McdFile> ReadMcdFromFile(AbstractInputStream s);

	void WriteMcdToFile(std::shared_ptr<ISerializable> obj, AbstractOutputStream s);
	void WriteActToFile(std::shared_ptr<ISerializable> obj, AbstractOutputStream s);
}
