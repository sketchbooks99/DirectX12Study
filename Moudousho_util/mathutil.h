#pragma once

#include <DirectXMath.h>

using Vector2 = DirectX::XMFLOAT2; 
using Vector3 = DirectX::XMFLOAT3;
using Vector4 = DirectX::XMFLOAT4;
using Vector = DirectX::XMVECTOR;
using Color = DirectX::XMFLOAT4;

using Matrix3x3 = DirectX::XMFLOAT3X3;
using Matrix4x4 = DirectX::XMFLOAT4X4;

namespace MathUtil {
	inline float* VecToFloatArray(DirectX::XMVECTOR vec) {
		std::vector<float> data;
		for (int i = 0; i < sizeof(vec) / sizeof(float); i++) {
			data.push_back(DirectX::XMVectorGetByIndex(vec, i));
		}
		return &data[0];
	}
}