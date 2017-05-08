#pragma once

namespace ff
{
	struct FrameTime;
	struct GlobalTime;
	class I2dRenderer;
	class I2dEffect;
	class IAudioDevice;
	class IGraphDevice;
	class IRenderTargetWindow;
	class IRenderDepth;
	class IPointerDevice;
	class IKeyboardDevice;
	class IJoystickInput;
	class IThreadDispatch;

	class AppGlobals
	{
	public:
		virtual IAudioDevice *GetAudio() const = 0;
		virtual IGraphDevice *GetGraph() const = 0;
		virtual IRenderTargetWindow *GetTarget() const = 0;
		virtual IRenderDepth *GetDepth() const = 0;
		virtual IPointerDevice *GetPointer() const = 0;
		virtual IKeyboardDevice *GetKeys() const = 0;
		virtual IJoystickInput *GetJoysticks() const = 0;
		virtual I2dRenderer *Get2dRender() const = 0;
		virtual I2dEffect *Get2dEffect() const = 0;
		virtual IThreadDispatch *GetGameDispatch() const = 0;
		virtual const GlobalTime &GetGlobalTime() const = 0;
		virtual const FrameTime &GetFrameTime() const = 0;
	};
}
