#include "act_obj.h"
#include "act_utils.h"
#include "act_file.h"

#include <map>
#include <sstream>

extern std::stringstream dump_str;

namespace
{
	struct CAct : Act::Object
	{
		CAct() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVCAct@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			Act::Task task("ReadAct");

			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});

			s->ReadObject(std::string(), [&](){
				CodeObj.Read(s);
			});

			int size_layers;
			if (s->StrictMode())
			{
				size_layers = s->ReadInt32();
				Layers.reserve(size_layers);
				s->SetBlockSize(size_layers);
			}
			s->ReadArray(std::string(), [&](){
				while (!s->EndOfBlock())
				{
					s->ReadObject(std::string(), [&](){
						Layers.emplace_back(Act::Object::ReadObject(s));
					});
				}
			});

			int size_res;
			if (s->StrictMode())
			{
				size_res = s->ReadInt32();
				Resources.reserve(size_res);
				s->SetBlockSize(size_res);
			}
			s->ReadArray(std::string(), [&](){
				while (!s->EndOfBlock())
				{
					s->ReadObject(std::string(), [&](){
						Resources.emplace_back(Act::Object::ReadObject(s));
					});
				}
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});

			s->WriteObject("code", [&](){
				CodeObj.Write(s);
			});

			if (s->StrictMode())
			{
				s->WriteInt32("layer_count", Layers.size());
			}
			s->WriteArray("layers", [&](){
				for (auto&& l : Layers)
				{
					s->WriteObject("", [&](){
						s->WriteInt32("classid", l->GetClassID());
						l->Write(s);
					});
				}
			});

			if (s->StrictMode())
			{
				s->WriteInt32("resource_count", Resources.size());
			}
			s->WriteArray("resources", [&](){
				for (auto&& r : Resources)
				{
					s->WriteObject("", [&](){
						s->WriteInt32("classid", r->GetClassID());
						r->Write(s);
					});
				}
			});
		}

		Act::Code CodeObj;
		std::vector<std::unique_ptr<Act::Object>> Layers;
		std::vector<std::unique_ptr<Act::Object>> Resources;
	};

	struct CActLayer : Act::Object
	{
		CActLayer() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVCActLayer@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			Act::Task task("ReadActLayer");

			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
			
			if (s->StrictMode())
			{
				int size_keys = s->ReadInt32();
				Keys.reserve(size_keys);
				s->SetBlockSize(size_keys);
			}
			s->ReadArray(std::string(), [&](){
				while (!s->EndOfBlock())
				{
					s->ReadObject(std::string(), [&](){
						Keys.emplace_back(Act::Object::ReadObject(s));
					});
				}
			});

			int size_unknown = s->ReadInt32();
			if (size_unknown != 0)
			{
				task.Error("unknown int32 in ActLayer should be 0");
			}

			s->ReadObject(std::string(), [&](){
				CodeObj.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});

			if (s->StrictMode())
			{
				s->WriteInt32("key_count", Keys.size());
			}
			s->WriteArray("keys", [&](){
				for (auto&& k : Keys)
				{
					s->WriteObject("", [&](){
						s->WriteInt32("classid", k->GetClassID());
						k->Write(s);
					});
				}
			});

			s->WriteInt32("unknown", 0);

			s->WriteObject("code", [&](){
				CodeObj.Write(s);
			});
		}

		std::vector<std::unique_ptr<Act::Object>> Keys;
		Act::Code CodeObj;
	};

	struct CActKey : Act::Object
	{
		CActKey() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVCActKey@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			Act::Task task("ReadActKey");

			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});

			bool has_layout = s->ReadBool();
			if (!has_layout)
			{
				LayoutObj = nullptr;
				return;
			}

			s->ReadObject(std::string(), [&](){
				LayoutObj = Act::Object::ReadObject(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
			if (!LayoutObj)
			{
				s->WriteBool("hasLayout", false);
				return;
			}
			s->WriteBool("hasLayout", true);
			s->WriteObject("layout", [&](){
				s->WriteInt32("classid", LayoutObj->GetClassID());
				LayoutObj->Write(s);
			});
		}

		std::unique_ptr<Act::Object> LayoutObj;
	};

	struct CAct2DMapLayout : Act::Object
	{
		CAct2DMapLayout() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVC2DMapLayout@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			Act::Task task("ReadAct2DMapLayout");

			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});

			int count, size;
			if (s->StrictMode())
			{
				count = s->ReadInt32();
				size = s->ReadInt32();
				Data.reserve(count);
				s->SetBlockSize(count);
			}
			else
			{
				size = s->ReadBool() ? 24 : 40;
			}

			if (size != 40 && size != 24)
			{
				task.Error("incorrect segment size");
			}
			IsShortVer = (size == 24);

			s->ReadArray(std::string(), [&](){
				while (!s->EndOfBlock())
				{
					s->ReadArray(std::string(), [&](){
						Segment seg(IsShortVer);
						seg.Read(s);
						Data.push_back(seg);
					});
				}
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});

			if (s->StrictMode())
			{
				s->WriteInt32("segment_count", Data.size());
				s->WriteInt32("segment_size", IsShortVer ? 24 : 40);
			}
			else
			{
				s->WriteBool("is_short", IsShortVer);
			}
			s->WriteArray("segments", [&](){
				for (auto&& seg : Data)
				{
					s->WriteArray("", [&](){
						seg.Write(s);
					});
				}
			});
		}

		struct Segment : Act::ISerializable {
			std::int32_t unknown0;
			std::int32_t unknown1;
			std::int32_t unknown2;
			float unknown3;
			float unknown4;
			float unknown5;
			std::int32_t unknown6;
			std::int32_t unknown7;
			std::int32_t unknown8;
			std::int32_t unknown9;

			bool short_ver;

			Segment(bool short_ver) :
				short_ver(short_ver)
			{
			}

			virtual void Read(Act::AbstractInputStream s) override
			{
				unknown0 = s->ReadInt32();
				unknown1 = s->ReadInt32();
				unknown2 = s->ReadInt32();
				unknown3 = s->ReadFloat();
				unknown4 = s->ReadFloat();
				unknown5 = s->ReadFloat();
				if (!short_ver)
				{
					unknown6 = s->ReadInt32();
					unknown7 = s->ReadInt32();
					unknown8 = s->ReadInt32();
					unknown9 = s->ReadInt32();
				}

				dump_str << unknown0 << "\t" << unknown1 << "\t" << unknown2 << "\t" << unknown5 << "\t"
					<< unknown6 << "\t" << unknown7 << "\t" << unknown8 << "\t" << unknown9 << "\t\n";
			}

			virtual void Write(Act::AbstractOutputStream s) override
			{
				s->WriteInt32("unknown0", unknown0);
				s->WriteInt32("unknown1", unknown1);
				s->WriteInt32("unknown2", unknown2);
				s->WriteFloat("unknown3", unknown3);
				s->WriteFloat("unknown4", unknown4);
				s->WriteFloat("unknown5", unknown5);
				if (!short_ver)
				{
					s->WriteInt32("unknown6", unknown6);
					s->WriteInt32("unknown7", unknown7);
					s->WriteInt32("unknown8", unknown8);
					s->WriteInt32("unknown9", unknown9);
				}
			}
		} ;

		std::vector<Segment> Data;
		bool IsShortVer = false;
	};

	struct ActResourceChip : Act::Object
	{
		ActResourceChip() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVCActResourceChip@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
		}
	};

	struct ActTextureResourceInfo : Act::Object
	{
		ActTextureResourceInfo() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVTextureResourceInfo@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			Properties.UseShortVer();
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
		}
	};

	struct MeshResourceInfo : Act::Object
	{
		MeshResourceInfo() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVMeshResourceInfo@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			Properties.UseShortVer();
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
		}
	};

	struct C2DLayout : Act::Object
	{
		C2DLayout() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVC2DLayout@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
		}
	};

	struct CActResource2D : Act::Object
	{
		CActResource2D() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVCActResource2D@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
		}
	};

	struct CActRenderTarget : Act::Object
	{
		CActRenderTarget() : Object(GetID()) {}

		static const char* GetID()
		{
			return ".?AVCActRenderTarget@@";
		}

		virtual void Read(Act::AbstractInputStream s) override
		{
			s->ReadObject(std::string(), [&](){
				Properties.Read(s);
			});
		}

		virtual void Write(Act::AbstractOutputStream s) override
		{
			s->WriteObject("properties", [&](){
				Properties.Write(s);
			});
		}
	};
}

namespace
{
	typedef std::unique_ptr<Act::Object>(*ObjectCreator)();

	template <typename T>
	typename std::map<std::uint32_t, ObjectCreator>::value_type MakeCreator()
	{
		return { 
			Act::Object::CalcHash(T::GetID()), 
			[]() -> std::unique_ptr < Act::Object > {
				return std::make_unique<T>();
			}
		};
	}

	std::map<std::uint32_t, ObjectCreator> MakeList()
	{
		std::map<std::uint32_t, ObjectCreator> ret;

		ret.insert(MakeCreator<CAct>());
		ret.insert(MakeCreator<CActLayer>());
		ret.insert(MakeCreator<CActKey>());
		ret.insert(MakeCreator<CAct2DMapLayout>());
		ret.insert(MakeCreator<ActResourceChip>());
		ret.insert(MakeCreator<ActTextureResourceInfo>());
		ret.insert(MakeCreator<MeshResourceInfo>());
		ret.insert(MakeCreator<C2DLayout>());
		ret.insert(MakeCreator<CActResource2D>());
		ret.insert(MakeCreator<CActRenderTarget>());

		return ret;
	}

	std::map<std::uint32_t, ObjectCreator> creator_list = MakeList();

	std::unique_ptr<Act::Object> GetObjectFromType(std::uint32_t type)
	{
		if (creator_list.find(type) == creator_list.end())
		{
			Act::Task::SendError("unknown object type: " + type);
		}
		return creator_list[type]();
	}
}

std::unique_ptr<Act::Object> Act::Object::ReadObject(Act::AbstractInputStream s, std::uint32_t type)
{
	std::unique_ptr<Act::Object> ret = GetObjectFromType(type);
	ret->Read(s);
	return ret;
}
