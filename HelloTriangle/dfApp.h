#pragma once

#include "../util/D3D12AppBase.h"
#include "../util/mathutil.h"
#include "../util/util.h"
#include "dfShader.h"

class dfApp : public D3D12AppBase {
public:
	dfApp() : D3D12AppBase() {};

	virtual void Cleanup() override;
	virtual void Setup() override;
	virtual void MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) override;

	struct Vertex {
		Vector3 Pos;
		Vector4 Color;
	};

private:
	//ComPtr<ID3D12Resource> CreateBuffer(UINT bufferSize, const void* initialData);

	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	UINT m_indexCount;

	dfShader m_shader;
	
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipeline;
};