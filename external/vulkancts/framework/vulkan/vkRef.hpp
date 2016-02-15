#ifndef _VKREF_HPP
#define _VKREF_HPP
/*-------------------------------------------------------------------------
 * Vulkan CTS Framework
 * --------------------
 *
 * Copyright (c) 2015 Google Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice(s) and this permission notice shall be
 * included in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *//*!
 * \file
 * \brief Vulkan object reference holder.
 *//*--------------------------------------------------------------------*/

#include "vkDefs.hpp"
#include "vkStrUtil.hpp"
#include "deMeta.hpp"

#include <algorithm>

namespace vk
{

namespace refdetails
{

using std::swap;

template<typename T>
struct Checked
{
	explicit inline		Checked		(T object_) : object(object_) {}

	T					object;
};

//! Check that object is not null
template<typename T>
inline Checked<T> check (T object)
{
	if (!object)
		throw tcu::TestError("Object check() failed", (std::string(getTypeName<T>()) + " = 0").c_str(), __FILE__, __LINE__);
	return Checked<T>(object);
}

//! Declare object as checked earlier
template<typename T>
inline Checked<T> notNull (T object)
{
	if (!object)
		throw tcu::InternalError("Null object was given to notNull()", (std::string(getTypeName<T>()) + " = 0").c_str(), __FILE__, __LINE__);
	return Checked<T>(object);
}

//! Allow null object
template<typename T>
inline Checked<T> allowNull (T object)
{
	return Checked<T>(object);
}

template<typename T>
class Deleter
{
public:
									Deleter		(const DeviceInterface& deviceIface, VkDevice device, const VkAllocationCallbacks* allocator)
										: m_deviceIface	(&deviceIface)
										, m_device		(device)
										, m_allocator	(allocator)
									{}
									Deleter		(void)
										: m_deviceIface	(DE_NULL)
										, m_device		(DE_NULL)
										, m_allocator	(DE_NULL)
									{}

	void							operator()	(T obj) const;

private:
	const DeviceInterface*			m_deviceIface;
	VkDevice						m_device;
	const VkAllocationCallbacks*	m_allocator;
};

template<>
class Deleter<VkInstance>
{
public:
									Deleter		(const PlatformInterface& platformIface, VkInstance instance, const VkAllocationCallbacks* allocator)
										: m_destroyInstance	((DestroyInstanceFunc)platformIface.getInstanceProcAddr(instance, "vkDestroyInstance"))
										, m_allocator		(allocator)
									{}
									Deleter		(void)
										: m_destroyInstance	((DestroyInstanceFunc)DE_NULL)
										, m_allocator		(DE_NULL)
									{}

	void							operator()	(VkInstance obj) const { m_destroyInstance(obj, m_allocator); }

private:
	DestroyInstanceFunc				m_destroyInstance;
	const VkAllocationCallbacks*	m_allocator;
};

template<>
class Deleter<VkDevice>
{
public:
									Deleter		(const InstanceInterface& instanceIface, VkDevice device, const VkAllocationCallbacks* allocator)
										: m_destroyDevice	((DestroyDeviceFunc)instanceIface.getDeviceProcAddr(device, "vkDestroyDevice"))
										, m_allocator		(allocator)
									{}
									Deleter		(void)
										: m_destroyDevice	((DestroyDeviceFunc)DE_NULL)
										, m_allocator		(DE_NULL)
									{}

	void							operator()	(VkDevice obj) const { m_destroyDevice(obj, m_allocator); }

private:
	DestroyDeviceFunc				m_destroyDevice;
	const VkAllocationCallbacks*	m_allocator;
};

template<>
class Deleter<VkDescriptorSet>
{
public:
							Deleter		(const DeviceInterface& deviceIface, VkDevice device, VkDescriptorPool pool)
								: m_deviceIface	(&deviceIface)
								, m_device		(device)
								, m_pool		(pool)
							{}
							Deleter		(void)
								: m_deviceIface	(DE_NULL)
								, m_device		(DE_NULL)
								, m_pool		(DE_NULL)
							{}

	void					operator()	(VkDescriptorSet obj) const { m_deviceIface->freeDescriptorSets(m_device, m_pool, 1, &obj); }

private:
	const DeviceInterface*	m_deviceIface;
	VkDevice				m_device;
	VkDescriptorPool		m_pool;
};

template<>
class Deleter<VkCommandBuffer>
{
public:
							Deleter		(const DeviceInterface& deviceIface, VkDevice device, VkCommandPool pool)
								: m_deviceIface	(&deviceIface)
								, m_device		(device)
								, m_pool		(pool)
							{}
							Deleter		(void)
								: m_deviceIface	(DE_NULL)
								, m_device		(DE_NULL)
								, m_pool		(DE_NULL)
							{}

	void					operator()	(VkCommandBuffer obj) const { m_deviceIface->freeCommandBuffers(m_device, m_pool, 1, &obj); }

private:
	const DeviceInterface*	m_deviceIface;
	VkDevice				m_device;
	VkCommandPool			m_pool;
};

template<typename T>
struct RefData
{
				RefData		(T object_, Deleter<T> deleter_)
								: object	(object_)
								, deleter	(deleter_)
				{}
				RefData		(void)
								: object	(0)
				{}

	T			object;
	Deleter<T>	deleter;
};

template<typename T>
class RefBase
{
public:
						~RefBase	(void);

	inline const T&		get			(void) const throw() { return m_data.object;	}
	inline const T&		operator*	(void) const throw() { return get();			}
	inline operator		bool		(void) const throw() { return !!get();			}

protected:
						RefBase		(RefData<T> data) : m_data(data)	{}

	void				reset		(void);				//!< Release previous object, set to null.
	RefData<T>			disown		(void) throw();		//!< Disown and return object (ownership transferred to caller).
	void				assign		(RefData<T> data);	//!< Set new pointer, release previous pointer.

private:
	RefData<T>			m_data;
};

template<typename T>
inline RefBase<T>::~RefBase (void)
{
	this->reset();
}

template<typename T>
inline void RefBase<T>::reset (void)
{
	if (!!m_data.object)
		m_data.deleter(m_data.object);

	m_data = RefData<T>();
}

template<typename T>
inline RefData<T> RefBase<T>::disown (void) throw()
{
	RefData<T> tmp;
	swap(m_data, tmp);
	return tmp;
}

template<typename T>
inline void RefBase<T>::assign (RefData<T> data)
{
	this->reset();
	m_data = data;
}

/*--------------------------------------------------------------------*//*!
 * \brief Movable Vulkan object reference.
 *
 * Similar to de::MovePtr.
 *//*--------------------------------------------------------------------*/
template<typename T>
class Move : public RefBase<T>
{
public:
	template<typename U>
				Move		(Checked<U> object, Deleter<U> deleter)
								: RefBase<T>(RefData<T>(object.object, deleter))
				{}

				Move		(RefData<T> data)
								: RefBase<T>(data)
				{}
				Move		(Move<T>& other)
								: RefBase<T>(other.RefBase<T>::disown())
				{}
				Move		(void)
								: RefBase<T>(RefData<T>())
				{}

	T			disown		(void) { return this->RefBase<T>::disown().object; }
	Move<T>&	operator=	(Move<T>& other);
	Move<T>&	operator=	(RefData<T> data);

	operator	RefData<T>	(void) { return this->RefBase<T>::disown(); }
};

template<typename T>
inline Move<T>& Move<T>::operator= (Move<T>& other)
{
	if (this != &other)
		this->assign(other.RefBase<T>::disown());

	return *this;
}

template<typename T>
inline Move<T>& Move<T>::operator= (RefData<T> data)
{
	this->assign(data);
	return *this;
}

/*--------------------------------------------------------------------*//*!
 * \brief Unique Vulkan object reference.
 *
 * Similar to de::UniquePtr.
 *//*--------------------------------------------------------------------*/
template<typename T>
class Unique : public RefBase<T>
{
public:
	template<typename U>
				Unique		(Checked<U> object, Deleter<U> deleter)
								: RefBase<T>(RefData<T>(object.object, deleter))
				{}

				Unique		(RefData<T> data)
								: RefBase<T>(data)
				{}

private:
				Unique		(const Unique<T>&);
	Unique<T>&	operator=	(const Unique<T>&);
};

} // refdetails

using refdetails::Move;
using refdetails::Unique;
using refdetails::Deleter;
using refdetails::check;
using refdetails::notNull;
using refdetails::allowNull;

} // vk

#endif // _VKREF_HPP
