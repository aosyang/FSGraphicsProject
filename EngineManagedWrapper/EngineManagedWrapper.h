// EngineManagedWrapper.h

#pragma once

using namespace System;

class REngine;
class IApp;

namespace EngineManagedWrapper {

	public ref class RhinoEngineWrapper
	{
	private:
		REngine*	m_Engine;
		IApp*		m_Application;
		bool		m_IsInitialized;

	public:
		RhinoEngineWrapper();
		~RhinoEngineWrapper();
		!RhinoEngineWrapper();

		bool Initialize(IntPtr hWnd);

		void RunOneFrame();

		void Shutdown();

		void Resize(int width, int height);
	};
}
