/**
 * @file image_desc.h
 * @brief image_desc descriptor
 *
 * This file has been automatically generated and should not be modified.
 *
 * Generated on: 2024-09-24 13:10:01
 * Source file:
 */

#ifndef __IMAGE_DESC_GENERATED_H__
#define __IMAGE_DESC_GENERATED_H__

#include <grafkit/common.h>
#include <span>
#include <string>
#include <vector>

/* image_desc */
namespace Grafkit::Resource
{
	enum class ImageFormat
	{
		RGB = 1,
		RGBA = 2,
		Grayscale = 3,
	};

	struct ImageDesc
	{
		std::vector<uint8_t> image;
		glm::uvec3 size;
		ImageFormat format;
		uint32_t channels;
		bool useMipmap = false;
	};

	struct SolidImageDesc
	{
		glm::u8vec4 color;
	};

	struct CheckerImageDesc
	{
		glm::uvec3 size;
		glm::uvec2 divisions;
		glm::u8vec4 color1;
		glm::u8vec4 color2;
		bool useMipmap = false;
	};

} // namespace Grafkit::Resource
#endif // __IMAGE_DESC_GENERATED_H__
