/**
 * @file json_serializers.h
 * @brief Json serialization functions
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated at: 2025-03-26 12:50:25
 * Source files:
 *   - animation_desc.gen.yaml
 *   - image_desc.gen.yaml
 *   - material_desc.gen.yaml
 *   - mesh_desc.gen.yaml
 *   - scene_desc.gen.yaml
 * Template file: json_header.j2
 */

// NOLINTBEGIN(readability-identifier-naming) The naming has to match with nlohmann_json
#include <grafkit/common.h>
#include <map>
#include <span>
#include <string>
#include <vector>

#include "json/json_glm.h"
#include <grafkit/descriptors/animation_desc.h>
#include <grafkit/descriptors/image_desc.h>
#include <grafkit/descriptors/material_desc.h>
#include <grafkit/descriptors/mesh_desc.h>
#include <grafkit/descriptors/scene_desc.h>

// MARK: animation_desc
namespace Grafkit::Resource
{

} // namespace Grafkit::Resource

// MARK: image_desc
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const ImageDesc &obj);
	void from_json(const nlohmann::json &j, ImageDesc &obj);

	void to_json(nlohmann::json &j, const SolidImageDesc &obj);
	void from_json(const nlohmann::json &j, SolidImageDesc &obj);

	void to_json(nlohmann::json &j, const CheckerImageDesc &obj);
	void from_json(const nlohmann::json &j, CheckerImageDesc &obj);

} // namespace Grafkit::Resource

// MARK: material_desc
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const MaterialDesc &obj);
	void from_json(const nlohmann::json &j, MaterialDesc &obj);

} // namespace Grafkit::Resource

// MARK: mesh_desc
namespace Grafkit::Resource
{
	void to_json(nlohmann::json &j, const PrimitiveDesc &obj);
	void from_json(const nlohmann::json &j, PrimitiveDesc &obj);

	void to_json(nlohmann::json &j, const MeshDesc &obj);
	void from_json(const nlohmann::json &j, MeshDesc &obj);

	void to_json(nlohmann::json &j, const PrimitiveDescV2 &obj);
	void from_json(const nlohmann::json &j, PrimitiveDescV2 &obj);

	void to_json(nlohmann::json &j, const MeshDescV2 &obj);
	void from_json(const nlohmann::json &j, MeshDescV2 &obj);

} // namespace Grafkit::Resource

// MARK: scene_desc
namespace Grafkit::Resource
{
} // namespace Grafkit::Resource

// NOLINTEND(readability-identifier-naming)
