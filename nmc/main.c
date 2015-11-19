#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <easynmc/easynmc.h>
#include <easynmc/aura.h>
#include "nmpp.h"
#include "fft.h"

unsigned int *pinmux  = (unsigned int *) 0x0800CC21;
unsigned int *port    = (unsigned int *) 0x0800A403;
unsigned int *ddr     = (unsigned int *) 0x0800A407;

void aura_panic() { 
	for (;;) { 
		*port ^= (1<<6);	
		*port ^= (1<<7);	
	} ;
}

void aura_hexdump (char *desc, unsigned int *addr, int len) {
	int i; 
	for (i=0; i<len; i++) { 
		printf("0x%x: 0x%x\n", ((unsigned int)addr << 2), *addr);
		addr++;
	}
}




void aura_nmc_nmppsSum_s32_256(void* in, void* out)
{
    int sum=0;
    int i;
    printf("NMC: Aura RPC call nmppsSum_32s\n"); 
    int *ptr = aura_get_buf(256*4);
    for(i=0; i<256; i++){
	sum+=ptr[i];
      //printf("%d\n",ptr[i]);
    }
    
    aura_put_u32(sum);   
    //struct aura_buffer *buf = aura_buffer_request(node, 4096);
    //uint32_t nmaddr = aura_buffer_to_nmc(buf);
    //...
    
    //aura_buffer_release(node, buf);
 
}


void aura_nmc_nmppsAbs_s32_256(void *in, void *out)
{
	int i=0;
	int sum=0;
	int *ptr = aura_get_buf(256*4);
	int dst[256];	
	for(i=0; i<256; i++){
	    if (ptr[i]>=0)
	      dst[i]=ptr[i];
	    else 
	      dst[i]=-ptr[i];
	    //printf("NMC: hello from aura_nmc_nmppsAbs_s32_256 %d %d\n",ptr[i] , dst[i] );
	}
	printf("NMC: hello from aura_nmc_nmppsAbs_s32_256\n");
	aura_put_buf(dst, 256*4);
}

void aura_nmc_nmppsAdd_s32_256(void *in, void *out)
{
	int i=0;
	int sum=0;
	int dst[256];	
	int *src0 = aura_get_buf(256*4);
	int *src1 = aura_get_buf(256*4);
	
	for(i=0; i<256; i++){
	    dst[i]=src0[i]+src1[i];
	    //printf("NMC: from aura_nmc_nmppsAdd_s32_256 [%d] %d %d %d\n",i, dst[i] , src0[i], src1[i] );
	}
	printf("NMC: hello from aura_nmc_nmppsAdd_s32_256\n");
	aura_put_buf(dst, 256*4);
}


void aura_nmc_nmppsFwdFFT256(void *in, void *out)
{
	int i=0;
	int sum=0;
	int *dst ;	
	int *src = aura_get_buf(256*2*4);
	

	int *dst = nmppsMalloc32(256*2*4);	
	
	NmppsFFT spec;
	nmppsFwdFFT256InitAlloc(nmppsMalloc32, nmppsFree32, &spec);
	nmppsFwdFFT256(src,dst;)
	nmppsFFTFree(spec);
	
	
	printf("NMC: hello from aura_nmc_nmppsFwdFFT256\n");
	aura_put_buf(dst, 256*4);
//	nmppsFree(dst);
}

int main(int argc, char **argv)
{
	printf("NMC: Aura RPC demo \n");
	aura_init();
	aura_loop_forever();

}

