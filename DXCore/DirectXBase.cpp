#include "pch.h"
#include "DirectXBase.h" 
#include <windows.ui.xaml.media.dxinterop.h>
#include <math.h>

using namespace Microsoft::WRL;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace D2D1;
using namespace DXCore;

// Constructor.
DirectXBase::DirectXBase() :
	m_dpi(-1.0f)
{
}

// Initialize the DirectX resources required to run.
void DirectXBase::Initialize(CoreWindow^ window, SwapChainBackgroundPanel^ panel, float dpi)
{
	m_window = window;
	m_panel = panel;

	window->PointerPressed += ref new TypedEventHandler<CoreWindow^,PointerEventArgs^>(this,&DirectXBase::OnPointerPressed);
	window->PointerMoved += ref new TypedEventHandler<CoreWindow^,PointerEventArgs^>(this,&DirectXBase::OnPointerMoved);
	window->PointerReleased += ref new TypedEventHandler<CoreWindow^,PointerEventArgs^>(this,&DirectXBase::OnPointerReleased);
	
	CreateDeviceIndependentResources();
	CreateDeviceResources();
	SetDpi(dpi);
}

void DirectXBase::OnPointerReleased(CoreWindow^ window,PointerEventArgs^ pArgs)
{
	
}

void DirectXBase::OnPointerMoved(CoreWindow^ window,PointerEventArgs^ pArgs)
{
	
}

void DirectXBase::OnPointerPressed(CoreWindow^ window,PointerEventArgs^ pArgs)
{
	
}
// These are the resources required independent of the device.
void DirectXBase::CreateDeviceIndependentResources()
{
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	 // If the project is in a debug build, enable Direct2D debugging via SDK Layers
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	ThrowIfFailed(
		D2D1CreateFactory( 
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory1),
			&options,
			&m_d2dFactory
			)
		);

	ThrowIfFailed(
		DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			&m_dwriteFactory
			)
		);

	ThrowIfFailed(
		CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_wicFactory)
			)
		);
}

// These are the resources that depend on the device.
void DirectXBase::CreateDeviceResources()
{
	// This flag adds support for surfaces with a different color channel ordering than the API default.
	// It is recommended usage, and is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	ComPtr<IDXGIDevice> dxgiDevice;

#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers with this flag.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the DX11 API device object, and get a corresponding context.
	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ThrowIfFailed(
		D3D11CreateDevice(
			nullptr,                    // specify null to use the default adapter
			D3D_DRIVER_TYPE_HARDWARE,
			0,                          // leave as 0 unless software device
			creationFlags,              // optionally set debug and Direct2D compatibility flags
			featureLevels,              // list of feature levels this app can support
			ARRAYSIZE(featureLevels),   // number of entries in above list
			D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION for Metro style apps
			&device,                    // returns the Direct3D device created
			&m_featureLevel,            // returns feature level of device created
			&context                    // returns the device immediate context
			)
		);

	// Get the DirectX11.1 device by QI off the DirectX11 one.
	ThrowIfFailed(
		device.As(&m_d3dDevice)
		);

	// And get the corresponding device context in the same way.
	ThrowIfFailed(
		context.As(&m_d3dContext)
		);

	// Obtain the underlying DXGI device of the Direct3D11.1 device.
	ThrowIfFailed(
		m_d3dDevice.As(&dxgiDevice)
		);

	// Obtain the Direct2D device for 2-D rendering.
	ThrowIfFailed(
		m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
		);

	// And get its corresponding device context object.
	ThrowIfFailed(
		m_d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext
			)
		);

	// Release the swap chain (if it exists) as it will be incompatible with the new device.
	m_swapChain = nullptr;
}

// Helps track the DPI in the helper class.
// This is called in the dpiChanged event handler in the view class.
void DirectXBase::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		// Save the DPI of this display in our class.
		m_dpi = dpi;
		
		// Update Direct2D's stored DPI.
		m_d2dContext->SetDpi(m_dpi, m_dpi);

		// Often a DPI change implies a window size change. In some cases Windows will issues
		// both a size changed event and a DPI changed event. In this case, the resulting bounds 
		// will not change, and the window resize code will only be executed once.
		UpdateForWindowSizeChange();
	}
}

// This routine is called in the event handler for the view SizeChanged event.
void DirectXBase::UpdateForWindowSizeChange()
{
	// Only handle window size changed if there is no pending DPI change.

#ifdef W8_1

	if (m_dpi != DisplayInformation::GetForCurrentView()->LogicalDpi)
		return;

#else
	if (m_dpi != DisplayProperties::LogicalDpi)
		return;
#endif		

	m_d2dContext->SetTarget(nullptr);
	m_d2dTargetBitmap = nullptr;
	m_renderTargetView = nullptr;
	m_depthStencilView = nullptr;
	CreateWindowSizeDependentResources();
}

// Allocate all memory resources that change on a window SizeChanged event.
void DirectXBase::CreateWindowSizeDependentResources()
{
	// Store the window bounds so the next time we get a SizeChanged event we can
	// avoid rebuilding everything if the size is identical.
	m_windowBounds = m_window->Bounds;

	m_ground_pos = float2(m_windowBounds.Width / 2, m_windowBounds.Height - 20.0f);


	// Calculate the necessary swap chain and render target size in pixels.
	m_renderTargetSize.Width = ConvertDipsToPixels(m_windowBounds.Width);
	m_renderTargetSize.Height = ConvertDipsToPixels(m_windowBounds.Height);


	// If the swap chain already exists, resize it.
	if (m_swapChain != nullptr)
	{
		ThrowIfFailed(
			m_swapChain->ResizeBuffers(
				2,
				static_cast<UINT>(m_renderTargetSize.Width),
				static_cast<UINT>(m_renderTargetSize.Height),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
				)
			);
	}
	// Otherwise, create a new one.
	else
	{
		// Allocate a descriptor.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
		swapChainDesc.Width = static_cast<UINT>(m_renderTargetSize.Width); 
		swapChainDesc.Height = static_cast<UINT>(m_renderTargetSize.Height);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;           
		swapChainDesc.Stereo = false; 
		swapChainDesc.SampleDesc.Count = 1;                         
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;                              
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		swapChainDesc.Flags = 0;

		ComPtr<IDXGIDevice1> dxgiDevice;
		ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		ComPtr<IDXGIAdapter> dxgiAdapter;
		ThrowIfFailed(
			dxgiDevice->GetAdapter(&dxgiAdapter)
			);

		ComPtr<IDXGIFactory2> dxgiFactory;
		ThrowIfFailed(
			dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
			);

		// Create the swap chain and then associate it with the SwapChainBackgroundPanel.
		ThrowIfFailed(
			dxgiFactory->CreateSwapChainForComposition(
				m_d3dDevice.Get(),
				&swapChainDesc,
				nullptr,
				&m_swapChain
				)
			);

		ComPtr<ISwapChainBackgroundPanelNative> panelNative;
		ThrowIfFailed(
			reinterpret_cast<IUnknown*>(m_panel)->QueryInterface(IID_PPV_ARGS(&panelNative))
			);
		
		ThrowIfFailed(
			panelNative->SetSwapChain(m_swapChain.Get())
			);

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces 
		// latency and ensures that the application will only render after each VSync, minimizing 
		// power consumption.
		ThrowIfFailed(
			dxgiDevice->SetMaximumFrameLatency(1)
			);
	}

	// Obtain the backbuffer for this window which will be the final 3D rendertarget.
	ComPtr<ID3D11Texture2D> backBuffer;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
		);

	// Create a view interface on the rendertarget to use on bind.
	ThrowIfFailed(
		m_d3dDevice->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_renderTargetView
			)
		);

	// Create a descriptor for the depth/stencil buffer.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT, 
		static_cast<UINT>(m_renderTargetSize.Width),
		static_cast<UINT>(m_renderTargetSize.Height),
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
		);

	// Allocate a 2-D surface as the depth/stencil buffer.
	ComPtr<ID3D11Texture2D> depthStencil;
	ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			&depthStencil
			)
		);

	// Create a DepthStencil view on this surface to use on bind.
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_depthStencilView
			)
		);

	// Create a viewport descriptor of the full window size.
	CD3D11_VIEWPORT viewport(
		0.0f,
		0.0f,
		m_renderTargetSize.Width,
		m_renderTargetSize.Height
		);

	// Set the current viewport using the descriptor.
	m_d3dContext->RSSetViewports(1, &viewport);

	// Now we set up the Direct2D render target bitmap linked to the swapchain. 
	// Whenever we render to this bitmap, it will be directly rendered to the 
	// swapchain associated with the window.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = 
		BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi,
			m_dpi
			);

	// Direct2D needs the DXGI version of the backbuffer surface pointer.
	ComPtr<IDXGISurface> dxgiBackBuffer;
	ThrowIfFailed(
		m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
		);

	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	ThrowIfFailed(
		m_d2dContext->CreateBitmapFromDxgiSurface(
			dxgiBackBuffer.Get(),
			&bitmapProperties,
			&m_d2dTargetBitmap
			)
		);

	// So now we can set the Direct2D render target.
	m_d2dContext->SetTarget(m_d2dTargetBitmap.Get()); 
	// Set D2D text anti-alias mode to Grayscale to ensure proper rendering of text on intermediate surfaces.
	m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}
 

// Method to deliver the final image to the display.
void DirectXBase::Present()
{
	// The application may optionally specify "dirty" or "scroll" rects to improve efficiency
	// in certain scenarios.
	DXGI_PRESENT_PARAMETERS parameters = {0};
	parameters.DirtyRectsCount = 0;
	parameters.pDirtyRects = nullptr;
	parameters.pScrollRect = nullptr;
	parameters.pScrollOffset = nullptr;
	
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present1(1, 0, &parameters);

	// If the device was removed either by a disconnect or a driver upgrade, we 
	// must completely reinitialize the renderer.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		Initialize(m_window.Get(), m_panel, m_dpi);
	}
	else
	{
		ThrowIfFailed(hr);
	}
}

// Method to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
float DirectXBase::ConvertDipsToPixels(float dips)
{
	static const float dipsPerInch = 96.0f;
	return floor(dips * m_dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}
// Call this method when the app suspends to hint to the driver that the app is entering an idle state
// and that its memory can be used temporarily for other apps.


#ifdef W8_1

void DirectXBase::Trim()
{
	ComPtr<IDXGIDevice3> dxgiDevice;
	m_d3dDevice.As(&dxgiDevice);

	dxgiDevice->Trim();
}
#endif