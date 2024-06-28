#include "stdafx.h"
#include <grafkit/render/animation.h>

using namespace Grafkit::Animation;

size_t Channel::FindKey(float time) const
{
	size_t left = 0;
	size_t right = keys.size() - 1;
	while (left <= right) {
		size_t mid = left + (right - left) / 2;
		if (keys[mid].time == time)
			return mid;
		if (keys[mid].time < time)
			left = mid + 1;
		else
			right = mid - 1;
	}
	return left;
}

void Animation::Update(const Grafkit::TimeInfo& timeInfo)
{
	for (const auto& sampler : samplers) {
		const auto& channel = channels[sampler.input];

		// Find the keyframe
		const size_t keyIndex = channel->FindKey(localTime);

		auto value = glm::vec4(0.0f);
		if (keyIndex >= channel->keys.size() - 1) {
			value = channel->keys[keyIndex].value;
		} else {
			const auto& keyLeft = channel->keys[keyIndex];
			const auto& keyRight = channel->keys[keyIndex + 1];

			// Interpolate
			const float t = (localTime - keyLeft.time) / (keyRight.time - keyLeft.time);

			const auto& leftValue = keyLeft.value;
			const auto& rightValue = keyRight.value;

			// TODO: Implement other interpolation methods
			value = glm::mix(leftValue, rightValue, t);
		}
		// Apply the value to the target
		const auto& target = targets[sampler.output];
		target->Update(value);
	}

	localTime += timeInfo.deltaTime;

	// TOOD: Implement proper looping
	// if (isLooping && localTime > end) {
	// 	localTime = start;
	// }
}
