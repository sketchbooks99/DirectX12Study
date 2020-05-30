#include "D3D12AppBase.h"

//#define WIN32_LEAN_AND_MEAN
//#define NOMINMAX

D3D12AppBase::D3D12AppBase()
{
    m_device = nullptr;
    m_dxgiFactory = nullptr;
    m_swapChain = nullptr;
    m_commandAllocator = nullptr;
    m_commandList = nullptr;
    m_commandQueue = nullptr;
    m_heapRtv = nullptr;
}

D3D12AppBase::~D3D12AppBase()
{

}

void D3D12AppBase::Initialize(HWND hwnd)
{
	HRESULT hr;
	// Create dxgifactory
	// It is neccesity to enumerate adapters.
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));

	// Create device.
	hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device));
	Util::checkResult(hr, "D3D12CreateDevice");

	// Create command allocator.
	hr = m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	Util::checkResult(hr, "CreateCommandAllocator");

	// Create command list.
	hr = m_device->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator, nullptr,
		IID_PPV_ARGS(&m_commandList));
	Util::checkResult(hr, "CreateCommandList");

	// Create command queue.
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};

	// No timeout.
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// It is 0 if we use only one adapter.
	cmdQueueDesc.NodeMask = 0;
	// There is no specification of priority.
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; 
	// Make the type same to the command list.
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// Create queue
	hr = m_device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&m_commandQueue));
	Util::checkResult(hr, "CreateCommandQueue");

	// Create swap chain.
	RECT rect;
	GetClientRect(hwnd, &rect);
    {
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
        swapChainDesc.Width = rect.right - rect.left;
        swapChainDesc.Height = rect.bottom - rect.top;
        swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.Stereo = false;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
        swapChainDesc.BufferCount = m_bufferCount;
        // Back buffer is stretchable.
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		// Destroy buffer immediately after flip it.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		// There is no specification.
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// Enable to toggle fullscreen mode.
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		hr = m_dxgiFactory->CreateSwapChainForHwnd(
			m_commandQueue,
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			(IDXGISwapChain1**)&m_swapChain
		);
		Util::checkResult(hr, "CreateSwapChainForHwnd");
    }

	// Create Descriptor Heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;		// The type of view.
		heapDesc.NodeMask = 0;								// Bit flag to identify multiple GPU.
		heapDesc.NumDescriptors = 2;						// The number of descriptor.
		// This is enumerator to judge whether is information of view required to refer from shader. 
		// In case of SRV or CBV, this is D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE.
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	

        hr = m_device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heapRtv));
		Util::checkResult(hr, "CreateDescriptorHeap");
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	hr = m_swapChain->GetDesc(&swapChainDesc);
	m_backBuffers.resize(m_bufferCount);

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heapRtv->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < m_bufferCount; i++)
	{
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
		Util::checkResult(hr, "m_swapChain->GetBuffer");

		m_device->CreateRenderTargetView(
			m_backBuffers[i],
			nullptr,
			handle);

		handle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// Reset the command allocator.
	hr = m_commandAllocator->Reset();

	// App work well if I commented out this check result function, but I'm not sure why. 
	//Util::checkResult(hr, "Reset command allocator"); 

	// Get current index of back buffer.
	auto backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();

	auto rtvH = m_heapRtv->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += backBufferIdx * m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_commandList->OMSetRenderTargets(1, &rtvH, true, nullptr);

	// Clear display.
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
	m_commandList->Close();

	ID3D12CommandList* cmdlists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(1, cmdlists);

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator, nullptr);

	m_swapChain->Present(1, 0);
}