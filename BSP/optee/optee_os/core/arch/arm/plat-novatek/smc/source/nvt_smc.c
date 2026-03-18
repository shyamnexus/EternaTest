#ifdef CFG_PL310
#include <kernel/tz_ssvce_pl310.h>
#include <kernel/cache_helpers.h>
#endif
#include <kernel/thread.h>
#include <mm/core_mmu.h>
#include <mm/core_memprot.h>
#include <sm/optee_smc.h>
#include "../include/smc_func_id/smc_id_def.h"
#include "../include/smc_func_id/hw_smc_id.h"
#include "../include/smc_func_id/software_smc_id.h"
#include "../include/crypto/nvt_aes_smc.h"
#include "../include/crypto/nvt_sha_smc.h"
#include "../include/crypto/nvt_rsa_smc.h"
#include "../include/crypto/nvt_ecdsa_smc.h"
#include "../include/nvt_efuse_smc.h"
#include <efuse_protected.h>
#include <crypto/crypto.h>
#include <string.h>
#include <tee/cache.h>

#include <plat/rsa.h>
#include <mbedtls/bignum.h>
#include <io.h>
#include <kernel/panic.h>

#ifdef CFG_PL310
void arm_cl2_enable(vaddr_t pl310_base);
void arm_cl2_disable(vaddr_t pl310);
int arm_cl2_smc_cfg(vaddr_t pl310_base, unsigned int offset, unsigned int val, unsigned int mask);
#endif

/*
    key_field define
    define from  lib/source/crypto/nvt_ecb.c
*/
#define NVT_OTP_KEY_KM_KEY_SET0 0x00000000
#define NVT_OTP_KEY_KM_KEY_SET1 0x00000001
#define NVT_OTP_KEY_KM_KEY_SET2 0x00000002
#define NVT_OTP_KEY_KM_KEY_SET3 0x00000003
#define NVT_OTP_KEY_KM_KEY_SET4 0x00000004

///< Nova OTP key tag  sync from core/arch/arm/plat-novatek/lib/source/crypto/nvt_ecb.c
#define NVT_OTP_KEY_TAG         MAKEFOURCC('N', 'O', 'T', 'P')

static uintptr_t pa_to_va(uintptr_t pa, uintptr_t size)
{
	if(core_pbuf_is(CORE_MEM_NSEC_SHM,pa,size))
	{
		return (uintptr_t)phys_to_virt(pa, MEM_AREA_NSEC_SHM, size);
	}
	else{
		EMSG("pa not in CORE_MEM_NSEC_SHM  pa%lx size:%lx\n",pa,size);
		return 0;
	}
}

static void smc_hello_world(struct thread_smc_args *args)
{

	struct TEST_DATA{
		char test_intput[10];
	};

	struct TEST_DATA * va;

	DMSG("in hello world test\n");

	DMSG("register: %lx %lx %lx %lx %lx %lx %lx\n",(uintptr_t)args->a0,
												(uintptr_t)args->a1,
												(uintptr_t)args->a2,
												(uintptr_t)args->a3,
												(uintptr_t)args->a4,
												(uintptr_t)args->a5,
												(uintptr_t)args->a6);

	//get share memory from normal world
	va = (struct TEST_DATA *)pa_to_va(args->a2, sizeof( struct TEST_DATA));

	if(va ==NULL)
	{
		EMSG("pa_to_va fail\n");
		return;
	}
	DMSG("normal world share memory string:%s\n",va->test_intput);

	args->a0 = 0x11;
	args->a1 = 0x12;
	args->a2 = 0x13;
	args->a3 = 0x14;

	DMSG("return register:%lx %lx %lx %lx\n",(uintptr_t)args->a0,
											(uintptr_t)args->a1,
											(uintptr_t)args->a2,
											(uintptr_t)args->a3);
}
#if 0
static void data_reverse(unsigned char* input_data, unsigned int size)
{
        unsigned int i=0;
        unsigned char tmp=0;
        for(i=0;i< (size/2);i++)
        {
                tmp = input_data[size - 1 - i];
                input_data[size - 1 - i] = input_data[i];
                input_data[i] = tmp;
        }

}
static void fill_data_r(UINT8 * s, UINT8 * target, int len)
{
    int   i;

    for(i=0; i<len; i++)
        target[len-i-1] = s[i];
}
#endif
#if 0
static char ascii_to_digit( char low_c , char high_c )
{

        if( low_c >= 0x30 && low_c <= 0x39 )
        {
                low_c = low_c -0x30;
        }
        else if(low_c >= 0x41 && low_c <= 0x46)
        {
                low_c = low_c - 0x37;
        }
        else if(low_c >= 0x61 && low_c <= 0x66)
        {
                low_c = low_c - 0x57;
        }
        else{
                DMSG("low_c:%c get not change to digit\n",low_c);
        }

        if( high_c >= 0x30 && high_c <= 0x39 )
        {
                high_c = high_c -0x30;
        }
        else if(high_c >= 0x41 && high_c <= 0x46)
        {
                high_c = high_c - 0x37;
        }
        else if(high_c >= 0x61 && high_c <= 0x66)
        {
                high_c = high_c - 0x57;
        }
        else{
                DMSG("high_c:%c get not change to digit\n",high_c);
        }
        return ( (high_c << 4) | (low_c & 0x0f) );
}
static int nvt_rsa_test(void)
{

__aligned(4)	unsigned char str_n[]="E04E2BBD71B470A2B9B3811E10CA56BC2560A160F3742C8AC807AFEF00B26EDC020202F3AB8F6FA31D2A0F19E803DF288AB6F0783323D01229990707758A684E3BCD379EEBF12B17E716B4CA2DE351A7C06DE8EC46331CC4C25B6C16A1C80582BC825073AD800A0A34A8AC3CDCF1232BC4389C70692A9F653B9881C013E698E2140CFB8B10E7CB21B501BD4260C1893890D9A0F5A948504156F2D489A4F13DA3D97CC969B453C35DAA850528865B02490A736E2E84ABC4AFB8D1F9AEED8920A99A765B80453F5B1E75D825E58671E1B973603EB970489E386FB7193D15FFEDF1448275088CE905E97955C775C38700589B62A56BD30610ACC45A045BAA203DBF";
__aligned(4)	unsigned char str_e[]="010001";
__aligned(4)	unsigned char str_d[]="050E8E1AE3C4DB2338078D7A258BC23FD4A1E491807B96837C75FBE7E9FE9BD436F15FAAA9ABFEAAD13E010027046B5409288DAC500ED72289B08B01868B5BC5A3C08128028EB9D77C9CBCEFAFF027879E257C325374D489B50E05964F0868E3652794F3BEAE1BCBD61B1562FBE22DD64F42AD78354EC2303093441D95F5456E0C90DF9965FAF47E2ECC488D1117A275780C0621D5865D2F2E750293327ADE66192FE7EC76006680CAE5929E451048D47466428089FB9104B6F6809740300B160212F00B70A57D35407B00EE2D9AFFA8FC2B0468810FC4AE378AC1E62AA786708211C2880D4DBD2C2A7149CE4FC60998EB28882B0099000A5EE3C86C4053FBCD";
__aligned(4)	unsigned char input[256]={0};
__aligned(4)	unsigned char output[256]={0};
__aligned(4)	unsigned char output2[256]={0};

	#define MAX_SIZE 256
__aligned(4)	unsigned char n[MAX_SIZE]={0};
__aligned(4)	unsigned char e[MAX_SIZE]={0};
__aligned(4)	unsigned char d[MAX_SIZE]={0};
__aligned(4)	unsigned int i=0;
__aligned(4)	unsigned char n_r[MAX_SIZE]={0};
__aligned(4)	unsigned char e_r[MAX_SIZE]={0};
__aligned(4)	unsigned char d_r[MAX_SIZE]={0};
__aligned(4)	unsigned int n_size= strlen(str_n)/2;
__aligned(4)	unsigned int e_size= strlen(str_e)/2;
__aligned(4)	unsigned int d_size= strlen(str_d)/2;
	for(i=0; i< MAX_SIZE;i++)
	{
		n[i]=ascii_to_digit( str_n[2*i+1] , str_n[2*i] );
		d[i]=ascii_to_digit( str_d[2*i+1] , str_d[2*i] );
	}

	e[0]=0x01;
	e[1]=0x00;
	e[2]=0x01;

	DMSG("n_size:%u e_size:%u d_size:%u\n",n_size,e_size,d_size);

	DMSG("n:%02x %02x %02x %02x last n %02x %02x %02x %02x\n",n[0],n[1],n[2],n[3],n[n_size -1],n[n_size -2],n[n_size -3],n[n_size -4]);
	DMSG("e:%02x %02x %02x %02x last e %02x %02x %02x %02x\n",e[0],e[1],e[2],e[3],e[n_size -1],e[n_size -2],e[n_size -3],e[n_size -4]);
	DMSG("d:%02x %02x %02x %02x last e %02x %02x %02x %02x\n",d[0],d[1],d[2],d[3],d[d_size -1],d[d_size -2],d[d_size -3],d[d_size -4]);

	rsa_open();
	rsa_setConfig(RSA_CONFIG_ID_KEY_WIDTH,    RSA_KEY_2048);
	rsa_setConfig(RSA_CONFIG_ID_MODE,         RSA_MODE_NORMAL);

	fill_data_r( n, n_r,  MAX_SIZE);
	fill_data_r( d, d_r,  MAX_SIZE);
	fill_data_r( e, e_r,  e_size);
	DMSG("revert key:\n");
	DMSG("n:%02x %02x %02x %02x last n %02x %02x %02x %02x\n",n_r[0],n_r[1],n_r[2],n_r[3],n_r[n_size -1],n_r[n_size -2],n_r[n_size -3],n_r[n_size -4]);
	DMSG("e:%02x %02x %02x %02x last e %02x %02x %02x %02x\n",e_r[0],e_r[1],e_r[2],e_r[3],e_r[n_size -1],e_r[n_size -2],e_r[n_size -3],e_r[n_size -4]);
	DMSG("d:%02x %02x %02x %02x last e %02x %02x %02x %02x\n",d_r[0],d_r[1],d_r[2],d_r[3],d_r[d_size -1],d_r[d_size -2],d_r[d_size -3],d_r[d_size -4]);

	DMSG("n_r addr:%x\n",n_r);
	DMSG("e_r addr:%x\n",e_r);
	rsa_setkey_n(n_r,   n_size/4, RSA_ORDER_LSB_FIRST);
	rsa_setkey_ed(e_r, n_size/4, RSA_ORDER_LSB_FIRST);

	input[0]=0;
	input[1]=1;
	input[2]=2;
	input[3]=3;
	input[4]=4;
	input[5]=5;
	input[6]=6;


	DMSG("input addr:%x\n",input);
	rsa_pio_enable(input, n_size/4, RSA_ORDER_LSB_FIRST);
	DMSG("output addr:%x\n",output);
	rsa_getOutput(output, n_size/4);
#if 1
	DMSG("test encrypt:\n");
	for(i=0; i< n_size;i++)
	{
		DMSG("%x ",output[i]);
	}
#endif

	DMSG("d_r addr :%x\n",d_r);
	rsa_setkey_ed(d_r, n_size/4, RSA_ORDER_LSB_FIRST);
	rsa_pio_enable(output, n_size/4, RSA_ORDER_LSB_FIRST);
	DMSG("output2 addr :%x\n",output2);
	rsa_getOutput(output2, n_size/4);

	DMSG("test decrypt:\n");
	for(i=0; i< n_size;i++)
	{
		DMSG("%x ",output2[i]);
	}
	rsa_close();
	return 0;
}
#endif
#if 0
static void enctry_rsa_decrypt_by_pub_key_no_pending(struct thread_smc_args *args)
{
	unsigned int key_size = args->a1;
	unsigned char *key_buf = (unsigned char *)pa_to_va(args->a2, key_size);
	unsigned int encrypt_size = args->a3;
	unsigned long  *encrypt_data = (unsigned long  *)pa_to_va(args->a4, encrypt_size);
	unsigned int rsa_lenth_type = args->a5; // 2048 or 1024
	unsigned int output_size = args->a6;
	unsigned long  *output_data = (unsigned long  *)pa_to_va(args->a7, output_size);
	unsigned char *N_key = NULL;
	unsigned char *E_key = NULL;
	unsigned int N_size = rsa_lenth_type/8;
	unsigned int E_size = key_size - N_size;
	int ret=0;
	__aligned(4)	unsigned long n_r[64]={0};
	__aligned(4)	unsigned long e_r[64]={0};

	if(key_size <= 0)
	{
		EMSG("key_size:%d fail\n",key_size);
		args->a0 = -1;
		return;
	}
	if(N_size > rsa_lenth_type/8){
		EMSG("N_size:%d error\n",N_size);
		args->a0 = -1;
		return;

	}

	if(key_buf == NULL)
	{
		EMSG("key_buf pa to va fail pa:%x size:%x\n",args->a2,key_size);
		args->a0 = -1;
		return;
	}

	if(encrypt_size <=0)
	{
		EMSG("encrypt_size:%d fail\n",key_size);
		args->a0 = -1;
		return;
	}
	if(encrypt_data == NULL)
	{
		EMSG("encrypt_data pa to va fail pa:%x size:%x\n",args->a4,key_size);
		args->a0 = -1;
		return;
	}
	if(output_data == NULL)
	{
		EMSG("output_data pa to va fail pa:%x size:%x\n",args->a7,key_size);
		args->a0 = -1;
		return;

	}
	if(output_size <=0)
	{
		EMSG("output_size:%d fail\n",key_size);
		args->a0 = -1;
		return;
	}

	if((encrypt_size % N_size)!=0)
	{
		EMSG(" n key size:%d  but encrypt size:%u not alignment key size\n",N_size,encrypt_size);
		args->a0 = -1;
		return;
	}

	N_key = key_buf;
	E_key = key_buf + N_size; // N size = rsa_type / 8

	fill_data_r( (unsigned char*)N_key, (unsigned char *)n_r,  N_size);
	fill_data_r( (unsigned char*)E_key, (unsigned char *)e_r,  E_size);

	rsa_open();
	if(rsa_lenth_type == 2048)
	{
		DMSG("set RSA_KEY_2048\n");
		rsa_setConfig(RSA_CONFIG_ID_KEY_WIDTH,    RSA_KEY_2048);
	}
	else if(rsa_lenth_type == 1024)
	{
		DMSG("set RSA_KEY_1024\n");
		rsa_setConfig(RSA_CONFIG_ID_KEY_WIDTH,    RSA_KEY_1024);
	}
	else
	{
		EMSG("rsa_lenth_type:%d error, please set 1024 or 2048\n",rsa_lenth_type);
		args->a0 = -1;
		return;
	}
	//DMSG("encrypt addr:%lx  output addr:%lx N_size:%u E_size:%u\n", encrypt_data, output_data,N_size,E_size);

	#if 1
	DMSG("revert encrypt data\n");
	data_reverse((unsigned char *)encrypt_data, encrypt_size);
	#endif
	rsa_setConfig(RSA_CONFIG_ID_MODE,         RSA_MODE_NORMAL);
	rsa_setkey_n(n_r,   N_size/4, RSA_ORDER_LSB_FIRST);
	rsa_setkey_ed(e_r, N_size/4, RSA_ORDER_LSB_FIRST); // need set N_size bytes

	rsa_pio_enable(encrypt_data, N_size/4, RSA_ORDER_LSB_FIRST);
	rsa_getOutput(output_data, N_size/4);
	rsa_close();

	data_reverse((unsigned char*) output_data, encrypt_size);
	args->a0 = 0;

	args->a1 = encrypt_size;
	ret = cache_operation(TEE_CACHEFLUSH, (void *)output_data, encrypt_size);
	if(ret != TEE_SUCCESS)
	{
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n",ret);
		args->a0 = -1;
        }
	#if 0
		DMSG("decrypt revert data:\n");
		DMSG("%lx %lx %lx %lx %lx %lx %lx %lx\n",
		output_data[0],output_data[1],output_data[2],output_data[3],output_data[4],output_data[5],output_data[6],output_data[7]);
	#endif
}
#endif
#if 0
static void entry_rsa_decrypt(struct thread_smc_args *args)
{

	unsigned int key_size = args->a1;
	unsigned char *key_buf = (unsigned char *)pa_to_va(args->a2, key_size);
	unsigned int encrypt_size = args->a3;
	unsigned char *encrypt_data = (unsigned char *)pa_to_va(args->a4, encrypt_size);
	unsigned int rsa_lenth_type = args->a5; // 2048 or 1024
	unsigned int output_size = args->a6;
	unsigned char *output_data = (unsigned char *)pa_to_va(args->a7, output_size);
	unsigned char *N_key = NULL;
	unsigned char *D_key = NULL;
	unsigned int N_size = rsa_lenth_type/8;
	unsigned int D_size = key_size - N_size;
	struct rsa_keypair key;
	int ret=0;


	if(key_size <= 0)
	{
		EMSG("key_size:%d fail\n",key_size);
		args->a0 = -1;
		return;
	}
	if(N_size > rsa_lenth_type/8 || D_size  > rsa_lenth_type/8){
		EMSG("N_size:%d or D_size:%d error\n",N_size,D_size);
		args->a0 = -1;
		return;
	}

	if(key_buf == NULL)
	{
		EMSG("key_buf pa to va fail pa:%x size:%x\n",args->a2,key_size);
		args->a0 = -1;
		return;
	}

	if(encrypt_size <=0)
	{
		EMSG("encrypt_size:%d fail\n",key_size);
		args->a0 = -1;
		return;
	}

	if(encrypt_data == NULL)
	{
		EMSG("encrypt_data pa to va fail pa:%x size:%x\n",args->a4,key_size);
		args->a0 = -1;
		return;
	}
	if(output_data == NULL)
	{
		EMSG("output_data pa to va fail pa:%x size:%x\n",args->a7,key_size);
		args->a0 = -1;
		return;
	}
	if(output_size <=0)
	{
		EMSG("output_size:%d fail\n",key_size);
		args->a0 = -1;
		return;
	}

	if((encrypt_size % N_size)!=0)
	{
		EMSG(" n key size:%d  but encrypt size:%u not alignment key size\n",N_size,encrypt_size);
		args->a0 = -1;
		return;
	}

	N_key = key_buf;
	D_key = key_buf + N_size; // N size = rsa_type / 8


	#if 0
	nvt_rsa_test();
	return;
	#endif
	crypto_acipher_alloc_rsa_keypair(&key, rsa_lenth_type);

	crypto_bignum_bin2bn(N_key, N_size, key.n);
	crypto_bignum_bin2bn(D_key, D_size, key.d);

	unsigned int offset =0;
	unsigned int out_offset =0;
	unsigned tmp_size = encrypt_size;
	unsigned int tmp_output_size = 0;

	while(tmp_size > 0 && output_size > 0)
	{
		tmp_output_size = output_size;
		args->a0 = crypto_acipher_rsaes_decrypt(TEE_ALG_RSAES_PKCS1_V1_5, &key, NULL, 0, (const uint8_t *)(encrypt_data+offset),
			N_size, (output_data+out_offset), &tmp_output_size);
		if(args->a0 != TEE_SUCCESS )
		{
			EMSG("crypto_acipher_rsaes_decrypt fail %x\n",args->a0);
			return;
		}
		out_offset = out_offset + tmp_output_size;
		output_size = output_size - tmp_output_size;

	}

	args->a1 = out_offset;
	ret = cache_operation(TEE_CACHEFLUSH, (void *)output_data, out_offset);
	if(ret != TEE_SUCCESS)
	{
		EMSG("cache_operation TEE_CACHEFLUSH fail ret:%x\n",ret);
		args->a0 = -1;
	}
	return;
}
#endif

static void arm_actlr_smp_cfg(bool en)
{
#ifdef CFG_ARM32_core
	if (en) {
		DMSG("Config ACTLR.SMP = 1\n");
		write_actlr(read_actlr() | (0x40));
	} else {
		DMSG("Config ACTLR.SMP = 0\n");
		write_actlr(read_actlr() & ~0x40);
	}
#endif
}

#ifdef CFG_PL310
vaddr_t pl310_base(void)
{
        static void *va;

        if (cpu_mmu_enabled()) {
                if (!va)
                        va = phys_to_virt(PL310_BASE, MEM_AREA_IO_SEC, 0x1000);
                return (vaddr_t)va;
        }
        return PL310_BASE;
}

void arm_cl2_config(vaddr_t pl310_base)
{
        /* Disable PL310 */
        io_write32(pl310_base + PL310_CTRL, 0);

        io_write32(pl310_base + PL310_TAG_RAM_CTRL, PL310_TAG_RAM_CTRL_INIT);
        io_write32(pl310_base + PL310_DATA_RAM_CTRL, PL310_DATA_RAM_CTRL_INIT);
        io_write32(pl310_base + PL310_AUX_CTRL, PL310_AUX_CTRL_INIT);
        io_write32(pl310_base + PL310_PREFETCH_CTRL, PL310_PREFETCH_CTRL_INIT);
        io_write32(pl310_base + PL310_POWER_CTRL, PL310_POWER_CTRL_INIT);
        /* address filter */
        if ((CFG_TZDRAM_START + CFG_TZDRAM_SIZE) != CFG_SHMEM_START) {
            panic("teeos and nsmem memory address discontinuous.\n");
        }
        io_write32(pl310_base + PL310_ADDR_FILT_START, CFG_TZDRAM_START | 0x1);
        io_write32(pl310_base + PL310_ADDR_FILT_END, CFG_TZDRAM_START + CFG_TZDRAM_SIZE + CFG_SHMEM_SIZE);

        /* invalidate all cache ways */
        arm_cl2_invbyway(pl310_base);
}

void arm_cl2_enable(vaddr_t pl310_base)
{
        arm_cl2_invbyway(pl310_base);
        /* Disable PL310 ctrl -> only set lsb bit */
        io_write32(pl310_base + PL310_CTRL, 0);
        arm_cl2_invbyway(pl310_base);
        /* Enable PL310 ctrl -> only set lsb bit */
        io_write32(pl310_base + PL310_CTRL, 1);
}

/* Modify with val if mask bit is set 1 */
int arm_cl2_smc_cfg(vaddr_t pl310_base, unsigned int offset, unsigned int val, unsigned int mask)
{
        if (offset > PL310_POWER_CTRL) {
                EMSG("Offset error with 0x%08x\n", offset);
                return -1;
        }

        io_mask32(pl310_base + offset, val, mask);
        return 0;
}

void arm_cl2_disable(vaddr_t pl310_base)
{
        /* disable PL310 ctrl -> only set lsb bit */
        io_write32(pl310_base + PL310_CTRL, 0);
        arm_cl2_invbyway(pl310_base);
}

#endif

uint32_t nvt_smc_fast(struct thread_smc_args *args)
{
	int32_t ret = 0;

	switch (args->a0) {
	case NVT_GET_SMC_SW_FUNC_ID_VER:
		args->a0 = OPTEE_SMC_RETURN_OK;
		args->a1 = NVT_SMC_SW_FUNC_ID_VER;
		break;
	case NVT_GET_SMC_HW_FUNC_ID_VER:
		args->a0 = OPTEE_SMC_RETURN_OK;
		args->a1 = NVT_SMC_HW_FUNC_ID_VER;
		break;
#ifdef CFG_PL310
	case NVT_L2CACHE_CTRL:
		if (args->a1 == 0) {
			DMSG("disable l2\n");
			arm_cl2_disable(pl310_base());
		} else if (args->a1 & 0x1)  {
			DMSG("enable l2\n");
			arm_cl2_enable(pl310_base());
		}
		break;
	case NVT_L2CACHE_AUX_CTRL:
		ret = arm_cl2_smc_cfg(pl310_base(), PL310_AUX_CTRL, args->a1, args->a2);
		if (ret < 0)
			EMSG("Error config PL310 L2 cache\n");
		break;
	case NVT_L2CACHE_DBG_CTRL:
		ret = arm_cl2_smc_cfg(pl310_base(), PL310_DEBUG_CTRL, args->a1, args->a2);
		if (ret != 0)
			EMSG("Error config PL310 L2 cache\n");
		break;
	case NVT_L2CACHE_PREFETCH_CTRL:
		ret = arm_cl2_smc_cfg(pl310_base(), PL310_PREFETCH_CTRL, args->a1, args->a2);
		if (ret != 0)
			EMSG("Error config PL310 L2 cache\n");
		break;
	case NVT_L2CACHE_TAG_LATENCY_CTRL:
		ret = arm_cl2_smc_cfg(pl310_base(), PL310_TAG_RAM_CTRL, args->a1, args->a2);
		if (ret != 0)
			EMSG("Error config PL310 L2 cache tag latency: val:0x%x mask:0x%x\n", args->a1, args->a2);
		break;
	case NVT_L2CACHE_DATA_LATENCY_CTRL:
		ret = arm_cl2_smc_cfg(pl310_base(), PL310_DATA_RAM_CTRL, args->a1, args->a2);
		if (ret != 0)
			EMSG("Error config PL310 L2 cache data latency: val:0x%x mask:0x%x\n", args->a1, args->a2);
		break;
	case NVT_L2CACHE_PWR_CTRL:
		ret = arm_cl2_smc_cfg(pl310_base(), PL310_POWER_CTRL, args->a1, args->a2);
		if (ret != 0)
			EMSG("Error config PL310 L2 cache\n");
		break;
#endif /* CFG_PL310 */
	case NVT_ACTLR_SMP_CFG:
		if (args->a1 == 0) {
			DMSG("ACTLR.SMP = 0\n");
			arm_actlr_smp_cfg(0);
		} else {
			DMSG("ACTLR.SMP = 1\n");
			arm_actlr_smp_cfg(1);
		}
		break;

	case NVT_HELLO_WORD:
		smc_hello_world(args);
		break;

#ifdef CFG_NVT_CRYPTO
	case NVT_AES_OPERATION:
		args->a0 = nvt_ivot_optee_aes_operation((NVT_SMC_AES_DATA *)args->a1);
		break;
#endif

#ifdef CFG_NVT_HASH
	case NVT_SHA_OPERATION:
		args->a0 = nvt_ivot_optee_sha_operation((NVT_SMC_SHA_DATA *)args->a1);
		break;
#endif

#ifdef CFG_NVT_RSA
	case NVT_RSA_OPERATION:
		args->a0 = nvt_ivot_optee_rsa_operation((NVT_SMC_RSA_DATA *)args->a1);
		break;
#endif

#ifdef CFG_NVT_ECDSA
	case NVT_ECDSA_OPERATION:
		args->a0 = nvt_ivot_optee_ecdsa_operation((NVT_SMC_ECDSA_DATA *)args->a1);
		break;
#endif

#ifdef CFG_NVT_OPT
	case NVT_EFUSE_OPERATION:
		args->a0 = nvt_ivot_optee_efuse_operation((NVT_SMC_EFUSE_DATA *)args->a1);
		break;
#endif
	default:
		ret = OPTEE_SMC_RETURN_EBADCMD;
	}

	return ret;
}

uint32_t nvt_smc_std(struct optee_msg_arg *arg, uint32_t num_params __maybe_unused)
{
	uint32_t rv = OPTEE_SMC_RETURN_OK;

	switch (arg->cmd) {


	default:
		rv = OPTEE_SMC_RETURN_EBADCMD;
	}

	return rv;
}
