#pragma once

#include <unordered_map>
#include <unordered_set>
#include <map>

#include <Core/DataStructure/Vector.hpp>
#include <Renderer/Backend/Common.hpp>

#include <Renderer/Backend/Core/StaticVector/StaticVector.hpp>
#include <Renderer/Backend/Core/Allocators/StackAllocator.hpp>
#include <Renderer/Backend/RHI/Common/Globals.hpp>
#include <Renderer/Backend/RHI/Buffers/Buffer.hpp>
#include <Renderer/Backend/RHI/CommandList/CommandPool.hpp>
#include <Renderer/Backend/RHI/RenderInstance/RenderInstance.hpp>
#include <Renderer/Backend/RHI/RenderDevice/RenderDevice.hpp>
#include <Renderer/Backend/RHI/RenderContext/RenderContext.hpp>
#include <Renderer/Backend/RHI/MemoryAllocator/MemoryAllocator.hpp>
#include <Renderer/Backend/RHI/Buffers/Buffer.hpp>
#include <Renderer/Backend/RHI/StagingManager/StagingManager.hpp>
#include <Renderer/Backend/RHI/CommandList/CommandPool.hpp>
#include <Renderer/Backend/RHI/CommandList/CommandList.hpp>
#include <Renderer/Backend/RHI/Descriptors/DescriptorSetAlloc.hpp>
#include <Renderer/Backend/RHI/ShaderProgram/ShaderProgram.hpp>
#include <Renderer/Backend/RHI/Images/Sampler.hpp>
#include <Renderer/Backend/RHI/RenderPass/Framebuffer.hpp>
#include <Renderer/Backend/RHI/RenderPass/FramebufferAllocator.hpp>
#include <Renderer/Backend/RHI/RenderPass/RenderPass.hpp>
#include <Renderer/Backend/RHI/Images/ImageHelper.hpp>
#include <Renderer/Backend/RHI/RenderPass/AttachmentAllocator.hpp>
#include <Renderer/Backend/RHI/Synchronization/Managers/FenceManager.hpp>
#include <Renderer/Backend/RHI/Synchronization/Fence/Fence.hpp>
#include <Renderer/Backend/RHI/Synchronization/Managers/SemaphoreManager.hpp>
#include <Renderer/Backend/RHI/Synchronization/Semaphore/Semaphore.hpp>
#include <Renderer/Backend/RHI/Synchronization/Managers/EventManager.hpp>
#include <Renderer/Backend/RHI/Synchronization/Event/Event.hpp>
#include <Renderer/Backend/RHI/Pipeline/Pipeline.hpp>
#include <Renderer/Backend/RHI/Pipeline/PipelineAllocator/PipelineAllocator.hpp>
#include <Renderer/Backend/RHI/RayTracing/BLAS/BLAS.hpp>
#include <Renderer/Backend/RHI/RayTracing/TLAS/TLAS.hpp>
#include <Renderer/Backend/RHI/RayTracing/ASBuilder.hpp>

TRE_NS_START

namespace Renderer
{
	class RENDERER_API RenderDevice
	{
    private:
        struct PerFrame
        {
            CommandPool commandPools[MAX_THREADS][(uint32)CommandBuffer::Type::MAX];

            struct Submission {
                StaticVector<CommandBufferHandle>  commands;
                StaticVector<SemaphoreHandle>      waitSemaphores;
                StaticVector<VkPipelineStageFlags> waitStages;
                StaticVector<uint64>               timelineSemaWait;

                StaticVector<SemaphoreHandle>      signalSemaphores;
                StaticVector<uint64>               timelineSemaSignal;

                FenceHandle*                       fence = NULL;

                void Clear()
                {
                    commands.Clear();
                    waitSemaphores.Clear();
                    waitStages.Clear();
                    timelineSemaWait.Clear();
                    signalSemaphores.Clear();
                    timelineSemaSignal.Clear();
                    fence = NULL;
                }

                ~Submission()
                {
                    // this->Clear();
                }
            };

            typedef StaticVector<Submission> Submissions;
            Submissions                   submissions[(uint32)CommandBuffer::Type::MAX];

            std::map<VkCommandPool, std::vector<VkCommandBuffer>> destroyedCmdBuffers;
            TRE::Vector<VkCommandPool>    destroyedCmdPools;

            TRE::Vector<VkPipeline>       destroyedPipelines;
            TRE::Vector<VkFramebuffer>    destroyedFramebuffers;
            TRE::Vector<VkImage>	      destroyedImages;
            TRE::Vector<VkImageView>      destroyedImageViews;
            TRE::Vector<VkBuffer>		  destroyedBuffers;
            TRE::Vector<VkBufferView>	  destroyedBufferViews;
            TRE::Vector<VkRenderPass>	  destroyedRenderPasses;
            TRE::Vector<VkDescriptorPool> destroyedDescriptorPool;
            TRE::Vector<VkSemaphore>	  destroyedSemaphores;
            TRE::Vector<VkEvent>		  destroyedEvents;
            TRE::Vector<VkSampler>        destroyedSamplers;

            TRE::Vector<VkFence>		  recycleFences;
            TRE::Vector<VkSemaphore>	  recycleSemaphores;
            // TRE::Vector<VkSemaphore>      recycledTimelineSema;

            TRE::Vector<VkDeviceMemory>	  freedMemory;
            TRE::Vector<MemoryAllocation> freeAllocatedMemory;

            TRE::Vector<VkAccelerationStructureKHR> destroyedAccls;

            bool shouldDestroy = false;
        };

        struct HandlePool
        {
            ObjectPool<CommandPool>   commandPools;
            ObjectPool<CommandBuffer> commandBuffers;
            ObjectPool<Buffer>		  buffers;
            ObjectPool<Image>		  images;
            ObjectPool<ImageView>	  imageViews;
            ObjectPool<Sampler>		  samplers;
            ObjectPool<Fence>		  fences;
            ObjectPool<Semaphore>	  semaphores;
            ObjectPool<PipelineEvent> events;

            // RT:
            ObjectPool<Blas>		  blases;
            ObjectPool<Tlas>		  tlases;
        };

	public:
        // Basic:
        RenderDevice(RenderContext* ctx);

        ~RenderDevice();

        void Shutdown();

		int32 CreateRenderDevice(const RenderInstance& renderInstance,
			const char** extensions = NULL, uint32 extCount = 0, const char** layers = NULL, uint32 layerCount = 0);

		void DestroryRenderDevice();

        int32 CreateLogicalDevice(const RenderInstance& renderInstance,
			const char** extensions = NULL, uint32 extCount = 0, const char** layers = NULL, uint32 layerCount = 0);

        void Init(uint32 enabledFeatures);

        void FlushStaging();


        // Frame managment:
        void BeginFrame();

        void EndFrame();

        void ClearFrame();

        // Shaders
        void CreateShaderProgram(const std::initializer_list<ShaderProgram::ShaderStage>& shaderStages, ShaderProgram* shaderProgramOut);


        // RT:
        BlasHandle CreateBlas(const BlasCreateInfo& blasInfo,
            VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

        TlasHandle CreateTlas(const TlasCreateInfo& createInfo,
            VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);

        VkAccelerationStructureKHR CreateAcceleration(VkAccelerationStructureCreateInfoKHR& info, BufferHandle* buffer);

        FORCEINLINE void RtSyncAcclBuilding() { acclBuilder.SyncAcclBuilding(); };

        FORCEINLINE void RtBuildTlasBatch() { acclBuilder.BuildTlasBatch(); };

        FORCEINLINE void RtBuildBlasBatchs() { acclBuilder.BuildBlasBatchs(); };

        FORCEINLINE void RtCompressBatch() { acclBuilder.CompressBatch(); };

        FORCEINLINE void BuildAS() { acclBuilder.BuildAll(); };


        // Command buffers and queues:
        CommandPoolHandle RequestCommandPool(uint32 queueFamily, CommandPool::Type type = CommandPool::Type::NONE);

        CommandBufferHandle RequestCommandBuffer(CommandBuffer::Type type = CommandBuffer::Type::GENERIC);

        PerFrame::Submission& CreateNewSubmission(CommandBuffer::Type type);

        PerFrame::Submission& GetLatestSubmission(CommandBuffer::Type type);

        void Submit(PerFrame::Submission& sub, CommandBuffer::Type type, CommandBufferHandle cmd, FenceHandle* fence = NULL,
                    uint32 semaphoreCount = 0, SemaphoreHandle** semaphores = NULL, uint32 signalValuesCount = 0,
                    const uint64* signalValues = NULL);

        void Submit(CommandBuffer::Type type, CommandBufferHandle cmd, FenceHandle* fence = NULL, uint32 semaphoreCount = 0,
                    SemaphoreHandle** semaphores = NULL, uint32 signalValuesCount = 0, const uint64* signalValues = NULL);

        void Submit(CommandBufferHandle cmd, FenceHandle* fence = NULL, uint32 semaphoreCount = 0, SemaphoreHandle** semaphores = NULL,
                    uint32 signalValuesCount = 0, const uint64* signalValues = NULL);

        void AddWaitSemapore(CommandBuffer::Type type, SemaphoreHandle semaphore, VkPipelineStageFlags stages, bool flush = false);

        void AddWaitSemapore(PerFrame::Submission& sub, SemaphoreHandle semaphore, VkPipelineStageFlags stages);

        void AddWaitTimelineSemapore(CommandBuffer::Type type, SemaphoreHandle semaphore, VkPipelineStageFlags stages,
                                     uint64 waitValue = 0, bool flush = false);

        void AddWaitTimelineSemapore(PerFrame::Submission& sub, SemaphoreHandle semaphore, VkPipelineStageFlags stages, uint64 waitValue = 0);

        void AddSignalSemaphore(CommandBuffer::Type type, uint32 semaphoreCount = 0, SemaphoreHandle** semaphores = NULL,
            uint32 signalValuesCount = 0, const uint64* signalValues = NULL);

        void AddSignalSemaphore(PerFrame::Submission& sub, uint32 semaphoreCount = 0, SemaphoreHandle** semaphores = NULL, 
                                uint32 signalValuesCount = 0, const uint64* signalValues = NULL);

        void SetFence(CommandBuffer::Type type, FenceHandle* fence);

        void SetFence(PerFrame::Submission& sub, FenceHandle* fence);

        void FlushQueue(CommandBuffer::Type type, bool triggerSwapchainSwap = false);

        void FlushQueues();

        // Buffer Creation:
        BufferHandle CreateBuffer(const BufferInfo& createInfo, const void* data = NULL);

        BufferHandle CreateRingBuffer(const BufferInfo& createInfo, const void* data = NULL, const uint32 ringSize = NUM_FRAMES);

        bool CreateBufferInternal(VkBuffer& outBuffer, MemoryAllocation& outMemoryView, const BufferInfo& createInfo);

        // Image Creation:
        ImageHandle CreateImage(const ImageCreateInfo& createInfo, const void* data = NULL);

        ImageViewHandle CreateImageView(const ImageViewCreateInfo& createInfo);

        // Memory allocation:
        MemoryAllocation AllocateMemory(VkBuffer buffer, uint32 usage, MemoryDomain domain, uint32 multiplier = 1);

        MemoryAllocation AllocateMemory(VkImage image, uint32 usage, MemoryDomain domain, uint32 multiplier = 1);


        // Sampler creation:
        SamplerHandle CreateSampler(const SamplerInfo& createInfo);


        // Sync primitive creation:
        FenceHandle RequestFence();

        void ResetFence(VkFence fence, bool isWaited);

        SemaphoreHandle RequestSemaphore();

        SemaphoreHandle RequestTimelineSemaphore(uint64 value = 1);

        void ResetTimelineSemaphore(Semaphore& semaphore);

        PiplineEventHandle RequestPiplineEvent();


        // Pipeline:
        Pipeline& RequestPipeline(ShaderProgram& program, const RenderPass& rp, const GraphicsState& state);

        Pipeline& RequestPipeline(ShaderProgram& program);


        // Render pass and framebuffer functionalities:
        const Framebuffer& RequestFramebuffer(const RenderPassInfo& info, const RenderPass* rp = NULL);

        const RenderPass& RequestRenderPass(const RenderPassInfo& info, bool compatible = false);

        RenderPassInfo GetSwapchainRenderPass(SwapchainRenderPass style);

        ImageView& GetTransientAttachment(uint32 width, uint32 height, VkFormat format, uint32 index = 0, uint32 samples = 1, uint32 layers = 1);


        // Descriptor sets allocators:
        DescriptorSetAllocator* RequestDescriptorSetAllocator(const DescriptorSetLayout& layout);


        // Swapchain related:
        SemaphoreHandle GetImageAcquiredSemaphore();

        SemaphoreHandle GetDrawCompletedSemaphore();


        // Destroy functions:
        void DestroyPendingObjects(PerFrame& frame);

        void DestroyImage(VkImage image);

        void DestroyImageView(VkImageView view);

        void DestroyFramebuffer(VkFramebuffer fb);

        void FreeMemory(VkDeviceMemory memory);

        void FreeMemory(const MemoryAllocation& alloc);

        void DestroySemaphore(VkSemaphore sem);

        void DestroryEvent(VkEvent event);

        void DestroyBuffer(VkBuffer buffer);

        void DestroyBufferView(VkBufferView view);

        void DestroySampler(VkSampler sampler);

        void FreeCommandBuffer(VkCommandPool pool, VkCommandBuffer cmd);

        void DestroyCommandPool(VkCommandPool pool);

        void RecycleSemaphore(VkSemaphore sem);

        void DestroyAllFrames();


        // Functions that already existed here:
        VkSampleCountFlagBits GetUsableSampleCount(uint32 sampleCount = 4) const;

        VkSampleCountFlagBits GetMaxUsableSampleCount() const;

        uint32 FindMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) const;

        uint32 FindMemoryTypeIndex(uint32 typeFilter, MemoryDomain usage) const;

        std::pair<uint32, uint32> FindMemoryTypeFlag(MemoryDomain usage) const;


        // Memory:
		VkDeviceMemory AllocateDedicatedMemory(VkImage image, MemoryDomain memoryDomain = MemoryDomain::GPU_ONLY) const;

		VkDeviceMemory AllocateDedicatedMemory(VkBuffer buffer, MemoryDomain memoryDomain = MemoryDomain::GPU_ONLY) const;

		void FreeDedicatedMemory(VkDeviceMemory memory) const;

        // Buffer
        VkBuffer CreateBufferHelper(const BufferInfo& info) const;

		VkDeviceMemory CreateBufferMemory(const BufferInfo& info, VkBuffer buffer,
			VkDeviceSize* alignedSize = NULL, uint32 multiplier = 1) const;

        VkDeviceAddress GetBufferAddress(BufferHandle buff) const;

        // Accl
		VkAccelerationStructureKHR CreateAcceleration(VkAccelerationStructureCreateInfoKHR& info, VkBuffer* buffer) const;

        // Commands
		VkCommandBuffer CreateCmdBuffer(VkCommandPool pool,
			VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			VkCommandBufferUsageFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;

        VkResult SubmitCmdBuffer(VkQueue queue, VkCommandBuffer* cmdBuff, uint32 cmdCount, VkPipelineStageFlags waitStage,
			VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence = VK_NULL_HANDLE) const;


        // Getters
		FORCEINLINE VkDevice GetDevice() const { return internal.device; }

		FORCEINLINE VkPhysicalDevice GetGPU() const { return internal.gpu; }

		FORCEINLINE const Internal::QueueFamilyIndices& GetQueueFamilyIndices() const { return internal.queueFamilyIndices; }

		FORCEINLINE const VkQueue* const GetQueues() const { return internal.queues; }

		FORCEINLINE bool IsPresentQueueSeprate() const { return internal.isPresentQueueSeprate; }

		FORCEINLINE bool IsTransferQueueSeprate() const { return internal.isTransferQueueSeprate; }

        FORCEINLINE const VkPhysicalDeviceProperties& GetProperties() const { return internal.gpuProperties2.properties; }

		FORCEINLINE const VkPhysicalDeviceMemoryProperties& GetMemoryProperties() const { return internal.memoryProperties; }

		FORCEINLINE const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetRtProperties() const { return internal.rtProperties; }

		FORCEINLINE const VkPhysicalDeviceAccelerationStructureFeaturesKHR& GetAcclFeatures() const { return internal.accelFeatures; }

        FORCEINLINE const RenderContext* GetRenderContext() const { return renderContext; }

        // Useful Getters:
        FORCEINLINE MemoryAllocator2& GetContextAllocator() { return gpuMemoryAllocator; }

        FORCEINLINE StagingManager& GetStagingManager() { return stagingManager; }

        FORCEINLINE ObjectPool<CommandBuffer>& GetCommandBufferPool() { return objectsPool.commandBuffers; }

        FORCEINLINE HandlePool& GetObjectsPool() { return objectsPool; }

        FORCEINLINE bool IsMemoryInDomain(uint32 typeIndex, MemoryDomain usage) const
        {
            return internal.memoryTypeFlags[typeIndex] & (1 << (uint32)usage);
        }

        FORCEINLINE bool IsMemoryInDomains(uint32 typeIndex, uint32 domainsFlags) const
        {
            return internal.memoryTypeFlags[typeIndex] & domainsFlags;
        }

        VkQueue GetQueue(CommandBuffer::Type type);

        FORCEINLINE VkQueue GetQueue(uint32 i) const { return internal.queues[i]; }
	private:
		void FetchDeviceAvailableExtensions();

		typedef bool(*FPN_RankGPU)(VkPhysicalDevice, VkSurfaceKHR);

		static bool IsDeviceSuitable(VkPhysicalDevice gpu, VkSurfaceKHR surface);

        static VkPhysicalDevice PickGPU(const RenderInstance& renderInstance,
                                        const RenderContext& ctx, FPN_RankGPU p_pick_func = IsDeviceSuitable);

		static Internal::QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice p_gpu, VkSurfaceKHR p_surface = NULL); 

        FORCEINLINE const PerFrame& Frame() const { return perFrame[renderContext->GetCurrentFrame()]; }

        FORCEINLINE PerFrame& Frame() { return perFrame[renderContext->GetCurrentFrame()]; }

        FORCEINLINE const PerFrame& PreviousFrame() const { return perFrame[renderContext->GetPreviousFrame()]; }

        FORCEINLINE PerFrame& PreviousFrame() { return perFrame[renderContext->GetPreviousFrame()]; }

        CommandBuffer::Type GetPhysicalQueueType(CommandBuffer::Type type);

        PerFrame::Submissions& GetQueueSubmissions(CommandBuffer::Type type);
	private:
		Internal::RenderDevice internal;
        RenderContext* renderContext;

        MemoryAllocator2 gpuMemoryAllocator;
        StagingManager	 stagingManager;
        FenceManager	 fenceManager;
        SemaphoreManager semaphoreManager;
        EventManager	 eventManager;

        // RT:
        AsBuilder        acclBuilder;

        std::unordered_map<Hash, DescriptorSetAllocator> descriptorSetAllocators;
        std::unordered_map<Hash, RenderPass>			 renderPasses;
        FramebufferAllocator							 framebufferAllocator;
        AttachmentAllocator								 transientAttachmentAllocator;
        PipelineAllocator								 pipelineAllocator;

        PerFrame		perFrame[MAX_FRAMES];
        HandlePool		objectsPool;

        uint32 enabledFeatures;
        bool submitSwapchain;
        bool stagingFlush;

		std::unordered_set<uint64> deviceExtensions;
		std::unordered_set<uint64> availbleDevExtensions;

		friend class RenderBackend;
        friend class Image;
        friend class Swapchain;
        friend class Semaphore;
        friend class Fence;
        friend class Swapchain;
	};
};

TRE_NS_END
