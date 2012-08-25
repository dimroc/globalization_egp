// mp3player.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "mp3player.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}


HRESULT Cmp3player::PlayFile(LPCWSTR fn, HWND hWnd, UINT RESOURCE_SYMBOL) {
    // Initialize the COM library.
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {        
        return hr;
    }

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr))
    {        
        return hr;
    }

    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    hr = pGraph->QueryInterface(IID_IMediaEventEx, (void **)&pEvent);
    hr = pEvent->SetNotifyWindow((OAHWND)hWnd, RESOURCE_SYMBOL, 0);       

    // Build the graph. 
    hr = pGraph->RenderFile(fn, NULL);
    if (SUCCEEDED(hr))
    {
        // Run the graph.
        hr = pControl->Run();
    }    
    return hr;
}

VOID Cmp3player::CleanUp() {
    if(pControl) {
        pControl->Release();
        pControl = NULL;
    }
    if(pEvent) {
        pEvent->SetNotifyWindow(NULL, 0, 0);
        pEvent->Release();
        pEvent = NULL;
    }
    if(pGraph) {
        pGraph->Release();
        pGraph = NULL;
    }
    CoUninitialize();
}
