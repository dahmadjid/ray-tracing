#include "Obj.hpp"
#include <fstream>
#include <filesystem>
#include <cassert>
#include "utils/Panic.hpp"

// parses "2.00 -1.00 3.00" to Vec3<f32> 
static Vec3<f32> vec3_from_string(const std::string& s) {
    Vec3<f32> out;
    size_t first_space = s.find(' ');
    assert(first_space != std::string::npos);
    size_t second_space = s.find(' ', first_space + 1);
    assert(second_space != std::string::npos);
    out.x = stof(s.substr(0, first_space));
    out.y = stof(s.substr(first_space + 1, second_space - first_space - 1));
    out.z = stof(s.substr(second_space + 1));
    assert(!isnan(out.x));
    assert(!isnan(out.y));
    assert(!isnan(out.z));
    return out;
}

// parses "2.00 -1.00" 
static Coordinate coord_from_string(const std::string& s) {
    size_t first_space = s.find(' ');
    Coordinate out = {
        .x = stof(s.substr(0, first_space)),
        .y = stof(s.substr(first_space + 1)),
    };
    assert(!isnan(out.x));
    assert(!isnan(out.y));
    return out;
}

// parses "2/3/4"
static Vec3<i32> face_index_from_string(const std::string& s) {
    Vec3<i32> out;
    size_t first_slash = s.find('/');
    assert(first_slash != std::string::npos);
    size_t second_slash = s.find('/', first_slash + 1);
    assert(second_slash != std::string::npos);
    out.x = stoi(s.substr(0, first_slash));
    out.y = stoi(s.substr(first_slash + 1, second_slash - first_slash - 1));
    out.z = stoi(s.substr(second_slash + 1));
    return out;
}

ParsedObj load_obj(std::string_view file_path) {
	ParsedObj obj;
    std::ifstream stream;
    stream.open(file_path.data(), std::ios_base::ate | std::ios_base::in);
    if (!stream.good()) {
        panic("Failure reading file {}", file_path);
        fmt::println("current working directory == {}", std::filesystem::current_path().string());
    }
    u32 size = stream.tellg();
    std::string text_data;
    text_data.resize(size);
    stream.seekg(0);
    stream.read(text_data.data(), size);
    stream.close();

    size_t pos = 0;
    while ((pos = text_data.find('\n')) != std::string::npos) {
        std::string line = text_data.substr(0, pos);
        text_data.erase(0, pos + 1);
        
        switch (line[0]) {
        case 'v':
            switch (line[1]) {
            case 't': {
                obj.uv_map.emplace_back(coord_from_string(line.substr(3)));
                break;
            }
            case 'n': {
                obj.vertex_normals.emplace_back(vec3_from_string(line.substr(3)));
                break;
            }
            case ' ': {
                obj.vertices.emplace_back(vec3_from_string(line.substr(2)));
                break;
            }
            default:
                break;
            }
            break;

        case 'f':
            if (line[1] == ' ') {
                // parsing 1/1/1 2/3/1 3/2/4
                std::string s = line.substr(2);
                size_t first_space = s.find(' ');
                assert(first_space != std::string::npos);
                size_t second_space = s.find(' ', first_space + 1);
                assert(second_space != std::string::npos);
                
                obj.faces.push_back(Vec3(
                    face_index_from_string(s.substr(0, first_space)),
                    face_index_from_string(s.substr(first_space + 1, second_space - first_space - 1)),
                    face_index_from_string(s.substr(second_space + 1))
                ));
            }
            break;

        default:
            break;
        }
    }
    return obj;
}