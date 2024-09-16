#include "stdafx.h"

#include <nlohmann/json.hpp>

#include <glm/glm.hpp>

// NOLINTBEGIN(readability-identifier-naming) The naming has to match with glm and nlohmann_json
namespace glm {

	void to_json(nlohmann::json& j, const vec2& v) { j = nlohmann::json { { "x", v.x }, { "y", v.y } }; }

	void from_json(const nlohmann::json& j, vec2& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
	}

	void to_json(nlohmann::json& j, const vec3& v) { j = nlohmann::json { { "x", v.x }, { "y", v.y }, { "z", v.z } }; }

	void from_json(const nlohmann::json& j, vec3& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
	}

	void to_json(nlohmann::json& j, const vec4& v)
	{
		j = nlohmann::json { { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } };
	}

	void from_json(const nlohmann::json& j, vec4& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
		j.at("w").get_to(v.w);
	}

	void to_json(nlohmann::json& json, const mat4& m)
	{
		std::array<std::array<float, 4>, 4> arr;
		for (uint32_t i = 0; i < 4; ++i) {
			for (uint32_t j = 0; j < 4; ++j) {
				arr[i][j] = m[static_cast<int>(i)][static_cast<int>(j)];
			}
		}
		json = arr;
	}

	void from_json(const nlohmann::json& json, mat4& m)
	{
		std::array<std::array<float, 4>, 4> arr = json;
		for (uint32_t i = 0; i < 4; ++i) {
			for (uint32_t j = 0; j < 4; ++j) {
				m[static_cast<int>(i)][static_cast<int>(j)] = arr[i][j];
			}
		}
	}

	void to_json(nlohmann::json& j, const ivec2& v) { j = nlohmann::json { { "x", v.x }, { "y", v.y } }; }

	void from_json(const nlohmann::json& j, ivec2& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
	}

	void to_json(nlohmann::json& j, const ivec3& v) { j = nlohmann::json { { "x", v.x }, { "y", v.y }, { "z", v.z } }; }

	void from_json(const nlohmann::json& j, ivec3& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
	}

	void to_json(nlohmann::json& j, const ivec4& v)
	{
		j = nlohmann::json { { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } };
	}

	void from_json(const nlohmann::json& j, ivec4& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
		j.at("w").get_to(v.w);
	}

	void to_json(nlohmann::json& j, const uvec2& v) { j = nlohmann::json { { "x", v.x }, { "y", v.y } }; }

	void from_json(const nlohmann::json& j, uvec2& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
	}

	void to_json(nlohmann::json& j, const uvec3& v) { j = nlohmann::json { { "x", v.x }, { "y", v.y }, { "z", v.z } }; }

	void from_json(const nlohmann::json& j, uvec3& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
	}

	void to_json(nlohmann::json& j, const uvec4& v)
	{
		j = nlohmann::json { { "x", v.x }, { "y", v.y }, { "z", v.z }, { "w", v.w } };
	}

	void from_json(const nlohmann::json& j, uvec4& v)
	{
		j.at("x").get_to(v.x);
		j.at("y").get_to(v.y);
		j.at("z").get_to(v.z);
		j.at("w").get_to(v.w);
	}

	void to_json(nlohmann::json& j, const u8vec2& q) { j = nlohmann::json { { "x", q.x }, { "y", q.y } }; }

	void from_json(const nlohmann::json& j, u8vec2& q)
	{
		j.at("x").get_to(q.x);
		j.at("y").get_to(q.y);
	}

	void to_json(nlohmann::json& j, const u8vec3& q)
	{
		j = nlohmann::json { { "x", q.x }, { "y", q.y }, { "z", q.z } };
	}

	void from_json(const nlohmann::json& j, u8vec3& q)
	{
		j.at("x").get_to(q.x);
		j.at("y").get_to(q.y);
		j.at("z").get_to(q.z);
	}

	void to_json(nlohmann::json& j, const u8vec4& q)
	{
		j = nlohmann::json { { "x", q.x }, { "y", q.y }, { "z", q.z }, { "w", q.w } };
	}

	void from_json(const nlohmann::json& j, u8vec4& q)
	{
		j.at("x").get_to(q.x);
		j.at("y").get_to(q.y);
		j.at("z").get_to(q.z);
		j.at("w").get_to(q.w);
	}

} // namespace glm
// NOLINTEND(readability-identifier-naming)
