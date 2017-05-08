#pragma once

#include "coreEntity\system\System.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class IAudioEffect;
	class IRenderTarget;
	class ISpriteFont;
}

class LoadingComponent;
class IEntity;

enum GameMode;
enum Difficulty;

class __declspec(uuid("4fb72bcc-3e6d-4f94-8388-182d83607f15")) __declspec(novtable)
	IShowScoresState : public ISystem
{
	// Only used as a component ID
};

class __declspec(uuid("8bddd387-4ea7-4fc2-8b00-45d66ab9e7ab"))
	ShowScoresState : public ff::ComBase, public IShowScoresState
{
public:
	DECLARE_HEADER(ShowScoresState);

	static bool Create(IEntityDomain *pDomain, REFGUID underSystemId, ISystem *pUnderSystem, ISystem **ppState);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

private:
	bool Init(IEntityDomain *pDomain, REFGUID underSystemId, ISystem *pUnderSystem);
	void HandleEventStart(IEntityDomain *pDomain, ff::hash_t type);
	void HandleEventStop(IEntityDomain *pDomain, ff::hash_t type);
	void RenderHighScoreText(IEntityDomain *pDomain, ff::I2dRenderer *render);
	void CacheHighScoreText(IEntityDomain *pDomain);
	size_t GetShowScoresIndex() const;
	size_t AdjustShowScoresIndex(int dir);

	GUID _underSystemId;
	ff::ComPtr<ISystem> _pUnderSystem;
	ff::TypedResource<ff::ISpriteAnimation> _pBackAnim;
	ff::TypedResource<ff::ISpriteFont> _pClassicFont;
	ff::TypedResource<ff::ISpriteFont> _spaceFont;
	ff::TypedResource<ff::IAudioEffect> _effectMain;
	ff::ComPtr<ff::IInputMapping> _inputMapping;
	ff::ComPtr<IEntityDomain> _pChildDomain;
	ff::ComPtr<IEntity> _pBackEntity;
	ff::ComPtr<LoadingComponent> _loading;
	ff::ComPtr<I2dLayerGroup> _pRenderLayer;

	ff::String _szHighest;
	ff::String _szOther;
	ff::String _szIntro;
	ff::String _sz1P;
	ff::String _szCOOP;
	ff::String _szHard;
	ff::String _szNormal;
	ff::String _szEasy;
	GameMode _gameMode;
	Difficulty _difficulty;
};
