#pragma once

#include "coreEntity\system\System.h"
#include "Resource\ResourceValue.h"

namespace ff
{
	class IInputMapping;
	class IRenderTarget;
	class ISpriteAnimation;
	class ISpriteFont;
}

class LoadingComponent;
class IEntity;

enum GameMode;
enum Difficulty;

class __declspec(uuid("b9205e7a-b137-4a9d-8aa1-ccbdf58f8119")) __declspec(novtable)
	IAddScoreState : public ISystem
{
	// Only used as a component ID
};

class __declspec(uuid("e73bd74e-fc11-4467-9bfd-4d36c98f5ecf"))
	AddScoreState : public ff::ComBase, public IAddScoreState
{
public:
	DECLARE_HEADER(AddScoreState);

	static bool Create(
		IEntityDomain* pDomain,
		REFGUID underSystemId,
		ISystem* pUnderSystem,
		size_t nPlayerIndex,
		PlayerGlobals* pPlayerGlobals,
		ISystem** ppState);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render(IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

private:
	bool Init(
		IEntityDomain* pDomain,
		REFGUID underSystemId,
		ISystem* pUnderSystem,
		size_t nPlayerIndex,
		PlayerGlobals* pPlayerGlobals);

	bool InitInputMapping(IEntityDomain *pDomain);
	void HandleEventStart(IEntityDomain *pDomain, ff::hash_t type);
	void HandleEventStop(IEntityDomain *pDomain, ff::hash_t type);

	struct SLetter
	{
		TCHAR _letter[5];
		ff::PointFloat _pos;
	};

	ff::PointFloat GetLetterPos(size_t index) const;
	bool InitLetters();
	void MoveLetter(ff::PointFloat dir);
	void AddLetter(IEntityDomain *pDomain, size_t index);

	GUID _underSystemId;
	ff::ComPtr<ISystem> _pUnderSystem;
	ff::TypedResource<ff::ISpriteAnimation> _pBackAnim;
	ff::TypedResource<ff::ISpriteAnimation> _pOrbAnim;
	ff::TypedResource<ff::ISpriteFont> _pClassicFont;
	ff::TypedResource<ff::ISpriteFont> _spaceFont;
	ff::ComPtr<ff::IInputMapping> _inputMapping;
	ff::ComPtr<IEntityDomain> _pChildDomain;
	ff::ComPtr<IEntity> _pBackEntity;
	ff::ComPtr<LoadingComponent> _loading;
	ff::ComPtr<I2dLayerGroup> _pRenderLayer;

	GameMode _gameMode;
	PlayerGlobals _playerGlobals;
	size_t _nPlayerIndex;
	size_t _nScoreIndex;
	ff::String _szScore;
	ff::String _szName;
	ff::String _szIntro;
	ff::String _szIntro2;
	size_t _nLetter;
	size_t _counter;
	ff::Vector<SLetter> _letters;
};
