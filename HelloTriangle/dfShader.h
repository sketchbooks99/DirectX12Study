#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include "../util/util.h"

// #include "../util/d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class dfShader {
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	dfShader();
	~dfShader() {}

	HRESULT loadShader(std::wstring vs_path, std::wstring ps_path);
	HRESULT loadShader(std::wstring vs_path, std::wstring ps_path, std::wstring gs_path) {}
	void loadCompute(std::wstring cpt_path) {}
	HRESULT CompileShaderFromFile(
		const std::wstring& filename, const std::wstring& profile, ComPtr<ID3DBlob>& shaderBlob, ComPtr<ID3DBlob>& errorBlob);
	HRESULT CreateRootSignature(ComPtr<ID3D12CommandList>& command);
	HRESULT CreatePipelineState();

	ComPtr<ID3DBlob> vs, ps;	// Vertex, Pixel shader.
	ComPtr<ID3DBlob> compute;	// Compute shader.
	ComPtr<ID3DBlob> errBlob;

private:
	D3D12_BLEND_DESC _blendState;

};