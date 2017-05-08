#pragma once

class __declspec(uuid("f795fdfa-b2a4-4eca-bf59-f4049547c0b4"))
	IGameBeatService : public IUnknown
{
public:
	virtual size_t GetFrameCount() const = 0; // how many frames have advanced?
	virtual size_t GetFramesPerBeat() const = 0; // this changes as the game progresses
	virtual size_t GetBeatsPerMeasure() const = 0; // always four
	virtual size_t GetBeatsPerShot() const = 0; // this changes as the game progresses

	virtual size_t GetMeasureCount() const = 0;
	virtual size_t GetBeatCount() const = 0;

	virtual double GetMeasureReal() const = 0;
	virtual double GetBeatReal() const = 0;
};
