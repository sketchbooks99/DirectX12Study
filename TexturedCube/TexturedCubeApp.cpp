#include <stdexcept>
#include "TexturedCubeApp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../util/stb_image.h"

void TexturedCubeApp::Prepare() {
    const float k = 1.0f;
    const Vector4 red(1.0f, 0.0f, 0.0f, 1.0f);
    const Vector4 green(0.0f, 1.0f, 0.0f, 1.0f);
    const Vector4 blue(0.0f, 0.0f, 1.0f, 1.0f);
    const Vector4 white(1.0f, 1.0f, 1.0f, 1.0f);
    const Vector4 black(0.0f, 0.0f, 0.0f, 1.0f);
    const Vector4 yellow(1.0f, 1.0f, 0.0f, 1.0f);
    const Vector4 magenta(1.0f, 0.0f, 1.0f, 1.0f);
    const Vector4 cyan(0.0f, 1.0f, 1.0f, 1.0f);

    Vertex triangleVertices[] = {
        // Front.
        { {-k,-k,-k}, red, { 0.0f, 1.0f} },
        { {-k, k,-k}, yellow, { 0.0f, 0.0f} },
        { { k, k,-k}, white, { 1.0f, 0.0f} },
        { { k,-k,-k}, magenta, { 1.0f, 1.0f} },
        // Right
        { { k,-k,-k}, magenta, { 0.0f, 1.0f} },
        { { k, k,-k}, white, { 0.0f, 0.0f} },
        { { k, k, k}, cyan, { 1.0f, 0.0f} },
        { { k,-k, k}, blue, { 1.0f, 1.0f} },
        // Left
        { {-k,-k, k}, black, { 0.0f, 1.0f} },
        { {-k, k, k}, green, { 0.0f, 0.0f} },
        { {-k, k,-k}, yellow, { 1.0f, 0.0f} },
        { {-k,-k,-k}, red, { 1.0f, 1.0f} },
        // Back
        { { k,-k, k}, blue, { 0.0f, 1.0f} },
        { { k, k, k}, cyan, { 0.0f, 0.0f} },
        { {-k, k, k}, green, { 1.0f, 0.0f} },
        { {-k,-k, k}, black, { 1.0f, 1.0f} },
        // Top
        { {-k, k,-k}, yellow, { 0.0f, 1.0f} },
        { {-k, k, k}, green, { 0.0f, 0.0f} },
        { { k, k, k}, cyan, { 1.0f, 0.0f} },
        { { k, k,-k}, white, { 1.0f, 1.0f} },
        // Bottom
        { {-k,-k, k}, red, { 0.0f, 1.0f} },
        { {-k,-k,-k}, red, { 0.0f, 0.0f} },
        { { k,-k,-k}, magenta, { 1.0f, 0.0f} },
        { { k,-k, k}, blue, { 1.0f, 1.0f} },
    };

    uint32_t indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    // Create vertex buffer and index buffer.
    m_vertexBuffer = CreateBuffer(sizeof(triangleVertices), triangleVertices);
    m_indexBuffer = CreateBuffer(sizeof(indices), indices);
    m_indexCount = _countof(indices);

    // Create views of each buffer.
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(triangleVertices);
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = sizeof(indices);
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // Compile shader.
    HRESULT hr;
    ComPtr<ID3DBlob> errBlob;
    hr = CompileShaderFromFile(L"VertexShader.hlsl", L"vs_6_0", m_vs, errBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA((const char*)errBlob->GetBufferPointer());
    }
    hr = CompileShaderFromFile(L"PixelShader.hlsl", L"ps_6_0", m_ps, errBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA((const char*)errBlob->GetBufferPointer());
    }

    CD3DX12_DESCRIPTOR_RANGE cbv, srv, sampler;
    cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 Register.
    cbv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 Register.
    sampler.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 1, 0); // s0 Register.

    CD3DX12_ROOT_PARAMETER rootParams[3];
    rootParams[0].InitAsDescriptorTable(1, &cbv, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParams[1].InitAsDescriptorTable(1, &srv, D3D12_SHADER_VISIBILITY_PIXEL);
    rootParams[2].InitAsDescriptorTable(1, &sampler, D3D12_SHADER_VISIBILITY_PIXEL);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Init(
        _countof(rootParams), rootParams, // pParameters
        0, nullptr, // pStaticSamplers.
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
    );
    ComPtr<ID3DBlob> signature;
    hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errBlob);
    if (FAILED(hr))
    {
        throw std::runtime_error("D3D12SerializeRootSignature failed.");
    }
    // Create rootSignature.
    hr = m_device->CreateRootSignature(
        0,
        signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    );
    if (FAILED(hr))
    {
        throw::std::runtime_error("CreateRootSignature failed.");
    }

    // Input layout. 
    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, Pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, Color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, UV), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA}
    };

    // Create pipeline state object.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    // Set the shader.
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_vs.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_ps.Get());
    // Settings of blend state.
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    // Settings of rasterizer state.
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    // The number of output is 1.
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    // Settings of depth buffer formats.
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

    psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };

    // Set the root signature.
    psoDesc.pRootSignature = m_rootSignature.Get();
}
