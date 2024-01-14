#pragma once

#include <vector>
#include <string_view>

#include "linear_algebra/Vec3.hpp"

struct Coordinate{
	f32 x;
	f32 y;
};

struct ParsedObj {
	std::vector<Vec3<f32>> vertices;
	std::vector<Vec3<f32>> vertex_normals;
	std::vector<Coordinate> uv_map;
	std::vector<Vec3<Vec3<i32>>> faces;
};
/*
supports triangulated meshes only with uv and normals included
*/
ParsedObj load_obj(std::string_view file_path);