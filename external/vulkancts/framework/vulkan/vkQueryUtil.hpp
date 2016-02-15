#ifndef _VKQUERYUTIL_HPP
#define _VKQUERYUTIL_HPP
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
 * \brief Vulkan query utilities.
 *//*--------------------------------------------------------------------*/

#include "vkDefs.hpp"
#include "deMemory.h"

#include <vector>

namespace vk
{

std::vector<VkPhysicalDevice>			enumeratePhysicalDevices				(const InstanceInterface& vk, VkInstance instance);
std::vector<VkQueueFamilyProperties>	getPhysicalDeviceQueueFamilyProperties	(const InstanceInterface& vk, VkPhysicalDevice physicalDevice);
VkPhysicalDeviceFeatures				getPhysicalDeviceFeatures				(const InstanceInterface& vk, VkPhysicalDevice physicalDevice);
VkPhysicalDeviceProperties				getPhysicalDeviceProperties				(const InstanceInterface& vk, VkPhysicalDevice physicalDevice);
VkPhysicalDeviceMemoryProperties		getPhysicalDeviceMemoryProperties		(const InstanceInterface& vk, VkPhysicalDevice physicalDevice);
VkFormatProperties						getPhysicalDeviceFormatProperties		(const InstanceInterface& vk, VkPhysicalDevice physicalDevice, VkFormat format);
VkImageFormatProperties					getPhysicalDeviceImageFormatProperties	(const InstanceInterface& vk, VkPhysicalDevice physicalDevice, VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags);

VkMemoryRequirements					getBufferMemoryRequirements				(const DeviceInterface& vk, VkDevice device, VkBuffer buffer);
VkMemoryRequirements					getImageMemoryRequirements				(const DeviceInterface& vk, VkDevice device, VkImage image);

std::vector<VkLayerProperties>			enumerateInstanceLayerProperties		(const PlatformInterface& vkp);
std::vector<VkExtensionProperties>		enumerateInstanceExtensionProperties	(const PlatformInterface& vkp, const char* layerName);
std::vector<VkLayerProperties>			enumerateDeviceLayerProperties			(const InstanceInterface& vki, VkPhysicalDevice physicalDevice);
std::vector<VkExtensionProperties>		enumerateDeviceExtensionProperties		(const InstanceInterface& vki, VkPhysicalDevice physicalDevice, const char* layerName);

bool									isShaderStageSupported					(const VkPhysicalDeviceFeatures& deviceFeatures, VkShaderStageFlagBits stage);

template <typename Context, typename Interface, typename Type>
bool validateInitComplete(Context context, void (Interface::*Function)(Context, Type*)const, const Interface& interface)
{
	Type vec[2];
	deMemset(&vec[0], 0x00, sizeof(Type));
	deMemset(&vec[1], 0xFF, sizeof(Type));

	(interface.*Function)(context, &vec[0]);
	(interface.*Function)(context, &vec[1]);

	for (size_t ndx = 0; ndx < sizeof(Type); ndx++)
	{
		if (reinterpret_cast<deUint8*>(&vec[0])[ndx] != reinterpret_cast<deUint8*>(&vec[1])[ndx])
			return false;
	}

	return true;
}

} // vk

#endif // _VKQUERYUTIL_HPP
