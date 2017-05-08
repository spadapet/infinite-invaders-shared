#pragma once

#include "coreEntity\entity\EntityListener.h"

namespace ff
{
	class IAudioPlaying;
}

class IEntity;

class __declspec(uuid("c23eb94c-a749-4187-bc01-4e88ca25a094"))
	AudioEffectComponent : public ff::ComBase, public IEntityEventListener
{
public:
	DECLARE_HEADER(AudioEffectComponent);

	void SetEffectPlaying(ff::IAudioPlaying *playing);
	ff::IAudioPlaying* GetEffectPlaying();

	// IEntityEventListener
	virtual void OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs) override;

private:
	ff::ComPtr<ff::IAudioPlaying> _playing;
};
