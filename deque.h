#pragma once

#include <initializer_list>
#include <algorithm>
#include <memory>

class Deque {
public:
    Deque();

    Deque(const Deque& rhs);

    Deque(Deque&& rhs);

    explicit Deque(size_t size);

    Deque(std::initializer_list<int> list);

    Deque& operator=(Deque rhs);

    void Swap(Deque& rhs);

    void PushBack(int value);

    void PopBack();

    void PushFront(int value);

    void PopFront();

    int& operator[](size_t ind);

    int operator[](size_t ind) const;

    size_t Size() const;

    void Clear();

private:
    const size_t block_size_ = 512;
    size_t left_ = 0;
    size_t right_ = 0;
    size_t blocks_ = 0;
    std::unique_ptr<std::unique_ptr<int[]>[]> data_;

    void Convert(size_t index, size_t& block_index, size_t& index_inside) const;

    int& IntByIndex(size_t index);

    int IntByIndex(size_t index) const;

    size_t BlockByIndex(size_t index) const;

    void Assign(size_t size);

    void Reallocate(size_t new_blocks);

    size_t EmptyValue() const;
};

int& Deque::IntByIndex(size_t index) {
    size_t block_index;
    size_t index_inside;
    Convert(index, block_index, index_inside);

    return data_[block_index][index_inside];
}

int Deque::IntByIndex(size_t index) const {
    size_t block_index;
    size_t index_inside;
    Convert(index, block_index, index_inside);

    return data_[block_index][index_inside];
}

size_t Deque::BlockByIndex(size_t index) const {
    return index / block_size_;
}

void Deque::Assign(size_t size) {
    if (size == 0) {
        blocks_ = 0;
        left_ = 0;
        right_ = 0;
        data_.reset();
        return;
    }
    blocks_ = (size + block_size_ - 1) / block_size_;
    left_ = 0;
    right_ = size - 1;

    data_.reset(new std::unique_ptr<int[]>[blocks_]);
    for (size_t i = 0; i < blocks_; i++) {
        data_[i].reset(new int[block_size_]{0});
    }
}

void Deque::Reallocate(size_t new_blocks) {
    const size_t size_before = Size();
    std::unique_ptr<std::unique_ptr<int[]>[]> new_data(new std::unique_ptr<int[]>[new_blocks]);

    size_t i = 0;
    size_t j = BlockByIndex(left_);
    while (true) {
        new_data[i].swap(data_[j]);
        i++;
        if (j == BlockByIndex(right_)) {
            break;
        }
        j++;
        if (j == blocks_) {
            j = 0;
        }
    }

    while (i < new_blocks) {
        new_data[i].reset(new int[block_size_]{0});
        i++;
    }

    data_.swap(new_data);
    new_data.reset();

    left_ -= BlockByIndex(left_) * block_size_;
    right_ = left_ + size_before - 1;

    blocks_ = new_blocks;
}

size_t Deque::EmptyValue() const {
    return block_size_ * blocks_;
}

Deque::Deque() {
    Assign(0);
}

Deque::Deque(const Deque& rhs) {
    size_t size = rhs.Size();
    Assign(size);

    for (size_t i = 0; i < size; ++i) {
        IntByIndex(i) = rhs[i];
    }
}

Deque::Deque(Deque&& rhs) {
    Swap(rhs);
}

void Deque::Convert(size_t index, size_t& block_index, size_t& index_inside) const {
    block_index = BlockByIndex(index);
    index_inside = index % block_size_;
}

Deque::Deque(size_t size) {
    Assign(size);
}

Deque::Deque(std::initializer_list<int> list) {
    size_t size = list.size();
    Assign(size);

    auto it = list.begin();
    for (size_t i = 0; i < size; ++i) {
        IntByIndex(i) = *it;
        it++;
    }
}

Deque& Deque::operator=(Deque rhs) {
    Swap(rhs);
    return *this;
}

void Deque::Swap(Deque& rhs) {
    std::swap(left_, rhs.left_);
    std::swap(right_, rhs.right_);
    std::swap(blocks_, rhs.blocks_);
    data_.swap(rhs.data_);
}

void Deque::PushBack(int value) {
    if (blocks_ == 0) {
        Assign(1);
        left_ = 0;
        right_ = 0;
        IntByIndex(left_) = value;
        return;
    }

    if (left_ == EmptyValue()) {
        left_ = 0;
        right_ = 0;
        IntByIndex(left_) = value;
        return;
    }

    size_t new_index = right_ + 1;
    if (new_index == block_size_ * blocks_) {
        new_index = 0;
    }

    if (!(new_index <= left_ && BlockByIndex(left_) == BlockByIndex(new_index))) {
        right_ = new_index;
        IntByIndex(right_) = value;
        return;
    }

    Reallocate(blocks_ * 2);
    PushBack(value);
}

void Deque::PopBack() {
    if (left_ == right_) {
        left_ = EmptyValue();
        right_ = EmptyValue();
        return;
    }

    if (right_ == 0) {
        right_ = block_size_ * blocks_ - 1;
    } else {
        right_--;
    }
}

void Deque::PushFront(int value) {
    if (blocks_ == 0) {
        Assign(1);
        left_ = 0;
        right_ = 0;
        IntByIndex(left_) = value;
        return;
    }

    if (left_ == EmptyValue()) {
        left_ = 0;
        right_ = 0;
        IntByIndex(left_) = value;
        return;
    }

    size_t new_index;
    if (left_ == 0) {
        new_index = block_size_ * blocks_ - 1;
    } else {
        new_index = left_ - 1;
    }

    if (!(right_ <= new_index && BlockByIndex(new_index) == BlockByIndex(right_))) {
        left_ = new_index;
        IntByIndex(left_) = value;
        return;
    }

    Reallocate(blocks_ * 2);
    PushFront(value);
}

void Deque::PopFront() {
    if (left_ == right_) {
        left_ = EmptyValue();
        right_ = EmptyValue();
        return;
    }

    if (left_ == block_size_ * blocks_ - 1) {
        left_ = 0;
    } else {
        left_++;
    }
}

int& Deque::operator[](size_t ind) {
    size_t real_index = left_ + ind;
    if (real_index >= block_size_ * blocks_) {
        real_index -= block_size_ * blocks_;
    }
    return IntByIndex(real_index);
}

int Deque::operator[](size_t ind) const {
    size_t real_index = left_ + ind;
    if (real_index >= block_size_ * blocks_) {
        real_index -= block_size_ * blocks_;
    }
    return IntByIndex(real_index);
}

size_t Deque::Size() const {
    if (right_ == EmptyValue()) {
        return 0;
    }
    if (right_ >= left_) {
        return right_ - left_ + 1;
    }
    return block_size_ * blocks_ - (left_ - right_ - 1);
}

void Deque::Clear() {
    Assign(0);
}