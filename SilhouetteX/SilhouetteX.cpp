//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "SilhouetteX.h"

using namespace std;
using namespace Concurrency;
using namespace DirectX;
using namespace DX;
//using namespace ShaderIDs;
using namespace XSDX;

using upCDXUTTextHelper = unique_ptr<CDXUTTextHelper>;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CDXUTDialogResourceManager		g_DialogResourceManager;	// manager for shared resources of dialogs
CModelViewerCamera				g_Camera;					// A model viewing camera
															// CD3DSettingsDlg				g_D3DSettingsDlg;			// Device settings dialog
															// CDXUTDialog					g_HUD;						// manages the 3D   
CDXUTDialog						g_SampleUI;					// dialog for sample specific controls
bool							g_bTypedUAVLoad = false;
bool							g_bROVSupport = false;
bool							g_bShowHelp = false;		// If true, it renders the UI control text
bool							g_bShowFPS = true;			// If true, it shows the FPS
bool							g_bLoadingComplete = false;

upCDXUTTextHelper				g_pTxtHelper;

upSilhouette					g_pSilhouette;

spShader						g_pShader;
spState							g_pState;

upDepthStencil					g_pDsBackBuffer;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum ButtonID
{
	IDC_TOGGLEFULLSCREEN = 1,
	IDC_TOGGLEREF,
	IDC_CHANGEDEVICE,
	IDC_TOGGLEWARP,
	IDC_RENDER_GS = 5,
	IDC_RENDER_TESS,
	IDC_RENDER_STYLIZED
};

uint8_t							g_uRenderMode = IDC_RENDER_GS;

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext);
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void *pUserContext);
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);

bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext);

void InitApp();
void RenderText();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w+t", stdout);
	freopen_s(&stream, "CONIN$", "r+t", stdin);
#endif

	// DXUT will create and use the best device
	// that is available on the system depending on which D3D callbacks are set below

	// Set DXUT callbacks
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackMouse(OnMouse, true);
	DXUTSetCallbackKeyboard(OnKeyboard);
	DXUTSetCallbackFrameMove(OnFrameMove);

	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
	DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

	InitApp();
	DXUTInit(true, true, nullptr); // Parse the command line, show msgboxes on error, no extra command line params
	DXUTSetCursorSettings(false, true); // Show the cursor and clip it when in full screen
	DXUTCreateWindow(L"Silhouette");

	/*auto deviceSettings = DXUTDeviceSettings();
	DXUTApplyDefaultDeviceSettings(&deviceSettings);
	deviceSettings.MinimumFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	deviceSettings.d3d11.AutoCreateDepthStencil = false;
	deviceSettings.d3d11.sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	deviceSettings.d3d11.sd.BufferUsage |= DXGI_USAGE_SHADER_INPUT;

	DXUTCreateDeviceFromSettings(&deviceSettings);*/
	DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, 1280, 960);
	DXUTMainLoop(); // Enter into the DXUT render loop

#if defined(DEBUG) | defined(_DEBUG)
	FreeConsole();
#endif

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	g_SampleUI.Init(&g_DialogResourceManager);
	g_SampleUI.SetCallback(OnGUIEvent);

	auto iX = -160;
	auto iY = -670;
	g_SampleUI.AddRadioButton(IDC_RENDER_GS, 0u, L"Simple silhouette by geometry shader", iX, iY += 26, 150, 22);
	g_SampleUI.AddRadioButton(IDC_RENDER_TESS, 0u, L"Simple silhouette by tessellation", iX, iY += 26, 150, 22);
	g_SampleUI.AddRadioButton(IDC_RENDER_STYLIZED, 0u, L"Stylized silhouette by tessellation", iX, iY += 26, 150, 22);
	g_SampleUI.GetRadioButton(IDC_RENDER_GS)->SetChecked(true);
	g_SampleUI.GetRadioButton(IDC_RENDER_GS)->SetTextColor(0);
	g_SampleUI.GetRadioButton(IDC_RENDER_TESS)->SetTextColor(0);
	g_SampleUI.GetRadioButton(IDC_RENDER_STYLIZED)->SetTextColor(0);
}

//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	pDeviceSettings->d3d11.SyncInterval = 0;

	return true;
}

//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	// Update the camera's position based on user input 
	g_Camera.FrameMove(fElapsedTime);
}

//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{
	UINT nBackBufferHeight = DXUTGetDXGIBackBufferSurfaceDesc()->Height;

	g_pTxtHelper->Begin();
	g_pTxtHelper->SetInsertionPos(2, 0);
	g_pTxtHelper->SetForegroundColor(Colors::Red);
	g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());

	// Draw help
	if (g_bShowHelp)
	{
		g_pTxtHelper->SetInsertionPos(2, nBackBufferHeight - 20 * 4);
		g_pTxtHelper->SetForegroundColor(Colors::Red);
		g_pTxtHelper->DrawTextLine(L"Controls:");

		g_pTxtHelper->SetInsertionPos(20, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Rotate camera: Left mouse button\n"
			L"Zoom camera: Mouse wheel scroll\n");

		g_pTxtHelper->SetInsertionPos(285, nBackBufferHeight - 20 * 3);
		g_pTxtHelper->DrawTextLine(L"Hide help: F1\n"
			L"Quit: ESC\n");
	}
	else
	{
		g_pTxtHelper->SetForegroundColor(Colors::White);
		g_pTxtHelper->DrawTextLine(L"Press F1 for help");
	}

	g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
	void* pUserContext)
{
	// Pass messages to dialog resource manager calls so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) return 0;

	// Give the dialogs a chance to handle the message first
	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) return 0;

	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages(hWnd, uMsg, wParam, lParam);

	return 0;
}

//--------------------------------------------------------------------------------------
// Handle mouse events
//--------------------------------------------------------------------------------------
void CALLBACK OnMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
	bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
	int xPos, int yPos, void *pUserContext)
{
	if (bLeftButtonDown)
	{
	}
	else
	{
	}
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if (bKeyDown)
	{
		switch (nChar)
		{
		case VK_F1:
			g_bShowHelp = !g_bShowHelp; break;
		case VK_F2:
			g_bShowFPS = !g_bShowFPS; break;
		}
	}
	/*else
	{
	switch (nChar)
	{
	}
	}*/
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	switch (nControlID)
	{
		// Standard DXUT controls
	case IDC_RENDER_GS:
	case IDC_RENDER_TESS:
	case IDC_RENDER_STYLIZED:
		g_uRenderMode = nControlID;
		break;
	}
}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
	DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
	void* pUserContext)
{
	HRESULT hr;

	LPDXContext pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));
	g_pTxtHelper = make_unique<CDXUTTextHelper>(pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15);

	// Load shaders asynchronously.
	g_pShader = make_shared<Shader>(pd3dDevice);
	g_pState = make_shared<State>(pd3dDevice);

	g_pSilhouette = make_unique<Silhouette>(pd3dDevice, g_pShader, g_pState);

	const auto loadVSTask = g_pShader->CreateVertexShader(L"VSBasePass.cso", Silhouette::VS_BASEPASS);
	auto loadHSTask = g_pShader->CreateHullShader(L"HSSilhouette.cso", Silhouette::HS_SILHOUETTE);
	loadHSTask = loadHSTask && g_pShader->CreateHullShader(L"HSSilhouetteTess.cso", Silhouette::HS_SILHOUETTE_TESS);
	const auto loadDSTask = g_pShader->CreateDomainShader(L"DSSilhouette.cso", Silhouette::DS_SILHOUETTE);
	auto loadGSTask = g_pShader->CreateGeometryShader(L"GSSilhouette.cso", Silhouette::GS_SILHOUETTE);
	loadGSTask = loadGSTask && g_pShader->CreateGeometryShader(L"GSParticle.cso", Silhouette::GS_PARTICLE);
	auto loadPSTask = g_pShader->CreatePixelShader(L"PSSimple.cso", Silhouette::PS_SIMPLE);
	loadPSTask = loadPSTask && g_pShader->CreatePixelShader(L"PSGauss.cso", Silhouette::PS_GAUSS);

	const auto createShaderTask = loadVSTask && loadHSTask && loadDSTask && loadGSTask && loadPSTask;
	
	// Once the mesh is loaded, the object is ready to be rendered.
	createShaderTask.then([pd3dDevice]()
	{
		Silhouette::CreateVertexLayout(pd3dDevice, Silhouette::GetVertexLayout(), g_pShader, 0u);
		g_pShader->ReleaseShaderBuffers();

		// View
		// Setup the camera's view parameters
		const auto vLookAtPt = XMFLOAT4(0.0f, 4.0f, 0.0f, 1.0f);
		const auto vEyePt = XMFLOAT4(0.0f, 8.0f, 16.0f, 1.0f);
		g_Camera.SetViewParams(XMLoadFloat4(&vEyePt), XMLoadFloat4(&vLookAtPt));

		g_bLoadingComplete = true;
	});

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
	const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	// Setup the camera's projection parameters
	auto fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_Camera.SetProjParams(g_fFOVAngleY, fAspectRatio, g_fZNear, g_fZFar);
	g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	g_Camera.SetButtonMasks(MOUSE_MIDDLE_BUTTON, MOUSE_WHEEL, MOUSE_LEFT_BUTTON);

	// Initialize window size dependent resources
	// Viewport clipping
	auto uVpNum = 1u;
	D3D11_VIEWPORT viewport;
	DXUTGetD3D11DeviceContext()->RSGetViewports(&uVpNum, &viewport);

	if (g_pSilhouette) g_pSilhouette->Init(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);

	g_pDsBackBuffer = make_unique<DepthStencil>(pd3dDevice);
	g_pDsBackBuffer->Create(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height, true, DXGI_FORMAT_D32_FLOAT);

	//g_HUD.SetLocation(pBackBufferSurfaceDesc->Width - 170, 0);
	//g_HUD.SetSize(170, 170);
	g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 300);
	g_SampleUI.SetSize(170, 300);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
	float fElapsedTime, void* pUserContext)
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!g_bLoadingComplete) return;

	// Set render targets to the screen.
	LPDXRenderTargetView pRTVs[] = { DXUTGetD3D11RenderTargetView() };
	LPDXDepthStencilView pDSV = g_pDsBackBuffer->GetDSV().Get();
	pd3dImmediateContext->ClearRenderTargetView(pRTVs[0], Colors::Wheat);
	pd3dImmediateContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0u);
	pd3dImmediateContext->OMSetRenderTargets(1u, pRTVs, pDSV);

	// Prepare the constant buffer to send it to the graphics device.
	// Get the projection & view matrix from the camera class
	const auto mProj = g_Camera.GetProjMatrix();
	const auto mView = g_Camera.GetViewMatrix();
	const auto mViewProj = XMMatrixMultiply(mView, mProj);

	// Render
	g_pSilhouette->UpdateFrame(g_Camera.GetEyePt(), mViewProj);
	switch (g_uRenderMode)
	{
	case IDC_RENDER_TESS:
		g_pSilhouette->Render(Silhouette::RENDER_TESS);
		break;
	case IDC_RENDER_STYLIZED:
		g_pSilhouette->Render(Silhouette::RENDER_STYLIZED);
		break;
	default:
		g_pSilhouette->Render();
	}

	//pd3dImmediateContext->OMSetRenderTargets(1u, pRTVs, pDSV);

	DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");
	g_SampleUI.OnRender(fElapsedTime);
	if (g_bShowFPS) RenderText();
	DXUT_EndPerfEvent();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D11DestroyDevice();
	DXUTGetGlobalResourceCache().OnDestroyDevice();

	g_bLoadingComplete = false;
	Silhouette::GetVertexLayout().Reset();
	g_pTxtHelper.reset();
	g_pDsBackBuffer.reset();
	g_pSilhouette.reset();
	g_pState.reset();
	g_pShader.reset();
}
