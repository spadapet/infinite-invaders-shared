#pragma once

#include "Dict/Value.h"
#include "Dict/SmallDict.h"

namespace ff
{
	class Dict
	{
	public:
		UTIL_API Dict(const Dict *parent = nullptr);
		UTIL_API Dict(const Dict &rhs);
		UTIL_API Dict(Dict &&rhs);
		UTIL_API Dict(const SmallDict &rhs);
		UTIL_API Dict(SmallDict &&rhs);
		UTIL_API ~Dict();

		// Copying
		UTIL_API const Dict &operator=(const Dict &rhs);
		UTIL_API const Dict &operator=(const SmallDict &rhs);
		UTIL_API void Add(const Dict &rhs, bool chain = true);
		UTIL_API void Add(const SmallDict &rhs);
		UTIL_API void Merge(const Dict &rhs, bool chain = true);
		UTIL_API void Reserve(size_t count);
		UTIL_API bool IsEmpty(bool chain = true) const;
		UTIL_API size_t Size(bool chain = true) const;

		// Parent dict
		UTIL_API void SetParent(const Dict *parent);
		UTIL_API const Dict *GetParent() const;

		// Operations
		UTIL_API void Clear();
		UTIL_API Vector<String> GetAllNames(bool chain = true, bool sorted = false, bool nameHashOnly = false) const;

		// Generic set/get
		UTIL_API void SetValue(StringRef name, Value *value);
		UTIL_API Value *GetValue(StringRef name, bool chain = true) const;

		// Option setters
		UTIL_API void SetInt(StringRef name, int value);
		UTIL_API void SetSize(StringRef name, size_t value);
		UTIL_API void SetBool(StringRef name, bool value);
		UTIL_API void SetRect(StringRef name, RectInt value);
		UTIL_API void SetRectF(StringRef name, RectFloat value);
		UTIL_API void SetFloat(StringRef name, float value);
		UTIL_API void SetDouble(StringRef name, double value);
		UTIL_API void SetPoint(StringRef name, PointInt value);
		UTIL_API void SetPointF(StringRef name, PointFloat value);
		UTIL_API void SetString(StringRef name, StringRef value);
		UTIL_API void SetGuid(StringRef name, REFGUID value);
		UTIL_API void SetData(StringRef name, IData *value);
		UTIL_API void SetSavedData(StringRef name, ISavedData *value);
		UTIL_API void SetDict(StringRef name, const Dict &value);
		UTIL_API void SetResource(StringRef name, const SharedResourceValue &value);

		// Option getters
		UTIL_API int GetInt(StringRef name, int defaultValue = 0, bool chain = true) const;
		UTIL_API size_t GetSize(StringRef name, size_t defaultValue = 0, bool chain = true) const;
		UTIL_API bool GetBool(StringRef name, bool defaultValue = false, bool chain = true) const;
		UTIL_API RectInt GetRect(StringRef name, RectInt defaultValue = RectInt(0, 0, 0, 0), bool chain = true) const;
		UTIL_API RectFloat GetRectF(StringRef name, RectFloat defaultValue = RectFloat(0, 0, 0, 0), bool chain = true) const;
		UTIL_API float GetFloat(StringRef name, float defaultValue = 0.0f, bool chain = true) const;
		UTIL_API double GetDouble(StringRef name, double defaultValue = 0.0, bool chain = true) const;
		UTIL_API PointInt GetPoint(StringRef name, PointInt defaultValue = PointInt(0, 0), bool chain = true) const;
		UTIL_API PointFloat GetPointF(StringRef name, PointFloat defaultValue = PointFloat(0, 0), bool chain = true) const;
		UTIL_API String GetString(StringRef name, String defaultValue = String(), bool chain = true) const;
		UTIL_API GUID GetGuid(StringRef name, REFGUID defaultValue = GUID_NULL, bool chain = true) const;
		UTIL_API ComPtr<IData> GetData(StringRef name, bool chain = true) const;
		UTIL_API ComPtr<ISavedData> GetSavedData(StringRef name, bool chain = true) const;
		UTIL_API Dict GetDict(StringRef name, bool chain = true) const;
		UTIL_API SharedResourceValue GetResource(StringRef name, bool chain = true) const;

		// Enum get/set helper

		template<typename E>
		void SetEnum(StringRef name, E value)
		{
			SetInt(name, (int)value);
		}

		template<typename E>
		E GetEnum(StringRef name, E defaultValue = (E)0, bool chain = true) const
		{
			return (E)GetInt(name, (int)defaultValue, chain);
		}

		UTIL_API void DebugDump() const;

	private:
		void InternalGetAllNames(Set<String> &names, bool chain, bool nameHashOnly) const;
		void CheckSize();
		StringCache &GetAtomizer() const;

		typedef Map<hash_t, ValuePtr, NonHasher<hash_t>> PropsMap;

		const Dict *_parent;
		StringCache *_atomizer;
		std::unique_ptr<PropsMap> _propsLarge;
		SmallDict _propsSmall;
	};

	// Common app settings

	UTIL_API extern StaticString OPTION_APP_MRU;
	UTIL_API extern StaticString OPTION_APP_USE_DIRECT3D;
	UTIL_API extern StaticString OPTION_APP_USE_JOYSTICKS;
	UTIL_API extern StaticString OPTION_APP_USE_MAIN_WINDOW_KEYBOARD;
	UTIL_API extern StaticString OPTION_APP_USE_MAIN_WINDOW_MOUSE;
	UTIL_API extern StaticString OPTION_APP_USE_MAIN_WINDOW_TOUCH;
	UTIL_API extern StaticString OPTION_APP_USE_RENDER_2D;
	UTIL_API extern StaticString OPTION_APP_USE_RENDER_DEPTH;
	UTIL_API extern StaticString OPTION_APP_USE_RENDER_MAIN_WINDOW;
	UTIL_API extern StaticString OPTION_APP_USE_XAUDIO;
	UTIL_API extern StaticString OPTION_APP_USE_XINPUT;
	UTIL_API extern StaticString OPTION_APP_ADVANCES_PER_SECOND;

	UTIL_API extern StaticString OPTION_GRAPH_FRAMES_PER_SECOND;
	UTIL_API extern StaticString OPTION_GRAPH_BACK_BUFFERS;
	UTIL_API extern StaticString OPTION_GRAPH_MULTI_SAMPLES;
	UTIL_API extern StaticString OPTION_GRAPH_SHOW_FPS_ON;
	UTIL_API extern StaticString OPTION_GRAPH_VSYNC_ON;

	UTIL_API extern StaticString OPTION_SOUND_MASTER_ON;
	UTIL_API extern StaticString OPTION_SOUND_MASTER_VOLUME;
	UTIL_API extern StaticString OPTION_SOUND_EFFECTS_VOLUME;
	UTIL_API extern StaticString OPTION_SOUND_MUSIC_VOLUME;

	UTIL_API extern StaticString OPTION_WINDOW_DEFAULT_CLIENT_SIZE;
	UTIL_API extern StaticString OPTION_WINDOW_FULL_SCREEN;
	UTIL_API extern StaticString OPTION_WINDOW_MAXIMIZED;
	UTIL_API extern StaticString OPTION_WINDOW_PADDING;
	UTIL_API extern StaticString OPTION_WINDOW_POSITION;
}
