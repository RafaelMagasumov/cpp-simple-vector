#pragma once

#include <utility>
#include <cassert>
#include <initializer_list>
#include <stdexcept> // для throw std::out_of_range
#include <iterator>
#include "array_ptr.h"




class ReserveProxyObj {
public:
	explicit ReserveProxyObj(size_t capacity_to_reserve) : capacity_(capacity_to_reserve) {}

	size_t Reserve_capacity() {// метод получения емкости
		return capacity_;
	}
private:
	size_t capacity_;
};


ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj(capacity_to_reserve);
}



template <typename Type>
class SimpleVector {
public:
	using Iterator = Type * ;
	using ConstIterator = const Type*;

	SimpleVector() noexcept = default;



	// Создаёт вектор из size элементов, инициализированных значением по умолчанию
	explicit SimpleVector(size_t size) : size_(size), capacity_(size) {
		ArrayPtr<Type> new_data(size);
		simple_vector_.swap(new_data);
		for (size_t i = 0; i < size_; ++i) {
			simple_vector_[i] = std::move(Type{});
		}
	}

	// Создаёт вектор из size элементов, инициализированных значением value
	SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size) {
		ArrayPtr<Type> new_data(size);
		simple_vector_.swap(new_data);
		for (size_t i = 0; i < size_; ++i) {
			simple_vector_[i] = std::move(value);
		}

	}

	// констурктор для резервации capacity 
	explicit SimpleVector(ReserveProxyObj capacity_to_reserve) {
		Reserve(capacity_to_reserve.Reserve_capacity());
		size_ = 0;
	}

	// Создаёт вектор из std::initializer_list
	SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(init.size()) {
		ArrayPtr<Type> new_data(size_);
		simple_vector_.swap(new_data);
		size_t i = 0;
		for (const auto& type : init) {
			simple_vector_[i++] = std::move(type);
		}
	}

	// конструктор копирования по конустантной ссылке 
	SimpleVector(const SimpleVector& other) : size_(other.GetSize()), capacity_(other.GetCapacity()) {
		ArrayPtr<Type> new_data(size_);
		std::copy(other.begin(), other.end(), &new_data[0]); // Можно ли написать std::copy(other.begin(), other.end(), new_data.Get());?
		simple_vector_.swap(new_data);

	}

	// аналог конструктора копирования с использованием rvalue
	SimpleVector(SimpleVector&& other) : size_(other.GetSize()), capacity_(other.GetCapacity()) {
		ArrayPtr<Type> new_data(size_);

		std::move(std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()), new_data.Get());

		simple_vector_.swap(new_data);
		other.Clear();

	}

	// оператор присваивания по ссылке 
	SimpleVector& operator=(const SimpleVector& rhs) {
		if (this == &rhs) return *this;

		SimpleVector temp(rhs); // копирование
		swap(temp);
		return *this;
	}

	// оператор присваивания для ссылки rvalue
	SimpleVector& operator=(SimpleVector&& rhs) {
		if (this == &rhs) return *this;
		SimpleVector temp(std::move(rhs));
		swap(temp);
		return *this;
	}

	// Добавляет элемент в конец вектора
	// При нехватке места увеличивает вдвое вместимость вектора
	void PushBack(const Type& item) {
		if (size_ == capacity_) {
			size_t new_capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
			ArrayPtr<Type> new_data(new_capacity_);
			std::copy(begin(), end(), new_data.Get());
			new_data[size_] = item;
			++size_;
			capacity_ = new_capacity_;
			simple_vector_.swap(new_data);
			// new_capacity удалит деструктор в array_ptr.h
		}
		else {
			simple_vector_[size_] = item;
			++size_;

		}
	}

	// аналог push_back для rvalue ссылки 
	void PushBack(Type&& item) {
		if (size_ == capacity_) {
			size_t new_capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
			ArrayPtr<Type> new_data(new_capacity_); // поменять копирование на присваивание
			std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_data.Get());
			//  std::move(begin(), end(), new_data.Get());
			new_data[size_] = std::move(item);
			++size_;
			capacity_ = new_capacity_;
			simple_vector_.swap(new_data);
			// new_capacity удалит деструктор в array_ptr.h
		}
		else {
			simple_vector_[size_] = std::move(item);
			++size_;

		}
	}


	// Вставляет значение value в позицию pos.
	// Возвращает итератор на вставленное значение
	// Если перед вставкой значения вектор был заполнен полностью,
	// вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
	Iterator Insert(ConstIterator pos, const Type& value) {

		size_t new_pos = pos - simple_vector_.Get(); // указывает на позицию куда надо вставить элемент

		if (size_ == capacity_) {
			size_t new_capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
			ArrayPtr<Type> new_data(new_capacity_);

			std::copy(begin(), begin() + new_pos, new_data.Get());
			new_data[new_pos] = value;
			std::copy(begin() + new_pos, end(), new_data.Get() + new_pos + 1);
			++size_;
			capacity_ = new_capacity_;
			simple_vector_.swap(new_data); // size_ И capacity_ прописаны в operator= 
		}
		else {
			std::copy_backward(begin(), end(), end());
			simple_vector_[new_pos] = value;
			++size_;
		}
		return begin() + new_pos;
	}

	// аналог insert для rvalue ссылки 
	Iterator Insert(ConstIterator pos, Type&& value) {

		size_t new_pos = pos - simple_vector_.Get(); // указывает на позицию куда надо вставить элемент

		if (size_ == capacity_) {
			size_t new_capacity_ = (capacity_ == 0 ? 1 : capacity_ * 2);
			ArrayPtr<Type> new_data(new_capacity_);

			std::move(std::make_move_iterator(begin()), std::make_move_iterator(begin() + new_pos), new_data.Get());
			new_data[new_pos] = std::move(value);
			std::move(std::make_move_iterator(begin() + new_pos), std::make_move_iterator(end()), new_data.Get() + new_pos + 1);
			++size_;
			capacity_ = new_capacity_;
			simple_vector_.swap(new_data); // size_ И capacity_ прописаны в operator= 
		}
		else {
			std::move_backward(std::make_move_iterator(begin()), std::make_move_iterator(end()), end());
			simple_vector_[new_pos] = std::move(value);
			++size_;
		}
		return begin() + new_pos;
	}



	// "Удаляет" последний элемент вектора. Вектор не должен быть пустым
	void PopBack() noexcept {
		if (size_ > 0) {
			--size_;
		}
	}

	// Удаляет элемент вектора в указанной позиции
	Iterator Erase(ConstIterator pos) {
		Iterator new_pos = begin() + (pos - cbegin());
		std::move(std::make_move_iterator(new_pos + 1), std::make_move_iterator(end()), new_pos);
		--size_;
		return new_pos;
	}


	// Обменивает значение с другим вектором
	void swap(SimpleVector& other) noexcept {
		simple_vector_.swap(other.simple_vector_);
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
		return size_ == 0;
	}

	// Возвращает ссылку на элемент с индексом index
	Type& operator[](size_t index) noexcept {
		return simple_vector_[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	const Type& operator[](size_t index) const noexcept {
		return simple_vector_[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	Type& At(size_t index) {
		return index >= size_ ? throw std::out_of_range("index_uot_of_range") : simple_vector_[index];
	}

	// Возвращает константную ссылку на элемент с индексом index
	// Выбрасывает исключение std::out_of_range, если index >= size
	const Type& At(size_t index) const {
		return index >= size_ ? (throw std::out_of_range("index_uot_of_range")) : simple_vector_[index];
	}

	// Обнуляет размер массива, не изменяя его вместимость
	void Clear() noexcept {
		size_ = 0;
	}

	// Изменяет размер массива.
	// При увеличении размера новые элементы получают значение по умолчанию для типа Type
	void Resize(size_t new_size) {
		if (new_size > size_) {
			ArrayPtr<Type> new_data(new_size);
			for (size_t i = 0; i < size_; ++i) {
				new_data[i] = std::move(simple_vector_[i]);
			}
			for (size_t i = size_; i < new_size; ++i) {
				new_data[i] = std::move(Type{});
			}
			simple_vector_.swap(new_data);
			capacity_ = new_size;
		}

		if (new_size <= size_) {
			for (auto it = begin() + new_size; it != begin() + size_; ++it) {
				*(it) = std::move(Type{});
			}
		}

		size_ = new_size;
	}



	// Возвращает итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator begin() noexcept {
		return simple_vector_.Get();
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	Iterator end() noexcept {
		return simple_vector_.Get() + size_;
	}

	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator begin() const noexcept {
		return simple_vector_.Get();
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator end() const noexcept {
		return simple_vector_.Get() + size_;
	}

	// Возвращает константный итератор на начало массива
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cbegin() const noexcept {
		return begin();
	}

	// Возвращает итератор на элемент, следующий за последним
	// Для пустого массива может быть равен (или не равен) nullptr
	ConstIterator cend() const noexcept {
		return end();
	}


	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			ArrayPtr<Type> new_data(new_capacity);
			//std::copy(begin(), end(), new_data.Get());
			std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_data.Get());
			simple_vector_.swap(new_data);
			capacity_ = new_capacity;
		}
	}


private:
	size_t size_ = 0;
	size_t capacity_ = 0;
	ArrayPtr<Type> simple_vector_;
};




template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	if (lhs.GetSize() != rhs.GetSize()) return false;
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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
	// lhs <= rhs
	return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs < rhs);
}