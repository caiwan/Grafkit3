#ifndef _HELLO_APPLICATION_H_
#define _HELLO_APPLICATION_H_

#include <optional>
#include <string>
#include <vector>
//
#include <grafkit/application.h>
#include <grafkit/core/window.h>
#include <grafkit/render.h>
#include <grafkit/core/pipeline.h>

namespace PlayerApplication
{
	class HelloApplication : public Grafkit::Application
	{
	public:
		HelloApplication();
		virtual ~HelloApplication();

		virtual void Init() override;
		// virtual void Run() override;
		virtual void Update() override;
		virtual void Compute(VkCommandBuffer& commandBuffer) override;
		virtual void Render(VkCommandBuffer& commandBuffer) override;
		virtual void Shutdown() override;

	private:
		Grafkit::Core::PipelinePtr graphicsPipeline;
	};
} // namespace PlayerApplication

#endif // _HELLO_APPLICATION_H_
