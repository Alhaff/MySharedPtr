#pragma once
#include <memory>
template <typename T>
class weak_ptr;

template <typename T>
class shared_ptr;

class ControlBlock
{
	template <typename T>
	friend class shared_ptr;

	template <typename T>
	friend  class weak_ptr;
private:
	size_t ref_count;
	size_t weak_count;
	ControlBlock(size_t ref = 1, size_t weak = 0)
		: ref_count(ref),
		weak_count(weak)
	{}
};

template <typename T>
class shared_ptr
{
	template <typename T1>
	friend  class weak_ptr;
private:
	T* pointer_;
	ControlBlock* control_block_;
public:

	shared_ptr(T* pointer = nullptr)
		: pointer_(pointer),
		control_block_(new ControlBlock()) {}

	shared_ptr(const shared_ptr& other) noexcept
		: pointer_(other.pointer_),
		control_block_(other.control_block_)
	{
		control_block_->ref_count++;
	}

	shared_ptr(const weak_ptr<T>& other)
		: pointer_(other.pointer_),
		control_block_(other.control_block_)
	{
		++control_block_->ref_count;
	}

	shared_ptr& operator=(const shared_ptr& other) noexcept
	{
		if (&other != this)
		{
			pointer_ = other.pointer_;
			control_block_ = other.control_block_;
			control_block_->ref_count++;
		}
		return *this;
	}

	shared_ptr(shared_ptr&& other) noexcept
		: pointer_(std::move(other.pointer_)),
		control_block_(std::move(other.control_block_))
	{
		other.pointer_ = nullptr;
		other.control_block_ = nullptr;
	}

	shared_ptr& operator=(shared_ptr&& other) noexcept
	{

		if (&other != this)
		{
			pointer_ = std::move(other.pointer_);
			control_block_ = std::move(other.control_block_);
			other.pointer_ = nullptr;
			other.control_block_ = nullptr;
		}
		return *this;
	}

	T* get() const
	{
		return pointer_;
	}

	T& operator*() const
	{
		return *pointer_;
	}

	T* operator->() const
	{
		return pointer_;
	}

	operator bool()
	{
		return (pointer_ != nullptr);
	}

	size_t use_count()
	{
		return (pointer_ != nullptr) ? control_block_->ref_count : 0;
	}

	void swap(shared_ptr& other)
	{
		std::swap(pointer_, other.pointer_);
		std::swap(control_block_, other.control_block_);
	}

	void reset(T* new_pointer = nullptr)
	{
		this->~shared_ptr();
		pointer_ = new_pointer;
		control_block_ = new ControlBlock();
	}

	~shared_ptr()
	{
		if (control_block_ != nullptr)
		{

			if (--control_block_->ref_count == 0)
			{
				delete pointer_;
				if (control_block_->weak_count == 0)
				{
					delete control_block_;
				}
			}
		}
	}
};

template <typename T>
class weak_ptr
{

	template <typename T1>
	friend class shared_ptr;
private:
	T* pointer_;
	ControlBlock* control_block_;
public:
	weak_ptr(T* pointer = nullptr)
		: pointer_(pointer),
		control_block_(new ControlBlock(0, 1))
	{
	}

	weak_ptr(const weak_ptr& other)
		: pointer_(other.pointer_),
		control_block_(other.control_block_)
	{
		control_block_->weak_count++;
	}

	weak_ptr(const shared_ptr<T>& other)
		: pointer_(other.pointer_),
		control_block_(other.control_block_)
	{
		control_block_->weak_count++;
	}

	weak_ptr(weak_ptr&& other)
		: pointer_(std::move(other.pointer_)),
		control_block_(std::move(other.control_block_))
	{
		other.pointer_ = nullptr;
		other.control_block_ = nullptr;
	}

	weak_ptr& operator=(const weak_ptr& other)
	{

		if (&other != this)
		{
			pointer_ = other.pointer_;
			control_block_ = other.control_block_;
			control_block_->weak_count++;
		}
		return *this;
	}

	weak_ptr& operator=(weak_ptr&& other)
	{

		if (&other != this)
		{
			pointer_ = std::move(other.pointer_);
			control_block_ = std::move(other.control_block_);
			other.pointer_ = nullptr;
			other.control_block_ = nullptr;
		}
		return *this;
	}

	const size_t& use_count()
	{
		return control_block_->ref_count;
	}

	void reset()
	{
		control_block_->weak_count--;
		pointer_ = nullptr;
		control_block_ = new ControlBlock(0,0);
	}

	bool expired()
	{
		return  (control_block_->ref_count == 0);
	}

	void swap(weak_ptr& other)
	{
		std::swap(pointer_, other.pointer_);
		std::swap(control_block_, other.control_block_);
	}

	shared_ptr<T> lock()
	{
		return (!expired()) ? shared_ptr<T>(*this) : shared_ptr<T>();
	}

	~weak_ptr()
	{
		if (control_block_ != nullptr)
		{
			if (--control_block_->weak_count == 0 && control_block_->ref_count == 0)
			{
				delete control_block_;
			}
		}

	}
};