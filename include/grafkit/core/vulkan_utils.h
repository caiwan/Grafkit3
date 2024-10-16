#ifndef __GRAFKIT_CORE_VK_UTILS_H__
#define __GRAFKIT_CORE_VK_UTILS_H__

#include <grafkit/common.h>
#include <string>

namespace Grafkit::Core {

#define VK_ENUM_AS_STR(r) \
	case VK_##r:          \
		return #r

	inline std::string VkResultAsString(VkResult errorCode)
	{
		switch (errorCode) {
			VK_ENUM_AS_STR(NOT_READY);
			VK_ENUM_AS_STR(TIMEOUT);
			VK_ENUM_AS_STR(EVENT_SET);
			VK_ENUM_AS_STR(EVENT_RESET);
			VK_ENUM_AS_STR(INCOMPLETE);
			VK_ENUM_AS_STR(ERROR_OUT_OF_HOST_MEMORY);
			VK_ENUM_AS_STR(ERROR_OUT_OF_DEVICE_MEMORY);
			VK_ENUM_AS_STR(ERROR_INITIALIZATION_FAILED);
			VK_ENUM_AS_STR(ERROR_DEVICE_LOST);
			VK_ENUM_AS_STR(ERROR_MEMORY_MAP_FAILED);
			VK_ENUM_AS_STR(ERROR_LAYER_NOT_PRESENT);
			VK_ENUM_AS_STR(ERROR_EXTENSION_NOT_PRESENT);
			VK_ENUM_AS_STR(ERROR_FEATURE_NOT_PRESENT);
			VK_ENUM_AS_STR(ERROR_INCOMPATIBLE_DRIVER);
			VK_ENUM_AS_STR(ERROR_TOO_MANY_OBJECTS);
			VK_ENUM_AS_STR(ERROR_FORMAT_NOT_SUPPORTED);
			VK_ENUM_AS_STR(ERROR_SURFACE_LOST_KHR);
			VK_ENUM_AS_STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			VK_ENUM_AS_STR(SUBOPTIMAL_KHR);
			VK_ENUM_AS_STR(ERROR_OUT_OF_DATE_KHR);
			VK_ENUM_AS_STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			VK_ENUM_AS_STR(ERROR_VALIDATION_FAILED_EXT);
			VK_ENUM_AS_STR(ERROR_INVALID_SHADER_NV);
			VK_ENUM_AS_STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
		default:
			return "UNKNOWN_ERROR";
		}
	}

#undef VK_ENUM_AS_STR

} // namespace Grafkit::Core

// MARK: Macros
#ifdef _DEBUG
#define VK_CHECK_RESULT(f)                                                                                         \
	{                                                                                                              \
		VkResult result = (f);                                                                                     \
		if (result != VK_SUCCESS) {                                                                                \
			std::stringstream errorMessage;                                                                        \
			errorMessage << "Fatal: VkResult is " << Grafkit::Core::VkResultAsString(result) << " in " << __FILE__ \
						 << " at line " << std::to_string(__LINE__);                                               \
			throw std::runtime_error(errorMessage.str());                                                          \
		}                                                                                                          \
	}
#else
#define VK_CHECK_RESULT(f)                                                                     \
	{                                                                                          \
		VkResult result = (f);                                                                 \
		if (result != VK_SUCCESS) {                                                            \
			std::stringstream errorMessage;                                                    \
			errorMessage << "Fatal: VkResult is " << Grafkit::Core::VkResultAsString(result)); \
			throw std::runtime_error(errorMessage.str());                                      \
		}                                                                                      \
	}
#endif // _DEBUG

#endif // __GRAFKIT_CORE_VK_UTILS_H__
