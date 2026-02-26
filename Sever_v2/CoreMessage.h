#pragma once
#include "MyParms.h"
#include <memory>
#include <type_traits>

class CoreMessage {
private:
    MyParams params_;

public:
    CoreMessage() = default;
    
    // Explicitly define copy and move constructors
    CoreMessage(const CoreMessage&) = default;
    CoreMessage(CoreMessage&&) = default;
    
    // Explicitly define copy and move assignment operators
    CoreMessage& operator=(const CoreMessage&) = default;
    CoreMessage& operator=(CoreMessage&&) = default;
    
    explicit CoreMessage(const MyParams& params)
        : params_(params) {}
    
    explicit CoreMessage(MyParams&& params)
        : params_(std::move(params)) {}
    
    // Variadic constructor - excludes CoreMessage and MyParams types
    // Uses SFINAE to ensure copy/move constructors take priority
    template<typename First, typename... Rest,
             typename = std::enable_if_t<
                 !std::is_same_v<std::remove_cv_t<std::remove_reference_t<First>>, CoreMessage> &&
                 !std::is_same_v<std::remove_cv_t<std::remove_reference_t<First>>, MyParams>
             >>
    CoreMessage(First&& first, Rest&&... rest)
        : params_(std::forward<First>(first), std::forward<Rest>(rest)...) {}
    
    MyParams getParams() const { 
        return params_; 
    }
    
    void setParams(const MyParams& params) { 
        params_ = params; 
    }
    
    template<typename T>
    void addParam(T&& value) {
        params_.Add(std::forward<T>(value));
    }
    
    template<typename T>
    T getParam(size_t index) const {
        return params_.Get<T>(index);
    }
    
    template<typename T>
    T getParamOrDefault(size_t index, const T& defaultValue) const {
        return params_.GetOrDefault<T>(index, defaultValue);
    }

    template<typename T>
    T getParamOrDefault(size_t index) const {
        return params_.GetOrDefault<T>(index);
    }
    
    size_t getParamCount() const {
        return params_.Count();
    }
    
    std::string toString() const {
        return "CoreMessage[params=" + std::to_string(params_.Count()) + "]";
    }
};

using CoreMessagePtr = std::shared_ptr<CoreMessage>;