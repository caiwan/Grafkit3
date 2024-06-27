#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <grafkit/resource/resource.h>
#include <string>

using namespace Grafkit::Resource;

void Grafkit::Resource::ResoureManger::CollectGarbage()
{
	size_t count = 0;
	do {
		for (auto& [kind, resources] : m_resources) {
			for (auto it = resources.begin(); it != resources.end();) {
				if (it->second.use_count() == 1) {
					it = resources.erase(it);
					count++;
				} else {
					++it;
				}
			}
		}
	} while (count > 0);
}
