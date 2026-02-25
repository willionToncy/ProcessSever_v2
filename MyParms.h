#pragma once
#include <iostream>
#include <any>
#include <vector>
#include <string>
#include <functional>
#include <typeindex>
#include <unordered_map>


//此文件为自定义的可变参数列表，不需要继承，采用予以擦除进行操作
class MyParams {
private:
    std::vector<std::any> params_;

    // 类型特定的处理器注册表
    static auto& GetProcessors() {
        static std::unordered_map<std::type_index, std::function<void(const std::any&)>> processors;
        return processors;
    }

public:
    MyParams() = default;

    // 可变参数构造函数
    template<typename... Args>
    MyParams(Args&&... args) {
        (params_.push_back(std::forward<Args>(args)), ...);
    }

    // 添加参数
    template<typename T>
    void Add(T&& value) {
        params_.push_back(std::forward<T>(value));
    }

    // 获取参数数量
    size_t Count() const { return params_.size(); }

    // 直接取值（不安全，会抛异常）
    template<typename T>
    T Get(size_t index) const {
        return std::any_cast<T>(params_.at(index));
    }

    // 安全取值
    template<typename T>
    bool TryGet(size_t index, T& value) const {
        if (index >= params_.size()) return false;
        try {
            value = std::any_cast<T>(params_[index]);
            return true;
        }
        catch (...) {
            return false;
        }
    }

    // 取值带默认值
    template<typename T>
    T GetOrDefault(size_t index, T&& defaultValue = T{}) const {
        T result = std::forward<T>(defaultValue);
        TryGet(index, result);
        return result;
    }

    // 类型检查
    template<typename T>
    bool IsType(size_t index) const {
        if (index >= params_.size()) return false;
        return params_[index].type() == typeid(T);
    }

    // 获取类型名称
    std::string GetTypeName(size_t index) const {
        if (index >= params_.size()) return "out_of_range";
        return params_[index].type().name();
    }

    // 注册自定义类型的处理器
    template<typename T>
    static void RegisterProcessor(std::function<void(const T&)> processor) {
        GetProcessors()[typeid(T)] = [processor](const std::any& any) {
            processor(std::any_cast<const T&>(any));
            };
    }

    // 访问并处理所有参数
    void ForEach(const std::function<void(const std::any&)>& visitor) const {
        for (const auto& param : params_) {
            visitor(param);
        }
    }
};
