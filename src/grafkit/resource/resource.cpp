#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <string>

#include <grafkit/interface/resource.h>

using namespace Grafkit::Resource;

ResourceLoaderRegistry::LoaderFunc ResourceLoaderRegistry::GetLoader(std::type_index type) const
{
	auto it = m_loaders.find(type);
	if (it != m_loaders.end()) {
		return it->second;
	}
	return nullptr;
}
