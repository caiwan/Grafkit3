#ifndef JSON_GLM_HPP
#define JSON_GLM_HPP

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

#include <nlohmann/json.hpp>

// NOLINTBEGIN(readability-identifier-naming) The naming has to match with glm and nlohmann_json
namespace glm {

	void to_json(nlohmann::json& j, const vec2& v);
	void from_json(const nlohmann::json& j, vec2& v);

	void to_json(nlohmann::json& j, const vec3& v);
	void from_json(const nlohmann::json& j, vec3& v);

	void to_json(nlohmann::json& j, const vec4& v);
	void from_json(const nlohmann::json& j, vec4& v);

	void to_json(nlohmann::json& j, const mat4& m);
	void from_json(const nlohmann::json& j, mat4& m);

	void to_json(nlohmann::json& j, const ivec2& v);
	void from_json(const nlohmann::json& j, ivec2& v);

	void to_json(nlohmann::json& j, const ivec3& v);
	void from_json(const nlohmann::json& j, ivec3& v);

	void to_json(nlohmann::json& j, const ivec4& v);
	void from_json(const nlohmann::json& j, ivec4& v);

	void to_json(nlohmann::json& j, const uvec2& v);
	void from_json(const nlohmann::json& j, uvec2& v);

	void to_json(nlohmann::json& j, const uvec3& v);
	void from_json(const nlohmann::json& j, uvec3& v);

	void to_json(nlohmann::json& j, const uvec4& v);
	void from_json(const nlohmann::json& j, uvec4& v);

	void to_json(nlohmann::json& j, const u8vec2& v);
	void from_json(const nlohmann::json& j, u8vec2& v);

	void to_json(nlohmann::json& j, const u8vec3& v);
	void from_json(const nlohmann::json& j, u8vec3& v);

	void to_json(nlohmann::json& j, const u8vec4& v);
	void from_json(const nlohmann::json& j, u8vec4& v);

} // namespace glm
  // NOLINTEND(readability-identifier-naming)

#endif // JSON_GLM_HPP
