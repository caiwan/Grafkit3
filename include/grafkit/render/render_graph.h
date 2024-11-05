#ifndef GRAFKIT_RENDER_RENDER_GRAPH_H
#define GRAFKIT_RENDER_RENDER_GRAPH_H

#include <grafkit/common.h>

namespace Grafkit
{

	GKAPI class RenderStage
	{
	public:
		RenderStage() = default;
		virtual ~RenderStage() = default;

	private:
	};

	GKAPI class RenderGraph
	{
	public:
		RenderGraph() = default;
		virtual ~RenderGraph() = default;
	};

} // namespace Grafkit

#endif // GRAFKIT_RENDER_RENDER_GRAPH_H
