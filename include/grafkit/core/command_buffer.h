#ifndef GRAFKIT_COMMAND_H
#define GRAFKIT_COMMAND_H

#include <grafkit/common.h>
#include <grafkit/core/device.h>

namespace Grafkit {
	namespace Core {
		class CommandBuffer {
		public:
			explicit CommandBuffer(const DeviceRef& device, const uint32_t frameIndex);

			virtual ~CommandBuffer();

			[[nodiscard]] VkCommandBuffer operator*() const { return m_commandBuffer; }

			[[nodiscard]] const VkCommandBuffer& GetVkCommandBuffer() const { return m_commandBuffer; }

			[[nodiscard]] uint32_t GetFrameIndex() const { return m_frameIndex; }

			void Reset();
			void End();

		private:
			const DeviceRef m_device;
			const uint32_t m_frameIndex;

			VkCommandBuffer m_commandBuffer;
		};
	} // namespace Core
} // namespace Grafkit

#endif // COMMAND_H
