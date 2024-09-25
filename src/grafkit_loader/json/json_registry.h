#ifndef JSON_DESERIALIZER_H
#define JSON_DESERIALIZER_H

#include <any>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

namespace Grafkit::Serialization {
	using DeserializerFunc = std::function<void(const nlohmann::json&, std::any&)>;

	class JsonSerializerRegistry {
	public:
		static JsonSerializerRegistry& Instance()
		{
			static JsonSerializerRegistry instance;
			return instance;
		}
		JsonSerializerRegistry(const JsonSerializerRegistry&) = delete;
		JsonSerializerRegistry& operator=(const JsonSerializerRegistry&) = delete;

		template <typename T> void Register(DeserializerFunc deserializer)
		{
			Register(typeid(T), std::move(deserializer));
		}
		void Register(std::type_index type, DeserializerFunc deserializer);

		template <typename T> void Deserialize(const nlohmann::json& json, std::any& object)
		{
			Deserialize(json, typeid(T), object);
		}

		void Deserialize(const nlohmann::json& json, std::type_index type, std::any& object);

	private:
		JsonSerializerRegistry();
		~JsonSerializerRegistry() = default;

		std::unordered_map<std::string, DeserializerFunc> m_deserializers;
	};

} // namespace Grafkit::Serialization

#endif // JSON_DESERIALIZER_H
