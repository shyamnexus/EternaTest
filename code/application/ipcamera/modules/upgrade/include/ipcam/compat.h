/**
 * @file compat.h
 * @brief C++17 compatibility layer for C++20/23 features
 * 
 * Provides fallback implementations for std::expected and std::span
 * when compiling with older C++ standards.
 */

#pragma once

#include <variant>
#include <optional>
#include <string>
#include <cstddef>
#include <cstdint>

namespace ipcam::upgrade {

// =============================================================================
// Simple Expected<T, E> implementation for C++17
// =============================================================================

template<typename T, typename E>
class Expected {
public:
    // Success constructors
    Expected(const T& value) : data_(value), hasValue_(true) {}
    Expected(T&& value) : data_(std::move(value)), hasValue_(true) {}
    
    // Error constructors
    struct UnexpectedTag {};
    Expected(UnexpectedTag, const E& error) : error_(error), hasValue_(false) {}
    Expected(UnexpectedTag, E&& error) : error_(std::move(error)), hasValue_(false) {}
    
    // Check if has value
    bool has_value() const { return hasValue_; }
    explicit operator bool() const { return hasValue_; }
    
    // Access value (undefined if no value)
    T& value() { return data_; }
    const T& value() const { return data_; }
    
    T& operator*() { return data_; }
    const T& operator*() const { return data_; }
    
    T* operator->() { return &data_; }
    const T* operator->() const { return &data_; }
    
    // Access error (undefined if has value)
    E& error() { return error_; }
    const E& error() const { return error_; }
    
    // Value or default
    T value_or(T&& defaultValue) const {
        return hasValue_ ? data_ : std::forward<T>(defaultValue);
    }

private:
    T data_{};
    E error_{};
    bool hasValue_;
};

// =============================================================================
// Specialization for Expected<void, E> (operations that return success/failure)
// =============================================================================

template<typename E>
class Expected<void, E> {
public:
    // Success constructor
    Expected() : hasValue_(true) {}
    
    // Error constructors
    struct UnexpectedTag {};
    Expected(UnexpectedTag, const E& error) : error_(error), hasValue_(false) {}
    Expected(UnexpectedTag, E&& error) : error_(std::move(error)), hasValue_(false) {}
    
    // Check if has value (success)
    bool has_value() const { return hasValue_; }
    explicit operator bool() const { return hasValue_; }
    
    // Access error (undefined if has value)
    E& error() { return error_; }
    const E& error() const { return error_; }

private:
    E error_{};
    bool hasValue_;
};

// Helper to create unexpected (error) value
template<typename E>
auto makeUnexpected(E&& error) {
    return [e = std::forward<E>(error)]<typename T>() mutable {
        return Expected<T, E>(typename Expected<T, E>::UnexpectedTag{}, std::move(e));
    };
}

// =============================================================================
// Simple Span<T> implementation for C++17
// =============================================================================

template<typename T>
class Span {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using const_iterator = const_pointer;

    // Constructors
    constexpr Span() noexcept : data_(nullptr), size_(0) {}
    constexpr Span(pointer ptr, size_type count) : data_(ptr), size_(count) {}
    constexpr Span(pointer first, pointer last) : data_(first), size_(last - first) {}
    
    template<std::size_t N>
    constexpr Span(T (&arr)[N]) noexcept : data_(arr), size_(N) {}
    
    template<typename Container>
    constexpr Span(Container& cont) : data_(cont.data()), size_(cont.size()) {}
    
    template<typename Container>
    constexpr Span(const Container& cont) : data_(cont.data()), size_(cont.size()) {}
    
    // Element access
    constexpr reference operator[](size_type idx) const { return data_[idx]; }
    constexpr reference front() const { return data_[0]; }
    constexpr reference back() const { return data_[size_ - 1]; }
    constexpr pointer data() const noexcept { return data_; }
    
    // Iterators
    constexpr iterator begin() const noexcept { return data_; }
    constexpr iterator end() const noexcept { return data_ + size_; }
    constexpr const_iterator cbegin() const noexcept { return data_; }
    constexpr const_iterator cend() const noexcept { return data_ + size_; }
    
    // Observers
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type size_bytes() const noexcept { return size_ * sizeof(T); }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    
    // Subviews
    constexpr Span<T> subspan(size_type offset, size_type count = static_cast<size_type>(-1)) const {
        if (count == static_cast<size_type>(-1)) {
            count = size_ - offset;
        }
        return Span<T>(data_ + offset, count);
    }
    
    constexpr Span<T> first(size_type count) const {
        return Span<T>(data_, count);
    }
    
    constexpr Span<T> last(size_type count) const {
        return Span<T>(data_ + size_ - count, count);
    }

private:
    pointer data_;
    size_type size_;
};

// Deduction guides
template<typename T, std::size_t N>
Span(T (&)[N]) -> Span<T>;

template<typename Container>
Span(Container&) -> Span<typename Container::value_type>;

template<typename Container>
Span(const Container&) -> Span<const typename Container::value_type>;

} // namespace ipcam::upgrade
