#include <veda/internal.h>
#include <fcntl.h>
namespace veda {
//------------------------------------------------------------------------------
static bool		s_initialized	= false;
static bool		s_memTrace	= false;
static int		s_ompThreads	= 0;
static std::string	s_stdLib;
static int		s_envOmpThread  = 0;

//------------------------------------------------------------------------------
bool		isMemTrace	(void) {	return s_memTrace;						}
const char*	stdLib		(void) {	return s_stdLib.c_str();					}
int		ompThreads	(void) {	return s_ompThreads;						}
void		checkInitialized(void) {	if(!s_initialized) VEDA_THROW(VEDA_ERROR_NOT_INITIALIZED);	}

//------------------------------------------------------------------------------
void setInitialized(const bool value) {
	if(value && s_initialized)
		VEDA_THROW(VEDA_ERROR_ALREADY_INITIALIZED);
	else if(!value && !s_initialized)
		VEDA_THROW(VEDA_ERROR_NOT_INITIALIZED);

	if(value) {
		// Init MemTrace -----------------------------------------------
		auto memTrace = std::getenv("VEDA_MEM_TRACE");
		s_memTrace = memTrace && std::atoi(memTrace);

		// Init OMP Threads --------------------------------------------
		auto env = std::getenv("VE_OMP_NUM_THREADS");
		if(env)
			s_ompThreads = std::atoi(env);
                        s_envOmpThread = s_ompThreads;
#if BUILD_VEOS_RELEASE
                const char* arch = veda::ve_arch_find();
                if(!strncmp(arch, "ve3", 3)) {
                   if(!std::getenv("VEORUN_BIN")) {
                     if(std::getenv("VEDA_FTRACE"))        setenv("VEORUN_BIN", "/opt/nec/ve/veos/libexec/aveorun-ftrace_ve3", 1);
                     else                                  setenv("VEORUN_BIN", "/opt/nec/ve/veos/libexec/aveorun_ve3", 1);
                   }
                   s_stdLib = "/opt/nec/ve3/lib/libveda.vso";
                }
                else {
                   if(!std::getenv("VEORUN_BIN")) {
                     if(std::getenv("VEDA_FTRACE"))        setenv("VEORUN_BIN", "/opt/nec/ve/veos/libexec/aveorun-ftrace_ve1", 1);
                     else                                  setenv("VEORUN_BIN", "/opt/nec/ve/veos/libexec/aveorun_ve1", 1);
                   }
                   s_stdLib = "/opt/nec/ve/lib/libveda.vso";
                }
#else
		// Init StdLib Path --------------------------------------------
		// Stolen from: https://stackoverflow.com/questions/33151264/get-dynamic-library-directory-in-c-linux
		Dl_info dl_info;
		dladdr((void*)&veda::setInitialized, &dl_info);
		std::string home(dl_info.dli_fname);
		auto pos = home.find_last_of('/');	assert(pos != std::string::npos);
		pos = home.find_last_of('/', pos-1);	assert(pos != std::string::npos);
		home.replace(pos, std::string::npos, "");

		// Set Paths ---------------------------------------------------
		if(!std::getenv("VEORUN_BIN")) {
			std::string veorun(home);
			veorun.append("/libexec/aveorun");
			if(std::getenv("VEDA_FTRACE"))
				veorun.append("-ftrace");
                         veorun.append("_");
                         const char* arch = veda::ve_arch_find();
                         if(!strncmp(arch, "ve", 2))
                                veorun.append(arch);
                         else
                                throw VEDA_ERROR_UNKNOWN_ARCHITECTURE;
			setenv("VEORUN_BIN", veorun.c_str(), 1);
		}

		s_stdLib = home;
		s_stdLib.append("/libve/libveda.vso");
#endif

		L_TRACE("AVEORUN: %s", std::getenv("VEORUN_BIN"));
		L_TRACE("libveda: %s", s_stdLib.c_str());

		// Set VE_LD_LIBRARY_PATH if is not set ------------------------
		setenv("VE_LD_LIBRARY_PATH", ".", 0);
	}
	else if(!value){
		char tmp[3];
		sprintf(tmp, "%d", s_envOmpThread);
		//Resetting the "VE_OMP_NUM_THREADS" value to its original value at exit
		setenv("VE_OMP_NUM_THREADS", tmp, 1);
	}

	// Set Initialized
	veda::Semaphore::init();
	s_initialized = value;
}

//------------------------------------------------------------------------------
VEDAresult VEOtoVEDA(const int err) {
	switch(err) {
		case VEO_COMMAND_OK:		return VEDA_SUCCESS;
		case VEO_COMMAND_EXCEPTION:	return VEDA_ERROR_VEO_COMMAND_EXCEPTION;
		case VEO_COMMAND_ERROR:		return VEDA_ERROR_VEO_COMMAND_ERROR;
		case VEO_COMMAND_UNFINISHED:	return VEDA_ERROR_VEO_COMMAND_UNFINISHED;
	}
	return VEDA_ERROR_VEO_COMMAND_UNKNOWN_ERROR;
}


const char* ve_arch_find()
{
        struct dirent* dp;
        DIR* fd;
        const char *arch_name=NULL;
        vedl_handle *vedl_hndl = NULL;
        if((fd = opendir("/dev/")) == NULL)
                throw VEDA_ERROR_NO_DEVICES_FOUND;

        while((dp = readdir(fd)) != NULL) {
                if(!strncmp(dp->d_name, "veslot", 6)) {
                        char device[] = "/dev/veslotX";
                        strncpy(device+5,dp->d_name,7);

			int fd1 = open(device, O_RDONLY, 0);
			if(fd1 < 0)
				continue;

			close(fd1);
                        vedl_hndl = vedl_open_ve(device, -1);
                        if (vedl_hndl == NULL)
                                continue;

                        arch_name = vedl_get_arch_class_name(vedl_hndl);
                        break;
                }
        }
        closedir(fd);
	if(vedl_hndl)
                vedl_close_ve(vedl_hndl);

	if(arch_name == NULL)
                throw VEDA_ERROR_NO_DEVICES_FOUND;

        return arch_name;
}

}
