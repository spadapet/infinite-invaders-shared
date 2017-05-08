#pragma once

// Standard entity events
extern const ff::hash_t ENTITY_EVENT_NULL;
extern const ff::hash_t ENTITY_EVENT_DESTROY; // param: nullptr
extern const ff::hash_t ENTITY_EVENT_ADD_COMPONENTS; // param: AddRemoveComponentsEventArgs*
extern const ff::hash_t ENTITY_EVENT_REMOVE_COMPONENTS; // param: AddRemoveComponentsEventArgs*
extern const ff::hash_t ENTITY_EVENT_ADD_COMPONENT; // param: nullptr (only sent to the one component being added)
extern const ff::hash_t ENTITY_EVENT_REMOVE_COMPONENT; // param: nullptr (only sent to the one component being removed)

struct AddRemoveComponentsEventArgs
{
	const GUID* _pCompIds;
	IUnknown** _ppComps;
	size_t _nCount;
};
