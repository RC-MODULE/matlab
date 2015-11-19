#include <aura/aura.h>
#include <ion/ion.h>

//#include "nmpp.h"




void aura_arm_nmppsFwdFFT256(struct aura_node *n )
{
	int src[512];
	int tst[512];
	int i;
	src[0]=1;
	tst[0]=1;
	for(i=1; i<512; i++){
	    src[i]=i;
	}
	
	int ret;
	struct aura_buffer *retbuf; 
	ret = aura_call(n, "FFT256Fwd", &retbuf, src);
	if (ret != 0) 
	    BUG(n, "Call failed!");

	int* dst=aura_buffer_get_bin(retbuf,256*2*4);

	//for(i=0; i<256; i++){
	//   if (dst[i]!=tst[i])
	//      slog(0, SLOG_ERROR, "ARM: test  aura_arm_nmppsAbs_s32_256 is NOT ok: [%d] %d vs %d",i, dst[i], tst[i]); 
	//}
	aura_buffer_release(n, retbuf);
	slog(0, SLOG_INFO, "ARM: TEST aura_arm_nmppsFFT256Fwd is ok");
}

void aura_arm_nmppsSum_s32_256(struct aura_node *n )
{
	int vec[256];
	int i;
	int check_value=0;
	for(i=0; i<256; i++){
	    vec[i]=i;
	    check_value+=i;
	}
	
	int ret;
	struct aura_buffer *retbuf; 
	ret = aura_call(n, "Sum_s32_256", &retbuf, vec);
	if (ret != 0) 
		BUG(n, "Call failed!");

	uint32_t v = aura_buffer_get_u32(retbuf);
	if (v != check_value)
	{
		slog(0, SLOG_ERROR, "test  aura_nmppsSum_s32_256 is NOT ok: %dx vs %dx", v, check_value); 
	}
	aura_buffer_release(n, retbuf);
	slog(0, SLOG_INFO, "ARM: TEST nmppsSum_s32_256 is ok");
}


void aura_arm_nmppsAbs_s32_256(struct aura_node *n )
{
	int src[256];
	int tst[256];
	int i;
	src[0]=1;
	tst[0]=1;
	for(i=1; i<256; i++){
	    if (src[i-1]>0)
	      src[i]=-src[i-1]-1;
	    else 
	      src[i]=-src[i-1]+1;
	    
	    if (src[i]>0)
		tst[i]=src[i];
	    else 
		tst[i]=-src[i];
	    //slog(0, SLOG_INFO, "ARM: test  aura_arm_nmppsAbs_s32_256 : [%d] %d vs %d",i, src[i], tst[i]); 
	}
	
	int ret;
	struct aura_buffer *retbuf; 
	ret = aura_call(n, "Abs_s32_256", &retbuf, src);
	if (ret != 0) 
	    BUG(n, "Call failed!");

	int* dst=aura_buffer_get_bin(retbuf,256*4);

	for(i=0; i<256; i++){
	    if (dst[i]!=tst[i])
	      slog(0, SLOG_ERROR, "ARM: test  aura_arm_nmppsAbs_s32_256 is NOT ok: [%d] %d vs %d",i, dst[i], tst[i]); 
	}
	aura_buffer_release(n, retbuf);
	slog(0, SLOG_INFO, "ARM: TEST aura_arm_nmppsAbs_s32_256 is ok");
}


void aura_arm_nmppsAdd_s32_256(struct aura_node *n )
{
	uint32_t src0[256];
	uint32_t src1[256];
	uint32_t tst[256];
	
	int i;
	 
	src0[0]=1;
	tst[0]=1;
	for(i=0; i<256; i++){
	    src0[i]=i;
	    src1[i]=1000+i;
	    tst [i]=src0[i]+src1[i];
	    //slog(0, SLOG_INFO, "ARM: test  aura_arm_nmppsAdd_s32_256 : [%d] %d  %d",i, src0[i], src1[i]); 
	}
	
	int ret;
	struct aura_buffer *retbuf; 
	//slog(0, SLOG_DEBUG, "a 0x%x b 0x%x", (uint32_t) src0, (uint32_t) src1);
	ret = aura_call(n, "Add_s32_256", &retbuf, (uint32_t) src0, (uint32_t) src1);
	if (ret != 0) 
	    BUG(n, "Call failed!");
	//aura_hexdump("retbuf", retbuf->data, retbuf->size);
	int* dst=aura_buffer_get_bin(retbuf,256*4);

	for(i=0; i<256; i++){
	    if (dst[i]!=tst[i])
	      slog(0, SLOG_ERROR, "ARM: test  aura_arm_nmppsAdd_s32_256 is NOT ok: [%d] %d vs %d",i, dst[i], tst[i]); 
	}
	aura_buffer_release(n, retbuf);
	slog(0, SLOG_INFO, "ARM: TEST aura_arm_nmppsAdd_s32_256 is ok");
}




int main() {

	slog_init(NULL, 0);

	int ret; 
	struct aura_node *n = aura_open("nmc", "./fft-nmc.abs");
	if (!n) {
		slog (0, SLOG_ERROR, "Failed to open node");
		exit(1);
	}
	aura_wait_status(n, AURA_STATUS_ONLINE);

	aura_arm_nmppsFwdFFT256(n);
	aura_arm_nmppsAbs_s32_256(n);
	aura_arm_nmppsAdd_s32_256(n);
	aura_arm_nmppsSum_s32_256(n);
	
	
	//test_u32(n);
	//test_sum_u32(n);
	//test_u64(n);
	//test_u32u32(n);
	
	//test_bin222_bullshit(n);
	//test_bin2(n);
	//test_bin2(n);
	//test_bin2_bullshit(n);
	//bin2_bullshit(2);
	//test_u32u64(n);
	printf("===========\n");
	aura_close(n);
	
	slog (0, SLOG_INFO, "Finished");

	return 0;
}


