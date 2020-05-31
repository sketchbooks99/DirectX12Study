#pragma once

#include "../util/mathutil.h"
#include "../util/util.h"
#include "dfShader.h"

class dfMesh {
	struct Vertex {
		Vector4 Position;
		Vector4 Color;
		Vector3 Normal;
		Vector2 UV;
	};

private:
	dfShader shader;
	std::vector<Vertex> verts;
	std::vector<uint32_t> indices;
};