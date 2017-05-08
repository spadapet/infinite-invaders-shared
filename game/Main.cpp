#include "pch.h"
#include "App.xaml.h"
#include "MainUtilInclude.h"
#include "Module/Module.h"
#include "String/StringUtil.h"

// {5c9c7958-b19b-4f15-9712-499a14e91ccd}
static const GUID MODULE_ID = { 0x5c9c7958, 0xb19b, 0x4f15,{ 0x97, 0x12, 0x49, 0x9a, 0x14, 0xe9, 0x1c, 0xcd } };
static ff::StaticString MODULE_NAME(L"Invader");
static ff::ModuleFactory RegisterThisModule(MODULE_NAME, MODULE_ID, ff::GetMainInstance, ff::GetModuleStartup);

[Platform::MTAThread]
int main(Platform::Array<Platform::String ^> ^args)
{
	ff::SetMainModule(MODULE_NAME, MODULE_ID, ff::GetMainInstance());

	auto callbackFunc = [](Windows::UI::Xaml::ApplicationInitializationCallbackParams ^args)
	{
		auto app = ref new ::Invader::App();
	};

	auto callback = ref new Windows::UI::Xaml::ApplicationInitializationCallback(callbackFunc);
	Windows::UI::Xaml::Application::Start(callback);

	return 0;
}
