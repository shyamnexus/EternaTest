/**
 * @file audio_caps_check.c
 * @brief Check audio encoder capabilities on the device
 */

#include <stdio.h>
#include "hdal.h"
#include "hd_debug.h"

static HD_RESULT mem_init(void)
{
    HD_RESULT ret = HD_OK;
    HD_COMMON_MEM_INIT_CONFIG mem_cfg = {0};

    // config common pool (main)
    mem_cfg.pool_info[0].type = HD_COMMON_MEM_COMMON_POOL;
    mem_cfg.pool_info[0].blk_size = 0x1000;
    mem_cfg.pool_info[0].blk_cnt = 1;
    mem_cfg.pool_info[0].ddr_id = DDR_ID0;

    ret = hd_common_mem_init(&mem_cfg);
    return ret;
}

static HD_RESULT mem_exit(void)
{
    HD_RESULT ret = HD_OK;
    ret = hd_common_mem_uninit();
    return ret;
}

int main(void) {
    HD_RESULT ret;
    HD_AUDIOENC_SYSCAPS syscaps = {0};
    HD_PATH_ID ctrl_path = 0;
    
    printf("\n=== Audio Encoder Capability Check ===\n\n");
    
    // Initialize HDAL
    ret = hd_common_init(0);
    if (ret != HD_OK) {
        printf("ERROR: hd_common_init failed: %d\n", ret);
        return -1;
    }
    
    // Initialize memory pool (REQUIRED!)
    ret = mem_init();
    if (ret != HD_OK) {
        printf("ERROR: mem_init failed: %d\n", ret);
        hd_common_uninit();
        return -1;
    }
    
    // Initialize audiocap first (required before audioenc)
    ret = hd_audiocap_init();
    if (ret != HD_OK) {
        printf("ERROR: hd_audiocap_init failed: %d\n", ret);
        mem_exit();
        hd_common_uninit();
        return -1;
    }
    
    ret = hd_audioenc_init();
    if (ret != HD_OK) {
        printf("ERROR: hd_audioenc_init failed: %d\n", ret);
        hd_audiocap_uninit();
        mem_exit();
        hd_common_uninit();
        return -1;
    }
    
    // Open control path
    ret = hd_audioenc_open(0, HD_AUDIOENC_0_CTRL, &ctrl_path);
    if (ret != HD_OK) {
        printf("ERROR: hd_audioenc_open ctrl failed: %d\n", ret);
        goto exit;
    }
    
    // Query system capabilities
    ret = hd_audioenc_get(ctrl_path, HD_AUDIOENC_PARAM_SYSCAPS, &syscaps);
    if (ret != HD_OK) {
        printf("ERROR: hd_audioenc_get SYSCAPS failed: %d\n", ret);
        goto exit;
    }
    
    printf("Device ID: 0x%x\n", syscaps.dev_id);
    printf("Chip ID: 0x%x\n", syscaps.chip_id);
    printf("Max Inputs: %u\n", syscaps.max_in_count);
    printf("Max Outputs: %u\n", syscaps.max_out_count);
    printf("\nOutput Capabilities:\n");
    
    for (int i = 0; i < syscaps.max_out_count && i < HD_AUDIOENC_MAX_OUT; i++) {
        printf("  Output[%d]: 0x%08x\n", i, syscaps.out_caps[i]);
        
        if (syscaps.out_caps[i] & HD_AUDIOENC_CAPS_PCM) {
            printf("    ✓ PCM supported\n");
        }
        if (syscaps.out_caps[i] & HD_AUDIOENC_CAPS_AAC) {
            printf("    ✓ AAC supported\n");
        }
        if (syscaps.out_caps[i] & HD_AUDIOENC_CAPS_ADPCM) {
            printf("    ✓ ADPCM supported\n");
        }
        if (syscaps.out_caps[i] & HD_AUDIOENC_CAPS_ULAW) {
            printf("    ✓ G.711 μ-law supported\n");
        }
        if (syscaps.out_caps[i] & HD_AUDIOENC_CAPS_ALAW) {
            printf("    ✓ G.711 A-law supported\n");
        }
        
        if (syscaps.out_caps[i] == 0) {
            printf("    (No codecs supported)\n");
        }
    }
    
    printf("\n");
    
exit:
    if (ctrl_path) hd_audioenc_close(ctrl_path);
    hd_audioenc_uninit();
    hd_audiocap_uninit();
    mem_exit();
    hd_common_uninit();
    
    return 0;
}
