#pragma once

#include <initializer_list>
#include <unordered_set>

// #include <Core/DataStructure/Array/Array.hpp>

#include <Renderer/Backend/Common.hpp>
#include <Renderer/Backend/RHI/Common/Globals.hpp>
#include <Renderer/Backend/Core/StaticString/StaticString.hpp>


TRE_NS_START

namespace Renderer
{
	class RENDERER_API RenderInstance
	{
	public:
		RenderInstance();

		~RenderInstance();

		// int32 CreateRenderInstance();

		int32 CreateRenderInstance(const char** extensions = NULL, uint32 extCount = 0, const char** layers = NULL, uint32 layerCount = 0);

		void DestroyRenderInstance();

        FORCEINLINE VkInstance GetApiObject() const { return internal.instance; }

	private:
		void FetchAvailbleInstanceExtensions();

		int32 CreateInstance(VkInstance* p_instance, const char** extensions = NULL, uint32 extCount = 0, const char** layers = NULL, uint32 layerCount = 0);

		static void DestroyInstance(VkInstance p_instance);

		// DEBUGGING SECTION // 

		static int32 SetupDebugMessenger(VkInstance p_instance, VkDebugUtilsMessengerEXT* p_debugMessenger);

		static int32 InitDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	private:
		Internal::RenderInstance internal;
		std::unordered_set<uint64> instanceExtensions;
		std::unordered_set<uint64> availbleInstExtensions;

		friend class RenderBackend;
	};
};

TRE_NS_END
