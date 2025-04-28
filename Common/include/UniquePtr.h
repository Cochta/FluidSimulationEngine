#pragma once

/**
 * @brief A smart pointer that uniquely owns an object.
 * @tparam T The type of the object being managed.
 */
template<typename T>
class UniquePtr
{
private:
    T *_ptr = nullptr; // The managed pointer to the object.

public:
    /**
     * @brief Constructor for UniquePtr that takes an initial raw pointer and transfers ownership.
     * @param ptr The raw pointer to manage.
     */
    constexpr explicit UniquePtr(T *ptr) noexcept: _ptr(ptr)
    {}

    // Disable default constructor, copy constructor, and copy assignment
    UniquePtr() = delete;

    UniquePtr(const UniquePtr &) = delete;

    UniquePtr &operator=(const UniquePtr &) = delete;

    /**
     * @brief Destructor for UniquePtr.
     * Deletes the managed object when the UniquePtr goes out of scope.
     */
    ~UniquePtr() noexcept
    {
        delete _ptr;
    }

    /**
     * @brief Move constructor: Transfer ownership of the managed pointer from another UniquePtr.
     * @param other The source UniquePtr to transfer ownership from.
     */
    constexpr UniquePtr(UniquePtr &&other) noexcept
    {
        _ptr = other._ptr;
        other._ptr = nullptr;
    }

    /**
     * @brief Move assignment: Transfer ownership of the managed pointer from another UniquePtr.
     * @param other The source UniquePtr to transfer ownership from.
     * @return A reference to the modified UniquePtr.
     */
    constexpr UniquePtr &operator=(UniquePtr &&other) noexcept
    {
        if (this != &other)
        {
            delete _ptr;
            _ptr = other._ptr;
            other._ptr = nullptr;
        }
        return *this;
    }

    /**
     * @brief Dereference operator: Get a reference to the managed object.
     * @return A reference to the managed object.
     */
    constexpr T &operator*() const noexcept
    {
        return *_ptr;
    }

    /**
     * @brief Member access operator: Get a pointer to the managed object.
     * @return A pointer to the managed object.
     */
    constexpr T *operator->() const noexcept
    {
        return _ptr;
    }

    /**
     * @brief Get the raw pointer to the managed object.
     * @return A pointer to the managed object.
     */
    constexpr T *Get() const noexcept
    {
        return _ptr;
    }

    /**
     * @brief Check if the UniquePtr is managing a pointer.
     * @return true if the UniquePtr is managing a pointer, false otherwise.
     */
    [[nodiscard]] constexpr bool IsNull() const noexcept
    {
        return _ptr == nullptr;
    }

    template<typename U>
    /**
     * @brief Implicit conversion operator to convert a UniquePtr to a UniquePtr of a different type.
     * @param The target type of the conversion.
     * @return A UniquePtr of the specified target type.
     */
    operator UniquePtr<U>()
    {
        auto n = _ptr;
        _ptr = nullptr;

        return UniquePtr<U>(n);
    }
};

template<typename T>
/**
 * @brief Create a UniquePtr with a new object of type T using constructor arguments.
 * @param args Arguments to construct the object of type T.
 * @return A UniquePtr that uniquely owns the created object.
 */
UniquePtr<T> MakeUnique(T value) noexcept
{
    return UniquePtr<T>(new T(value));
}