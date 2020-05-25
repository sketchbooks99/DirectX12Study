#include <stdexcept>
#include "TexturedCubeApp.h"
#include <DirectXTex/DirectXTex.h>
#pragma comment(lib, "DirectXTex.lib")

#include "WICTextureLoader12.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../util/stb_image.h"

#include "../util/util.h"



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
    srv.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 Register.
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
        throw std::runtime_error("CreateRootSignature failed.");
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
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    // Settings of multi sample
    psoDesc.SampleDesc = { 1, 0 };
    psoDesc.SampleMask = UINT_MAX; 

    hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipeline));
    if (FAILED(hr))
    {
        throw std::runtime_error("CreateGraphicsPipelineState failed.");
    }

    PrepareDescriptorHeapForTexturedCubeApp();

    // Create constant buffer / constant buffer view. 
    m_constantBuffers.resize(FrameBufferCount);
    m_cbViews.resize(FrameBufferCount);
    for (UINT i = 0; i < FrameBufferCount; i++) {
        UINT bufferSize = sizeof(ShaderParameters) + 255 & ~255;
        m_constantBuffers[i] = CreateBuffer(bufferSize, nullptr);

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc{};
        cbDesc.BufferLocation = m_constantBuffers[i]->GetGPUVirtualAddress();
        cbDesc.SizeInBytes = bufferSize;
        CD3DX12_CPU_DESCRIPTOR_HANDLE handleCBV(m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart(), ConstantBufferDescriptorBase + i, m_srvcbvDescriptorSize);
        m_device->CreateConstantBufferView(&cbDesc, handleCBV);

        m_cbViews[i] = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapSrvCbv->GetGPUDescriptorHandleForHeapStart(), ConstantBufferDescriptorBase + i, m_srvcbvDescriptorSize);
    }

    // Create texture.
    m_texture = DXCreateTexture(L"normal.png");
    //m_texture = CreateTexture("texture.tga");

    // Create sampler.
    D3D12_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D12_ENCODE_BASIC_FILTER(
        D3D12_FILTER_TYPE_LINEAR, // min
        D3D12_FILTER_TYPE_LINEAR, // mag
        D3D12_FILTER_TYPE_LINEAR, // mip
        D3D12_FILTER_REDUCTION_TYPE_STANDARD);
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MaxLOD = FLT_MAX;
    samplerDesc.MinLOD = -FLT_MAX;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

    // Use 0 index of descriptor heap for sampler.
    auto descriptorSampler = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapSampler->GetCPUDescriptorHandleForHeapStart(), SamplerDescriptorBase, m_samplerDescriptorSize);
    m_device->CreateSampler(&samplerDesc, descriptorSampler);
    m_sampler = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapSampler->GetGPUDescriptorHandleForHeapStart(), SamplerDescriptorBase, m_samplerDescriptorSize);

    // Prepare shader resource view from texture.
    auto srvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart(), TextureSrvDescriptorBase, m_srvcbvDescriptorSize);
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    D3D12_RESOURCE_DESC textureDesc = m_texture->GetDesc();
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, 
        m_heapSrvCbv->GetCPUDescriptorHandleForHeapStart());
    m_srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_heapSrvCbv->GetGPUDescriptorHandleForHeapStart(), TextureSrvDescriptorBase, m_srvcbvDescriptorSize);
}

void TexturedCubeApp::Cleanup() {
    auto index = m_swapChain->GetCurrentBackBufferIndex();
    auto fence = m_frameFences[index];
    auto value = ++m_frameFenceValues[index];
    m_commandQueue->Signal(fence.Get(), value);
    fence->SetEventOnCompletion(value, m_fenceWaitEvent);
    WaitForSingleObject(m_fenceWaitEvent, GpuWaitTimeout);
}

void TexturedCubeApp::MakeCommand(ComPtr<ID3D12GraphicsCommandList>& command) {
    using namespace DirectX;

    // Set each matrices.
    ShaderParameters shaderParams;
    XMStoreFloat4x4(&shaderParams.mtxWorld, XMMatrixRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), XMConvertToRadians(45.0f)));
    auto mtxView = XMMatrixLookAtLH(
        XMVectorSet(0.0f, 3.0f, -5.0f, 0.0f),
        XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
        XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
    );
    auto mtxProj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), m_viewport.Width / m_viewport.Height, 0.1f, 100.0f);
    XMStoreFloat4x4(&shaderParams.mtxView, XMMatrixTranspose(mtxView));
    XMStoreFloat4x4(&shaderParams.mtxProj, XMMatrixTranspose(mtxProj));

    // Update constant buffer.
    auto& constantBuffer = m_constantBuffers[m_frameIndex];
    {
        void* p;
        CD3DX12_RANGE range(0, 0);
        constantBuffer->Map(0, &range, &p);
        memcpy(p, &shaderParams, sizeof(shaderParams));
        constantBuffer->Unmap(0, nullptr);
    }

    // Set the pipeline state.
    command->SetPipelineState(m_pipeline.Get());
    // Set the root signature.
    command->SetGraphicsRootSignature(m_rootSignature.Get());
    // Set the viewport and scissor.
    command->RSSetViewports(1, &m_viewport);
    command->RSSetScissorRects(1, &m_scissorRect);

    // Set the descriptor heap.
    ID3D12DescriptorHeap* heaps[] = {
        m_heapSrvCbv.Get(), m_heapSampler.Get()
    };
    command->SetDescriptorHeaps(_countof(heaps), heaps);

    // Set the primitive type, vertex, index buffers.
    command->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    command->IASetIndexBuffer(&m_indexBufferView);

    command->SetGraphicsRootDescriptorTable(0, m_cbViews[m_frameIndex]);
    command->SetGraphicsRootDescriptorTable(1, m_srv);
    command->SetGraphicsRootDescriptorTable(2, m_sampler);

    // Make rendering order.
    command->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

TexturedCubeApp::ComPtr<ID3D12Resource1> TexturedCubeApp::CreateBuffer(UINT bufferSize, const void* initialData)
{
    HRESULT hr;
    ComPtr<ID3D12Resource1> buffer;
    hr = m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&buffer)
    );

    // Make copy if there is specification of initial data.
    if (SUCCEEDED(hr) && initialData != nullptr)
    {
        void* mapped;
        CD3DX12_RANGE range(0, 0);
        hr = buffer->Map(0, &range, &mapped);
        if (SUCCEEDED(hr))
        {
            memcpy(mapped, initialData, bufferSize);
            buffer->Unmap(0, nullptr);
        }
    }

    return buffer;
}

TexturedCubeApp::ComPtr<ID3D12Resource> TexturedCubeApp::DXCreateTexture(const std::wstring& fileName)
{
    HRESULT hr;
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
        throw std::runtime_error("Faileed CoInitlializeEx.");
    
    ComPtr<ID3D12Resource> textureUploadHeap;
    ComPtr<ID3D12Resource> texture;

    // Create texture from WIC.
    {
        std::unique_ptr<uint8_t[]> wicData;
        D3D12_SUBRESOURCE_DATA subresourceData;
        hr = DirectX::LoadWICTextureFromFile(m_device.Get(),
            fileName.c_str(),
            &texture,
            wicData,
            subresourceData);
        if (FAILED(hr))
            throw std::runtime_error("Failed LoadWICTextureFromFile.");

        const UINT64 uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, 1);

        // Create the GPU upload buffer.
        hr = m_device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&textureUploadHeap)
        );
        if (FAILED(hr))
            throw std::runtime_error("Failed CreateCommittedResource.");

        
        
        UpdateSubresources(m_commandList.Get(),
            texture.Get(),
            textureUploadHeap.Get(),
            0,
            0,
            1,
            &subresourceData);
        // Finally, set the resource barrier.
        auto barrierTex = CD3DX12_RESOURCE_BARRIER::Transition(
            texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );

        m_commandList->ResourceBarrier(1, &barrierTex);
        m_commandList->Close();
    }
    return texture;
}

//TexturedCubeApp::ComPtr<ID3D12Resource> TexturedCubeApp::DXCreateTexture(const std::wstring& fileName)
//{
//    ComPtr<ID3D12Resource> texture;
//    ComPtr<ID3D12Resource1> staging;
//    DirectX::ScratchImage image;
//    DirectX::TexMetadata metadata;
//    HRESULT hr;
//    
//    std::vector<std::wstring> name_elements = split(fileName, '.');
//    std::wstring fmt = name_elements[name_elements.size() - 1];
//
//    // TGAFile : Load "TGA" file.
//    // WICFile : Load "PNG/JPEG" file.
//    // DDSFile : Load "DDS" file.
//    // HDRFile : Load "HDR" file.
//    std::string funcName = "";
//    if(!fmt.compare(L"tga"))
//    {
//        hr = DirectX::LoadFromTGAFile(fileName.c_str(), &metadata, image);
//        funcName = "LoadFromTGAFile";
//    } 
//    else if(!fmt.compare(L"png") || !fmt.compare(L"jpeg") || !fmt.compare(L"jpg"))
//    {
//        hr = DirectX::LoadFromWICFile(fileName.c_str(), 0, &metadata, image);
//        funcName = "LoadFromWICFile";
//    }
//    else if(!fmt.compare(L"hdr"))
//    {
//        hr = DirectX::LoadFromHDRFile(fileName.c_str(), &metadata, image);
//        funcName = "LoadFromHDRFile";
//    }
//    else 
//    {
//        std::cout << "This format is not supported." << std::endl;
//        return 0;
//    }
//
//    if (FAILED(hr))
//    {
//        throw std::runtime_error("Failed Load" + funcName);
//    }
//    
//    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
//
//    // Prepare for upload heap.
//    DirectX::PrepareUpload(m_device.Get(),
//        image.GetImages(), image.GetImageCount(),
//        metadata, subresources);
//    const auto totalBytes = GetRequiredIntermediateSize(
//        texture.Get(), 0, subresources.size());
//    m_device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//        D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(totalBytes),
//        D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr,
//        IID_PPV_ARGS(&staging));
//
//    // Create texture.
//    DirectX::CreateTexture(m_device.Get(), metadata, &texture);
//
//    // Prepare the command.
//    ComPtr<ID3D12GraphicsCommandList> command;
//    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&command));
//    // Processing to transfer.
//    UpdateSubresources(
//        command.Get(),
//        texture.Get(), staging.Get(),
//        0, 0,
//        uint32_t(subresources.size()),
//        subresources.data());
//    // Finally, set the resource barrier.
//    auto barrierTex = CD3DX12_RESOURCE_BARRIER::Transition(
//        texture.Get(),
//        D3D12_RESOURCE_STATE_COPY_DEST,
//        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
//    );
//
//    command->ResourceBarrier(1, &barrierTex);
//    command->Close();
//
//    return texture;
//}

// Manually create version.
TexturedCubeApp::ComPtr<ID3D12Resource1> TexturedCubeApp::CreateTexture(const std::string& fileName)
{
    ComPtr<ID3D12Resource1> texture;
    int texWidth = 0, texHeight = 0, channels = 0;
    auto* pImage = stbi_load(fileName.c_str(), &texWidth, &texHeight, &channels, 0);

    // Prepare the Desc of texture resourec from size and format.
    auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_R8G8B8A8_UNORM,
        texWidth, texHeight,
        1,  // The size of array.
        1   // The number of mipmap.
    );

    // Create texture.
    m_device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
    );

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts;
    UINT numRows;
    UINT64 rowSizeBytes, totalBytes;
    m_device->GetCopyableFootprints(&texDesc, 0, 1, 0, &layouts, &numRows, &rowSizeBytes, &totalBytes);
    ComPtr<ID3D12Resource1> stagingBuffer = CreateBuffer(totalBytes, nullptr);

    // Copy the image to staging buffer.
    {
        const UINT imagePitch = texWidth * sizeof(uint32_t);
        void* pBuf;
        CD3DX12_RANGE range(0, 0);
        stagingBuffer->Map(0, &range, &pBuf);
        for (UINT h = 0; h < numRows; h++)
        {
            auto dst = static_cast<char*>(pBuf) + h * rowSizeBytes;
            auto src = pImage + h * imagePitch;
            memcpy(dst, src, imagePitch);
        }
    }

    // Prepare the command.
    ComPtr<ID3D12GraphicsCommandList> command;
    m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&command));
    ComPtr<ID3D12Fence1> fence;
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

    // Transferring command.
    D3D12_TEXTURE_COPY_LOCATION src{}, dst{};
    dst.pResource = texture.Get();
    dst.SubresourceIndex = 0;
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

    src.pResource = stagingBuffer.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint = layouts;
    command->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    // Go to next state after copied.
    auto barrierTex = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    command->ResourceBarrier(1, &barrierTex);

    command->Close();

    // Execute the command.
    ID3D12CommandList* cmds[] = { command.Get() };
    m_commandQueue->ExecuteCommandLists(1, cmds);
    // Arise the signal after executed.
    const UINT64 expected = 1;
    m_commandQueue->Signal(fence.Get(), expected);

    // Wait to finish processing texture.
    while (expected != fence->GetCompletedValue())
    {
        Sleep(1);
    }

    stbi_image_free(pImage);
    return texture;
}

void TexturedCubeApp::PrepareDescriptorHeapForTexturedCubeApp()
{
    // Descriptor heap of CBV/SRV.
    // 0: Shader resource view.
    // 1, 2: Constance buffer view.
    UINT count = FrameBufferCount + 1;
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        count,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        0
    };
    m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_heapSrvCbv));
    m_srvcbvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Descriptor heap of dynamic sampler.
    D3D12_DESCRIPTOR_HEAP_DESC samplerHeapDesc{
        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
        1,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
        0
    };
    m_device->CreateDescriptorHeap(&samplerHeapDesc, IID_PPV_ARGS(&m_heapSampler));
    m_samplerDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}