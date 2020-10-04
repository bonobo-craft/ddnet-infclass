#pragma once

#include <string>
#include "json_fwd.hpp"

class Localization {
public:
	static Localization& getInstance();
	void load(const std::string& filepath);
	std::string localize(const std::string& text, const std::string& language, bool fallback = false);
private:
	Localization() = default;
	Localization(const Localization&) = delete;
	void operator=(const Localization&) = delete;
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique2(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

	
	std::unique_ptr<nlohmann::json> json;
};

inline std::string localize(const std::string& text, const std::string& language) {
	return Localization::getInstance().localize(text, language);
}