#include "dfShader.h"
#include "../util/util.h"

// For DirectX Shader Compiler.
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

// ============================================================================
dfShader::dfShader() {
    vs = nullptr;
    ps = nullptr;
}

// ============================================================================
HRESULT dfShader::loadShader(std::wstring vs_path, std::wstring ps_path)
{
    HRESULT hr;
    hr = CompileShaderFromFile(vs_path, L"vs_6_0", vs, errBlob);
    if (FAILED(hr)) {
        Util::Log((const char*)errBlob->GetBufferPointer());
    }
    hr = CompileShaderFromFile(ps_path, L"ps_6_0", ps, errBlob);
    if (FAILED(hr)) {
        Util::Log((const char*)errBlob->GetBufferPointer());
    }
    return hr;
}

// ============================================================================
HRESULT dfShader::CompileShaderFromFile(
    const std::wstring& filename, const std::wstring& profile, ComPtr<ID3DBlob>& shaderBlob, ComPtr<ID3DBlob>& errorBlob
)
{
    using namespace std::experimental::filesystem;

    path filePath(filename);
    std::ifstream infile(filePath);
    std::vector<char> srcData;
    if(!infile)
        throw std::runtime_error("shader not found.");
    srcData.resize(uint32_t(infile.seekg(0, infile.end).tellg()));
    infile.seekg(0, infile.beg).read(srcData.data(), srcData.size());

    // Compiling process by DXC (DirectX Shader Compiler.)
    ComPtr<IDxcLibrary> library;
    ComPtr<IDxcCompiler> compiler;
    ComPtr<IDxcBlobEncoding> source;
    ComPtr<IDxcOperationResult> dxcResult;

    DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
    library->CreateBlobWithEncodingFromPinned(srcData.data(), UINT(srcData.size()), CP_ACP, &source);
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    LPCWSTR compilerFlags[] = {
#if _DEBUG
        L"/Zi", L"/O0",
#else
        L"/O2" // Optimizing in Release build.
#endif
    };
    compiler->Compile(source.Get(), filePath.wstring().c_str(),
        L"main", profile.c_str(),
        compilerFlags, _countof(compilerFlags),
        nullptr, 0,
        nullptr,
        &dxcResult);

    HRESULT hr;
    dxcResult->GetStatus(&hr);
    if (SUCCEEDED(hr))
    {
        dxcResult->GetResult(
            reinterpret_cast<IDxcBlob**>(shaderBlob.GetAddressOf())
        );
    }
    else
    {
        dxcResult->GetErrorBuffer(
            reinterpret_cast<IDxcBlobEncoding**>(errorBlob.GetAddressOf())
        );
    }
    return hr;
}