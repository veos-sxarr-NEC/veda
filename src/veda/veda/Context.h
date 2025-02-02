#pragma once

namespace veda {
	class Context {
	public:
		typedef std::tuple<VEDAdeviceptr, size_t> VPtrTuple;
	
	private:
		typedef std::map	<VEDAidx, VEDAdeviceptrInfo*>	Ptrs;
		typedef std::vector	<VEDAfunction>			Kernels;
		typedef std::vector	<Stream>			Streams;
		typedef std::map	<veo_lib, Module>		Modules;

			std::mutex		mutex_streams;
			std::mutex		mutex_ptrs;
			std::mutex		mutex_modules;

			VEDAcontext_mode	m_mode;
			Modules			m_modules;
			Ptrs			m_ptrs;
			Kernels			m_kernels;
			Streams			m_streams;
			Device&			m_device;
			veo_proc_handle*	m_handle;
			VEDAmodule		m_lib;
			VEDAidx			m_memidx;
			VEDAdeviceptr		m_memOverride;

		void			incMemIdx		(void);
		void			syncPtrs		(void);

	public:
					Context			(Device& device);
					Context			(const Context&) = delete;
		Device&			device			(void);
		Module*			moduleLoad		(const char* name);
		Stream&			stream			(const VEDAstream stream);
		VEDAcontext_mode	mode			(void) const;
		VEDAdeviceptr		memAlloc		(const size_t size, VEDAstream stream);
		VEDAdeviceptrInfo	getPtr			(VEDAdeviceptr vptr);
		VEDAfunction		kernel			(Kernel kernel) const;
		VEDAfunction		moduleGetFunction	(Module* mod, const char* name);
		VEDAresult		query			(VEDAstream stream);
		VPtrTuple		memAllocPitch		(const size_t w_bytes, const size_t h, const uint32_t elementSize, VEDAstream stream);
		bool			isActive		(void) const;
		int			streamCount		(void) const;
		size_t			memUsed			(void);
		veo_ptr			hmemId			(void) const;
		void			call			(VEDAfunction func, VEDAstream stream, VEDAargs args, const bool destroyArgs, const bool checkResult, uint64_t* result);
		void			call			(VEDAhost_function func, VEDAstream stream, void* userData, const bool checkResult, uint64_t* result);
		void			destroy			(void);
		void			init			(const VEDAcontext_mode mode);
		void			memFree			(VEDAdeviceptr vptr, VEDAstream stream);
		void			setMemOverride		(VEDAdeviceptr vptr);
		void			memReport		(void);
		void			memSwap			(VEDAdeviceptr A, VEDAdeviceptr B, VEDAstream stream);
		void			memcpyD2D		(VEDAdeviceptr dst, VEDAdeviceptr src, const size_t size, VEDAstream stream);
		void			memcpyD2H		(void* dst, VEDAdeviceptr src, const size_t size, VEDAstream stream);
		void			memcpyH2D		(VEDAdeviceptr dst, const void* src, const size_t size, VEDAstream stream);
		void			memset			(VEDAdeviceptr dst, const uint16_t value, const size_t size, VEDAstream stream);
		void			memset			(VEDAdeviceptr dst, const uint32_t value, const size_t size, VEDAstream stream);
		void			memset			(VEDAdeviceptr dst, const uint64_t value, const size_t size, VEDAstream stream);
		void			memset			(VEDAdeviceptr dst, const uint64_t x, const uint64_t y, const size_t size, VEDAstream stream);
		void			memset			(VEDAdeviceptr dst, const uint8_t value, const size_t size, VEDAstream stream);
		void			memset2D		(VEDAdeviceptr dst, const size_t pitch, const uint16_t value, const size_t w, const size_t h, VEDAstream stream);
		void			memset2D		(VEDAdeviceptr dst, const size_t pitch, const uint32_t value, const size_t w, const size_t h, VEDAstream stream);
		void			memset2D		(VEDAdeviceptr dst, const size_t pitch, const uint64_t value, const size_t w, const size_t h, VEDAstream stream);
		void			memset2D		(VEDAdeviceptr dst, const size_t pitch, const uint64_t x, const uint64_t y, const size_t w, const size_t h, VEDAstream stream);
		void			memset2D		(VEDAdeviceptr dst, const size_t pitch, const uint8_t value, const size_t w, const size_t h, VEDAstream stream);
		void			moduleUnload		(const Module* mod);
		void			sync			(VEDAstream stream);
		void			sync			(void);
	const	char*			kernelName		(VEDAfunction func) const;
	const	char*			kernelName		(const Kernel k) const;
	};
}
