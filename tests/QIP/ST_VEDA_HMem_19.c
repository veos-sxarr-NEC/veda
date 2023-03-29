
#include <veda.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <stdbool.h>

#define CHECK(err) check(err, __FILE__, __LINE__)
#define SIZE 256*sizeof(int)

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

    if(argc<=1) {
        printf("Pass the program iteration count to run for longer time\ni.e. $./test_binary 25000 for 1 hour approx\n");
        exit(1);
    }
    int iteration_count = atoi(argv[1]);
    size_t pass=0,fail=0;
    while(pass+fail < iteration_count){
        int devcnt=0, cnt=0;
        CHECK(vedaInit(0));
        CHECK(vedaDeviceGetCount(&devcnt));
        for(int dev = 0; dev < devcnt; dev++) {
        	VEDAcontext cont;
                CHECK(vedaCtxCreate(&cont, VEDA_CONTEXT_MODE_SCALAR, dev));
                CHECK(vedaCtxStreamCnt(&cnt));
                VEDAdeviceptr ptr_free = 0;
                CHECK(vedaMemAlloc(&ptr_free, SIZE));
		VEDAmodule mod;
		const char* modName = "./libST_VEDA_HMEM_19_ve.vso";
		CHECK(vedaModuleLoad(&mod, modName));
		printf("vedaModuleLoad(%p, \"%s\")\n", mod, modName);

		VEDAfunction func;
		const char* funcName = "init";
		CHECK(vedaModuleGetFunction(&func, mod, funcName));
		printf("vedaModuleGetFunction(%p, %p, \"%s\")\n", func, mod, funcName);
		VEDAargs args;
		CHECK(vedaArgsCreate(&args));
		CHECK(vedaArgsSetPtr(args, 0, ptr_free));
		CHECK(vedaArgsSetI32(args, 1, 5 ));
		CHECK(vedaArgsSetI32(args, 2, SIZE/sizeof(int)));
		printf("vedaLaunchKernel\n");
		CHECK(vedaLaunchKernel(func, 0, args));
		printf("vedaCtxSynchronize\n");
		CHECK(vedaCtxSynchronize());
		int *reference = (int *)malloc(SIZE);
		void *updated_data;
		updated_data = (int *)malloc(SIZE);
		for (unsigned int i = 0; i < SIZE/sizeof(int); ++i) reference[i] = 5;

		CHECK(vedaHMemcpyDtoX(updated_data, ptr_free, SIZE));
		int *a = (int *)updated_data;
		bool success = true;
		for (unsigned int i = 0; i < SIZE/sizeof(int); i++)
		    if (reference[i] != a[i]){
      			success = false;
			break;
    		}	
		if(success == true)
			pass++;
		else
			fail++;
		printf("Passed=%d Failed=%d\n",pass,fail);

		VEDAfunction func1;
                funcName = "check";
                CHECK(vedaModuleGetFunction(&func1, mod, funcName));
                printf("vedaModuleGetFunction(%p, %p, \"%s\")\n", func1, mod, funcName);
                VEDAargs args1;
                VEDAdeviceptr result= 0;
                CHECK(vedaMemAlloc(&result, sizeof(int)));
                CHECK(vedaArgsCreate(&args1));
                CHECK(vedaArgsSetPtr(args1, 0, ptr_free));
                CHECK(vedaArgsSetI32(args1, 1, 5 ));
                CHECK(vedaArgsSetI32(args1, 2, SIZE/sizeof(int)));
		CHECK(vedaArgsSetPtr(args1, 3, result));
                printf("vedaLaunchKernel\n");
                CHECK(vedaLaunchKernel(func1, 0, args1));
                printf("vedaCtxSynchronize\n");
                CHECK(vedaCtxSynchronize());

                if(result == 0)
                {
                     printf( "VE check : Failed \n");
                }
                else
                {
                     printf( "VE check : Passed \n");
                }

		free(reference);
		free(updated_data);
		printf("vedaMemFreeAsync\n");
		CHECK(vedaMemFree(ptr_free));
		CHECK(vedaMemFree(result));
		printf("vedaCtxDestroy\n");
		CHECK(vedaCtxDestroy(cont));
        }
        CHECK(vedaExit());
    }
}
