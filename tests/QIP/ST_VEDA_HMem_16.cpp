
#include <veda.h>
#include <vera.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <thread>
#include <unistd.h>

#define CHECK(err) check(err, __FILE__, __LINE__)
#define SIZE 16*sizeof(int64_t)
void run(int);
void check(VEDAresult err, const char* file, const int line) {
        if(err != VEDA_SUCCESS) {
                const char* name = 0;
                vedaGetErrorName(err, &name);
                printf("Error: %i %s @ %s (%i)\n", err, name, file, line);
                assert(false);
                exit(1);
        }
}
int main(int argc, char** argv) {
    int devcnt=0,dev=0;
    if(argc<=1) {
        printf("Pass the program iteration count to run for longer time\ni.e. $./test_binary 25000 for 1 hour approx\n");
        exit(1);
    }
    int iteration_count = atoi(argv[1]);
    int count=0;
   
    CHECK(vedaInit(0));
    CHECK(vedaDeviceGetCount(&devcnt));
    CHECK(vedaExit());

    printf("Starting long run with %d threads on %d devices\n",devcnt,devcnt);
    while(count++ < iteration_count){
        CHECK(vedaInit(0));
        for(dev=0; dev<devcnt; dev++) {
            std::thread t(run,dev);
            t.join();

        }
        CHECK(vedaExit());
    }
    return 0;
}

void run(int dev){
	static size_t pass=0,fail=0;
	int cnt=0;
	VEDAcontext cont;
        CHECK(vedaCtxCreate(&cont, VEDA_CONTEXT_MODE_OMP, dev));
        CHECK(vedaCtxStreamCnt(&cnt));
        VEDAptr<int64_t> ptr_free = 0;
        CHECK(vedaMemAlloc(&ptr_free, SIZE));
        if(ptr_free.size() != SIZE)
        {
        printf("Memory allocation failed\n");
        exit(0);
        }
        VEDAmodule mod;
        const char* modName = "libomp_func_ve.vso";
        CHECK(vedaModuleLoad(&mod, modName));
        printf("vedaModuleLoad(%p, \"%s\")\n", mod, modName);
        
        VEDAfunction func;
        const char* funcName = "ve_non_omp_func";
        CHECK(vedaModuleGetFunction(&func, mod, funcName));
        printf("vedaModuleGetFunction(%p, %p, \"%s\")\n", func, mod, funcName);
        VEDAargs args;
        CHECK(vedaArgsCreate(&args));
        CHECK(vedaArgsSetPtr(args, 0, ptr_free));
        CHECK(vedaArgsSetI32(args, 1, SIZE ));
        printf("vedaLaunchKernel\n");
        CHECK(vedaLaunchKernel(func, 0, args));
        printf("vedaCtxSynchronize\n");
        CHECK(vedaCtxSynchronize());
        int *reference = (int *)malloc(SIZE);
	void *updated_data;
	updated_data = (int *)malloc(SIZE);
        for (unsigned int i = 0; i < SIZE/sizeof(int64_t); ++i) 
		reference[i] = i;
        
        CHECK(vedaHMemcpyDtoX(updated_data, ptr_free, SIZE));
	int *a = (int *)updated_data;
        bool success = true;
        for (unsigned int i = 0; i < SIZE/sizeof(int64_t); i++)
        if (reference[i] != a[i] ){
          success = false;
        break;
        }
        if(success == true)
        pass++;
        else
        fail++;
        printf("Passed=%d Failed=%d\n",pass,fail);
        
        free(reference);
	free(updated_data);
        printf("vedaMemFreeAsync\n");
        CHECK(vedaMemFree(ptr_free));
        printf("vedaCtxDestroy\n");
        CHECK(vedaCtxDestroy(cont));
}
