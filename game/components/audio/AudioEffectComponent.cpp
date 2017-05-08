#include "pch.h"
#include "Audio\AudioPlaying.h"
#include "components\audio\AudioEffectComponent.h"
#include "coreEntity\entity\EntityEvents.h"
#include "entities\EntityEvents.h"
#include "Module\ModuleFactory.h"

static ff::ModuleStartup RegisterModuleClass([](ff::Module &module)
{
	ff::StaticString name(L"AudioEffectComponent");
	module.RegisterClassT<AudioEffectComponent>(name);
});

BEGIN_INTERFACES(AudioEffectComponent)
	HAS_INTERFACE(IEntityEventListener)
END_INTERFACES()

AudioEffectComponent::AudioEffectComponent()
{
}

AudioEffectComponent::~AudioEffectComponent()
{
}

void AudioEffectComponent::SetEffectPlaying(ff::IAudioPlaying *playing)
{
	_playing = playing;
}

ff::IAudioPlaying *AudioEffectComponent::GetEffectPlaying()
{
	return _playing;
}

void AudioEffectComponent::OnEntityEvent(IEntity *entity, ff::hash_t eventName, void *eventArgs)
{
	if (eventName == ENTITY_EVENT_DIED || eventName == ENTITY_EVENT_DESTROY)
	{
		if (_playing)
		{
			_playing->Stop();
			_playing = nullptr;
		}
	}
}
