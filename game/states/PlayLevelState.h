#pragma once

#include "coreEntity\system\System.h"

namespace ff
{
	class IRenderTarget;
}

class LoadingComponent;
class PauseAdvanceRender;
class IEntity;

class __declspec(uuid("480f842a-b63b-4bf7-ad81-f93673d778a1")) __declspec(novtable)
	IPlayLevelState : public ISystem
{
	// Only used as a component ID
};

class __declspec(uuid("a866eac3-8012-4894-88f1-9266ac3bf773"))
	PlayLevelState : public ff::ComBase, public IPlayLevelState
{
public:
	DECLARE_HEADER(PlayLevelState);

	IEntityDomain* GetChildDomain();
	bool DidCompleteLevel() const;
	void OnStartPlaying();
	void OnStopPlaying();

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// ComBase
	HRESULT _Construct(IUnknown *unkOuter) override;

private:
	ff::ComPtr<IEntityDomain> _pChildDomain;
	ff::ComPtr<IEntity> _pChildEntity;
	ff::ComPtr<LoadingComponent> _loading;
	size_t _nCompletedCounter;
	bool _completedLevel;

	ff::ComPtr<PauseAdvanceRender, IAdvanceComponent> _pPauseAR;
};
