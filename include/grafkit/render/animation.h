#ifndef GRAFKIT_RENDER_ANIMATION_H
#define GRAFKIT_RENDER_ANIMATION_H

#include <grafkit/common.h>

// TOOD: There has to be a way to animat via code and not only via [glTF]

namespace Grafkit::Animation {
	enum class Interpolation { STEP, LINEAR, SMOOTH, CUBICSPLINE };

	// This has to be an abstract class that updates the approriate target
	class Target {
	public:
		virtual void Update(const glm::vec4& value) = 0;
	};

	// TOOD: This has to be memory-aligned for multiprocess
	struct Key {
		float time;
		Interpolation interpolation = Interpolation::LINEAR;
		glm::vec4 value;
	};

	struct Channel {
		uint32_t id;
		std::vector<Key> keys;

		size_t FindKey(float time) const;
	};

	// This is less likely needed
	// Multiple targets can affect the same node (besides translation, rotation, ... ) such as constraints(?) and
	// other modifiers One channel can affect multiple targets, [possibly]
	struct Sampler {
		uint32_t id;
		uint32_t input; // = channelIndex
		uint32_t output; // = targetIndex
	};

	struct Animation {
		uint32_t id;
		std::vector<ChannelPtr> channels;
		std::vector<TargetPtr> targets;
		std::vector<Sampler> samplers;

		float localTime = 0.0f;
		bool isLooping = false;

		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();

		void Update(const Grafkit::TimeInfo& timeInfo);
	};

} // namespace Grafkit::Animation

#endif // GRAFKIT_RENDER_ANIMATION_H
