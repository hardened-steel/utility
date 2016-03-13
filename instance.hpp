// Project: utility
//
//  Created on: 13 марта 2016 г.
//      Author: motorhead
//

#ifndef INSTANCE_HPP_
#define INSTANCE_HPP_

#include <typeinfo>
#include <iostream>
#include <type_traits>

namespace utility {

	namespace details {
		class instance_control_block
		{
			size_t counter = 1;
		public:
			virtual instance_control_block* clone() const = 0;
			virtual uintptr_t offset(uintptr_t ptr) const = 0;
			virtual uintptr_t getptr() const = 0;
			size_t inc() { return counter += 1; }
			size_t dec() { return counter -= 1; }
			bool unique() const { return counter == 1; }
			virtual ~instance_control_block() {}
		};
		template<class T> class unique_control_block: public instance_control_block
		{
			T object;
		public:
			template<class...TArgs> unique_control_block(TArgs&&...args): object(std::forward<TArgs>(args)...) { std::cout << "create::unique::" << typeid(T).name() << std::endl; }
			uintptr_t offset(uintptr_t ptr) const override { return ptr - getptr(); }
			uintptr_t getptr() const override { return (uintptr_t)&object; }
			instance_control_block* clone() const override { return nullptr; }
			~unique_control_block() { std::cout << "destroy::unique::" << typeid(T).name() << std::endl; }
		};
		template<class T> class shared_control_block: public details::instance_control_block
		{
			T object;
		public:
			template<class...TArgs> shared_control_block(TArgs&&...args): object(std::forward<TArgs>(args)...) { std::cout << "create::shared::" << typeid(T).name() << std::endl; }
			uintptr_t offset(uintptr_t ptr) const override { return ptr - getptr(); }
			uintptr_t getptr() const override { return (uintptr_t)&object; }
			instance_control_block* clone() const override { return new shared_control_block(object); }
			~shared_control_block() { std::cout << "destroy::shared::" << typeid(T).name() << std::endl; }
		};
	}

	template<class T, bool Copyable = std::is_copy_constructible<T>::value> class instance;

	template<class T> class instance<T, false>
	{
		template<class U, bool C> friend class instance;
	protected:
		mutable details::instance_control_block* cblock;
		mutable T* object;
	protected:
		void reset() const {
			if(nullptr != cblock) delete cblock;
		}
	public:
		template<class...TArgs>
		instance(TArgs&&...args): cblock(new details::unique_control_block<T>(std::forward<TArgs>(args)...)), object((T*)cblock->getptr()) {}
		instance(const instance& other) = delete;
		instance(instance& other) = delete;
		instance& operator=(const instance& other) = delete;
		instance& operator=(instance& other) = delete;

		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, false>& other)
		: cblock(new details::unique_control_block<T>(other.get())), object((T*)cblock->getptr()) {}
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, false>& other)
		: cblock(new details::unique_control_block<T>(other.get())), object((T*)cblock->getptr()) {}
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, false>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, false>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }

		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, true>& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { cblock->inc(); }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, true>& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { cblock->inc(); }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, true>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, true>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }

		~instance() { reset(); }
	};

	template<class T> class instance<T, true>
	{
		template<class U, bool C> friend class instance;
	protected:
		mutable details::instance_control_block* cblock;
		mutable T* object;
	protected:
		void reset() const {
			if(nullptr != cblock) if(cblock->dec() == 0) delete cblock;
		}
	public:
		template<class...TArgs>
		instance(TArgs&&...args): cblock(new details::shared_control_block<T>(std::forward<TArgs>(args)...)), object((T*)cblock->getptr()) {}

		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, false>& other)
		: cblock(new details::shared_control_block<T>(other.get())), object((T*)cblock->getptr()) {}
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, false>& other)
		: cblock(new details::shared_control_block<T>(other.get())), object((T*)cblock->getptr()) {}
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, false>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, false>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }

		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, true>& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { cblock->inc(); }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, true>& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { cblock->inc(); }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(const instance<D, true>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }
		template<class D, class = typename std::enable_if<std::is_base_of<T, D>::value>::type> instance(instance<D, true>&& other)
		: cblock(other.cblock), object((T*)cblock->getptr()) { other.cblock = nullptr; }

		~instance() { reset(); }

		T& get() {
			if(!cblock->unique()) {
				auto new_cblock = cblock->clone();
				auto offset = cblock->offset((uintptr_t)object);
				reset();
				cblock = new_cblock;
				object = (T*)(cblock->getptr() + offset);
			}
			return *object;
		}
		const T& get() const { return *object; }
		operator T&() { return get(); }
		operator const T&() const { return get(); }
	};

} // namespace utility

#endif /* INSTANCE_HPP_ */
