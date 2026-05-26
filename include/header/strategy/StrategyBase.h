#pragma once

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>

template <typename Derived>
class StrategyBase {
public:
    using CreatorFunc = std::function<std::unique_ptr<Derived>()>;

    virtual std::string getName() const = 0;
    virtual ~StrategyBase() = default;

    static std::unique_ptr<Derived> create(const std::string& name) {
        auto& registry = getRegistry();
        auto it = registry.find(name);
        if (it != registry.end()) {
            return it->second();
        }
        return nullptr;
    }

    static std::vector<std::string> getAvailableStrategies() {
        std::vector<std::string> names;
        for (const auto& pair : getRegistry()) {
            names.push_back(pair.first);
        }
        return names;
    }

    static void registerStrategy(const std::string& name, CreatorFunc func) {
        getRegistry()[name] = std::move(func);
    }

private:
    static std::map<std::string, CreatorFunc>& getRegistry() {
        static std::map<std::string, CreatorFunc> registry;
        return registry;
    }
};

#define REGISTER_STRATEGY(STRATEGY_TYPE, STRATEGY_CLASS) \
    static bool STRATEGY_CLASS##_registered = []() { \
        STRATEGY_TYPE::registerStrategy(#STRATEGY_CLASS, []() { return std::make_unique<STRATEGY_CLASS>(); }); \
        return true; \
    }()

