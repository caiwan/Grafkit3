#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <grafkit/resource/resource.h>
#include <string>

using namespace Grafkit::Resource;

ResourceLoaderRegistry::LoaderFunc ResourceLoaderRegistry::GetLoader(std::type_index type) const
{
	auto it = loaders.find(type);
	if (it != loaders.end()) {
		return it->second;
	}
	return nullptr;
}
