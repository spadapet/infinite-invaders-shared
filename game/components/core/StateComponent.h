#pragma once

class __declspec(uuid("c464b275-c021-48d3-bf92-c1b6e19be4fe")) __declspec(novtable)
	IStateComponent : public IUnknown
{
public:
	virtual int GetState() const = 0;
	virtual void SetState(int state) = 0;

	virtual size_t GetStateCounter() const = 0;
	virtual size_t IncrementStateCounter() = 0;
	virtual void ResetStateCounter() = 0;

	template<typename E>
	E GetEnumState() const
	{
		return (E)GetState();
	}

	template<typename E>
	void SetEnumState(E state)
	{
		SetState((int)state);
	}
};
