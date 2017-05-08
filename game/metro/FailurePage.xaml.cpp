#include "pch.h"
#include "metro\FailurePage.xaml.h"
#include "Windows\FileUtil.h"

Invader::FailurePage::FailurePage()
{
	InitializeComponent();
}

Invader::FailurePage::FailurePage(Platform::String ^logFile)
{
	InitializeComponent();

	ff::String text;
	ff::String path = ff::String::from_pstring(logFile);

	if (ff::FileExists(path) && ff::ReadWholeFile(path, text))
	{
		ErrorRun->Text = text.pstring();
	}
}
