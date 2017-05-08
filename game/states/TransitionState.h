#pragma once

namespace ff
{
	class IGraphTexture;
	class ISprite;
}

class __declspec(uuid("09d1108e-8fe5-4078-8b4e-1bdaf0b445f7")) __declspec(novtable)
	ITransitionState : public ISystem
{
	// Only used as a component ID
};

enum TransitionType
{
	TRANSITION_WIPE_HORIZONTAL,
	TRANSITION_WIPE_VERTICAL,
	TRANSITION_FADE,
	TRANSITION_FADE_TO_COLOR,
};

class __declspec(uuid("3db0e92a-7d26-4b73-8ecc-aaa7d7c85440"))
	TransitionState : public ff::ComBase , public ITransitionState
{
public:
	DECLARE_HEADER(TransitionState);

	static bool Create(
		IEntityDomain *pDomain,
		ISystem *pOld,
		ISystem *pNew,
		REFGUID newSysId,
		TransitionType type,
		const DirectX::XMFLOAT4 *pColor,
		float seconds,
		ISystem **ppTrans);

	void SetAdvance(bool bAdvanceOldSystem, bool bAdvanceNewSystem);

	// ISystem
	virtual int GetSystemPriority() const override;
	virtual PingResult Ping(IEntityDomain *pDomain) override;
	virtual void Advance(IEntityDomain *pDomain) override;
	virtual void Render (IEntityDomain *pDomain, ff::IRenderTarget *pTarget) override;

	// ComBase
	virtual HRESULT _Construct(IUnknown *unkOuter) override;

private:
	bool Init(
		ISystem *pOld,
		ISystem *pNew,
		REFGUID newSysId,
		TransitionType type,
		const DirectX::XMFLOAT4 *pColor,
		float seconds);

	void AdvanceFrame(IEntityDomain *pDomain);
	bool IsNewSystemReady(IEntityDomain *pDomain) const;
	void RenderWipe(IEntityDomain *pDomain, ff::IRenderTarget *pTarget);
	void RenderFade(IEntityDomain *pDomain, ff::IRenderTarget *pTarget);
	void RenderFadeToColor(IEntityDomain *pDomain, ff::IRenderTarget *pTarget);

	TransitionType _type;
	DirectX::XMFLOAT4 _color;
	ff::ComPtr<ISystem> _pOldSystem;
	ff::ComPtr<ISystem> _pNewSystem;
	GUID _newSysId;
	size_t _frame;
	size_t _totalFrames;
	bool _bAdvanceOldSystem;
	bool _bAdvanceNewSystem;
	bool _bWaitingForNewSystem;

	ff::ComPtr<ff::IGraphTexture> _pTexture;
	ff::ComPtr<ff::IRenderTarget> _pTarget;
	ff::ComPtr<ff::ISprite> _sprite;
};
