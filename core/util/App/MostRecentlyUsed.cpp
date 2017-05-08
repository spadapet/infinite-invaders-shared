#include "pch.h"
#include "App/MostRecentlyUsed.h"
#include "Dict/Dict.h"
#include "String/StringUtil.h"
#include "Windows/FileUtil.h"

static ff::StaticString OPTION_MRU_PATHS(L"Paths");
static ff::StaticString OPTION_MRU_NAMES(L"Names");
static ff::StaticString OPTION_MRU_PINNED(L"Pinned");

bool ff::RecentlyUsedItem::operator==(const RecentlyUsedItem &rhs) const
{
	return ff::PathsEqual(rhs._path, _path);
}

ff::MostRecentlyUsed::MostRecentlyUsed()
	: _listener(nullptr)
	, _limit(16)
{
}

ff::MostRecentlyUsed::~MostRecentlyUsed()
{
}

void ff::MostRecentlyUsed::Load(const Dict &dict)
{
	Clear();

	ValuePtr paths = dict.GetValue(OPTION_MRU_PATHS);
	ValuePtr names = dict.GetValue(OPTION_MRU_NAMES);
	ValuePtr pinned = dict.GetValue(OPTION_MRU_PINNED);

	if (paths && paths->IsType(Value::Type::StringVector) &&
		names && names->IsType(Value::Type::StringVector) &&
		pinned && pinned->IsType(Value::Type::IntVector) &&
		paths->AsStringVector().Size() == pinned->AsIntVector().Size() &&
		paths->AsStringVector().Size() == names->AsStringVector().Size())
	{
		for (size_t i = PreviousSize(paths->AsStringVector().Size()); i != INVALID_SIZE; i = PreviousSize(i))
		{
			Add(
				paths->AsStringVector()[i],
				names->AsStringVector()[i],
				pinned->AsIntVector()[i] ? true : false);
		}
	}
}

ff::Dict ff::MostRecentlyUsed::Save() const
{
	Vector<String> paths;
	Vector<String> names;
	Vector<int> pinned;

	for (const RecentlyUsedItem &item : _items)
	{
		paths.Push(item._path);
		names.Push(item._name);
		pinned.Push(item._pinned);
	}

	ValuePtr pathsValue, namesValue, pinnedValue;
	Value::CreateStringVector(std::move(paths), &pathsValue);
	Value::CreateStringVector(std::move(names), &namesValue);
	Value::CreateIntVector(std::move(pinned), &pinnedValue);

	Dict dict;
	dict.SetValue(OPTION_MRU_PATHS, pathsValue);
	dict.SetValue(OPTION_MRU_NAMES, namesValue);
	dict.SetValue(OPTION_MRU_PINNED, pinnedValue);

	return dict;
}

void ff::MostRecentlyUsed::SetListener(IMostRecentlyUsedListener *listener)
{
	_listener = listener;
}

const ff::RecentlyUsedItem &ff::MostRecentlyUsed::GetItem(size_t index) const
{
	return _items[index];
}

size_t ff::MostRecentlyUsed::GetCount() const
{
	return _items.Size();
}

void ff::MostRecentlyUsed::SetLimit(size_t limit)
{
	_limit = std::min<size_t>(128, limit);
}

void ff::MostRecentlyUsed::Add(StringRef path, StringRef name, bool pinned)
{
	assertRet(path.size());

	// Look for dupes
	for (size_t i = PreviousSize(_items.Size()); i != INVALID_SIZE; i = PreviousSize(i))
	{
		if (ff::PathsEqual(_items[i]._path, path))
		{
			pinned = pinned || _items[i]._pinned;
			_items.Delete(i);
			break;
		}
	}

	// Always add to the top of the list
	RecentlyUsedItem item;
	item._path = path;
	item._name = name;
	item._pinned = pinned;

	_items.Insert(0, item);

	// Limit the amount of items
	while (_items.Size() > _limit)
	{
		size_t nDelete = _items.Size() - 1;
		for (size_t i = nDelete; i != INVALID_SIZE; i = PreviousSize(i))
		{
			if (!_items[i]._pinned)
			{
				nDelete = i;
				break;
			}
		}

		_items.Delete(nDelete);
	}

	if (_listener != nullptr)
	{
		_listener->OnMruChanged(*this);
	}
}

bool ff::MostRecentlyUsed::Pin(StringRef path, bool pinned)
{
	assertRetVal(path.size(), false);

	for (RecentlyUsedItem &item: _items)
	{
		if (ff::PathsEqual(item._path, path) && item._pinned != pinned)
		{
			item._pinned = pinned;

			if (_listener != nullptr)
			{
				_listener->OnMruChanged(*this);
			}

			return true;
		}
	}

	return false;
}

bool ff::MostRecentlyUsed::Remove(StringRef path)
{
	assertRetVal(path.size(), false);

	for (const RecentlyUsedItem &item: _items)
	{
		if (ff::PathsEqual(item._path, path))
		{
			_items.DeleteItem(item);

			if (_listener != nullptr)
			{
				_listener->OnMruChanged(*this);
			}

			return true;
		}
	}

	return false;
}

void ff::MostRecentlyUsed::Clear()
{
	if (!_items.IsEmpty())
	{
		_items.Clear();

		if (_listener != nullptr)
		{
			_listener->OnMruChanged(*this);
		}
	}
}
