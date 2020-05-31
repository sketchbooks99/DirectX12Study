#include "dfApp.h"

void dfApp::Setup() {
	Vertex vertices[] = {
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
	};
	uint32_t indices[] = { 0, 1, 2 };

	// Create index buffer and vertex buffer.
	m_vertexBuffer = CreateBuffer(sizeof(vertices), vertices);
	m_indexBuffer = CreateBuffer(sizeof(indices), indices);
	m_indexCount = _countof(indices);

	// Create views of each buffer.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = sizeof(vertices);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(vertices);
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	// Compile Shader.
	HRESULT hr;
	hr = m_shader.loadShader(L"VertexShader.hlsl", L"PixelShader.hlsl");
	Util::CheckResult(hr, "loadShader");

	// Construction of root signature.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(
		0, nullptr,
		0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);
	ComPtr<ID3DBlob> signature;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &m_shader.errBlob);
	Util::CheckResult(hr, "D3D12SerializeRootSignature");

	// Create root signature.
	hr = m_device->CreateRootSignature(
		0, signature->GetBufferPointer(), signature->GetBufferSize(),
		IID_PPV_ARGS(&m_rootSignature)
	);
	Util::CheckResult(hr, "CreateRootSignature");

	D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}
	};

	// Create pipeline state object.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	// Set the shader.
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_shader.vs.Get());
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_shader.ps.Get());
	// Setting of blend state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	// Setting of rasterizer state.
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	// The number of output.
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Setting of depth buffer format.
	psoDesc.DSVFormat = DXGI_FORMAT_R32_FLOAT;
	psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	// Set the root signature.
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// Setting of multiple sample.
	psoDesc.SampleDesc = { 1, 0 };
	psoDesc.SampleMask = UINT_MAX;

	hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
	Util::CheckResult(hr, "CreateGraphicsPipelineState.");
}

void dfApp::Cleanup()
{
	auto index = m_swapChain->GetCurrentBackBufferIndex();
	auto fence = m_frameFences[index];
	auto value = ++m_frameFenceValues[index];
	m_commandQueue->Signal(fence.Get(), value);
	fence->SetEventOnCompletion(value, m_fenceWaitEvent);
	WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
}

void dfApp::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command)
{
	// Set the pipeline state.
	command->SetPipelineState(m_pipeline.Get());
	// Set the root signature.
	command->SetGraphicsRootSignature(m_rootSignature.Get());
	// Set viewport and scissor.
	command->RSSetViewports(1, &m_viewport);
	command->RSSetScissorRects(1, &m_scissorRect);

	// Set the Primitive, Vertex, Index buffers.
	command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	command->IASetIndexBuffer(&m_indexBufferView);

	// Render order.
	command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}