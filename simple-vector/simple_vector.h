#pragma once

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include "array_ptr.h"

struct ReserveProxyObj {
    size_t capacity;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return {capacity_to_reserve};
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) :
        items_(size),
        size_(size),
        capacity_(size) {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) :
        SimpleVector(size) {
        std::fill(begin(), end(), value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) :
        SimpleVector(init.size()) {
        Assign(init.begin(), init.end());
    }

    SimpleVector(const ReserveProxyObj& proxy) :
        SimpleVector(proxy.capacity) {
        size_ = 0;
    }

    SimpleVector(const SimpleVector& other) :
        SimpleVector(other.capacity_) {
        size_ = other.size_;
        Assign(other.cbegin(), other.cend());
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto copy(rhs);
            swap(copy);
        }
        return *this;
    }

    SimpleVector(SimpleVector&& other) {
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    };

    SimpleVector& operator=(SimpleVector&& other) {
        items_ = std::move(other.items_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
        return *this;
    };

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            Extend(new_capacity);
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type item) {
        if (size_ == capacity_) {
            Extend(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        items_[size_++] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type value) {
        auto distance = pos - cbegin();

        if (size_ == capacity_) {
            Extend(capacity_ == 0 ? 1 : capacity_ * 2);
        }

        std::copy_backward(std::make_move_iterator(begin() + distance), std::make_move_iterator(end()), end() + 1);
        ++size_;
        items_[distance] = std::move(value);
        return begin() + distance;
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        std::copy(std::make_move_iterator(const_cast<Iterator>(pos) + 1), std::make_move_iterator(end()), const_cast<Iterator>(pos));
        --size_;
        return const_cast<Iterator>(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0u;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Out of range");
        }
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (size_ < new_size && new_size <= capacity_) {
            Fill(end(), begin() + new_size, Type{});
        } else if (new_size > capacity_) {
            Extend(std::max(new_size, capacity_ * 2));
        }

        size_ = new_size;
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return cbegin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return cend();
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }

private:
    template<typename Iterator>
    void Assign(const Iterator first, const Iterator last) {
        std::copy(first, last, begin());
    }

    void Extend(size_t new_capacity) {
        ArrayPtr<Type> ptr(new_capacity);
        std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), ptr.Get());
        items_.swap(ptr);
        capacity_ = new_capacity;
    }

    template< typename Iterator>
    void Fill(Iterator first, Iterator last, Type value)
    {
        for (; first != last; ++first) {
            *first = std::move(value);
        }
    }

    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return lhs <= rhs && rhs <= lhs;
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}

