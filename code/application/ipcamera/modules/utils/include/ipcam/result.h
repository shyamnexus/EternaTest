#pragma once

#include <string>
#include <utility>

namespace ipcam {

// Result type for operation outcomes
template<typename T>
struct Result {
    bool success;
    T value;
    std::string error;
    
    static Result Ok(T val) { 
        return Result{true, std::move(val), ""}; 
    }
    
    static Result Err(const std::string& msg) { 
        return Result{false, T{}, msg}; 
    }
    
    operator bool() const { return success; }
};

// Specialization for void operations
template<>
struct Result<void> {
    bool success;
    std::string error;
    
    static Result Ok() { 
        return Result{true, ""}; 
    }
    
    static Result Err(const std::string& msg) { 
        return Result{false, msg}; 
    }
    
    operator bool() const { return success; }
};

} // namespace ipcam
