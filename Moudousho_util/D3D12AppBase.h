#pragma once

#include <Windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include "util.h"
#include "mathutil.h"

class D3D12AppBase {
public:
	// Featured by slarin lab
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	D3D12AppBase();
	virtual ~D3D12AppBase();

	void Initialize(HWND hwnd);
	void Terminate(){}

	virtual void Setup() {}
	virtual void Render() {}
	virtual void Cleanup() {}

protected:
	ID3D12Device* m_device;
	IDXGIFactory6* m_dxgiFactory;				// 
	IDXGISwapChain4* m_swapChain;				// Swap chain
	ID3D12CommandAllocator* m_commandAllocator;	// Command allocator
	ID3D12GraphicsCommandList* m_commandList;	// Command list
	ID3D12CommandQueue* m_commandQueue;			// Command queue
	ID3D12DescriptorHeap* m_heapRtv;			// Descriptor heap for RTV(Render Target View).
	ID3D12Fence* m_fence;
	UINT64 m_fenceVal = 0;

	UINT m_bufferCount = 2;
	std::vector<ID3D12Resource*> m_backBuffers;

	// For debug function.
	void EnableDebugLayer();

	void background(float* color);
};