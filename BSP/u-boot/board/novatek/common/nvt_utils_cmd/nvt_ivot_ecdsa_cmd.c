/*
 * Copyright (c) 2021 Novatek Inc.
 *
 */

#include <stdlib.h>
#include <common.h>
#include <command.h>
#include <asm/arch/nvt_ecdsa.h>


static uint8_t priv[] = { 0xa4, 0x28, 0xae, 0x13, 0x4e, 0xdd, 0x48, 0x3c, 0x90, 0x64, 0x81, 0xae, 0x8c, 0x24, 0x82, 0x9d,
             			0x81, 0x9c, 0x3d, 0x24, 0x5a, 0x9c, 0xc6, 0x59, 0x25, 0xaa, 0x4a, 0x50, 0x1e, 0xcc, 0xfa, 0xf2};

static uint8_t pubx[] = {0x9f, 0x9a, 0x81, 0x95, 0x25, 0xf4, 0xd7, 0xdc, 0xd5, 0xc5, 0x23, 0x11, 0x17, 0x3e, 0x21, 0x27,
             			0x40, 0x81, 0x35, 0xd2, 0x85, 0xdd, 0xad, 0x7f, 0x0b, 0x1f, 0xc1, 0x75, 0xd9, 0xcf, 0xec, 0xd2};

static uint8_t puby[] = {0xb6, 0xee, 0x81, 0x35, 0x6c, 0x7e, 0x04, 0xbb, 0x38, 0xb0, 0x5a, 0xd0, 0x70, 0xef, 0xbc, 0xfd,
             			0x1e, 0xd1, 0x15, 0x75, 0x5e, 0x28, 0x3b, 0x73, 0x54, 0xa1, 0x77, 0xe5, 0xe8, 0x80, 0x46, 0xe3};

static uint8_t randk[] = {0x86, 0x2d, 0xf8, 0x12, 0x58, 0xf7, 0x04, 0x9b, 0xfc, 0xdd, 0x0b, 0x78, 0x30, 0x03, 0x82, 0xf8,
             			0x94, 0x6c, 0x4e, 0x98, 0x99, 0x2e, 0xfd, 0xb1, 0x7d, 0xe1, 0x4c, 0x98, 0xee, 0xfe, 0x7f, 0x71};

static uint8_t hash[] = {0x8c, 0x2d, 0xa5, 0xfe, 0x6c, 0x34, 0x9c, 0x75, 0x3f, 0xba, 0xe4, 0x78, 0x39, 0x9a, 0xbb, 0x59,
             			0xc9, 0xab, 0xc1, 0x8c, 0x14, 0x06, 0x9d, 0x9e, 0x0b, 0x16, 0x48, 0x38, 0x71, 0x2e, 0x46, 0xd9};

static uint8_t signr[] = {0x0b, 0xec, 0x9f, 0x7f, 0xa0, 0x97, 0x61, 0x87, 0xc5, 0xb6, 0xef, 0x62, 0xde, 0x99, 0x60, 0x07,
             			0x08, 0x30, 0x8f, 0xf8, 0xa5, 0xa1, 0x0f, 0x68, 0xc1, 0x66, 0xbb, 0xaf, 0xf3, 0x3f, 0x98, 0x28};

static uint8_t signs[] = {0x64, 0xb8, 0x32, 0x81, 0xd3, 0x53, 0xef, 0xfe, 0x71, 0x78, 0x8b, 0xe0, 0xdf, 0xc9, 0xb1, 0x92,
             			0x8e, 0xec, 0x07, 0x79, 0xdc, 0x54, 0xdf, 0x54, 0xc6, 0x4c, 0xc8, 0xc6, 0x75, 0x9e, 0x52, 0x46};

/**
 * do_nvt_ecdsa() - Handle the "nvt_ecdsa" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_nvt_ecdsa(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	int key_w;
	int offset;
	int dst_size;
	struct nvt_ecdsa_pio_t ecdsa = {0};

	/* Check argument */
	if (argc < 3)
		return CMD_RET_USAGE;

	/* Key Length */
	if (!strncmp(argv[1],      "ecdsa256", 8))
		key_w = NVT_ECDSA_KEY_WIDTH_256;
	else
		return CMD_RET_USAGE;

	/* Command */
	if (strncmp(argv[2], "verify", 6))
		return CMD_RET_USAGE;

	/* Open ECDSA Device */
	ret = nvt_ecdsa_open();
	if (ret < 0) {
		printf("nvt ecdsa open failed!\n");
		return CMD_RET_FAILURE;
	}

	/* ECDSA Encryption */
	switch (key_w) {
		case NVT_ECDSA_KEY_WIDTH_256:
			ecdsa.key_w = NVT_ECDSA_KEY_WIDTH_256;
			ecdsa.mode = NVT_ECDSA_SIGN;
			ecdsa.key_priv = priv;
			ecdsa.randk = randk;
			ecdsa.src = hash;
			ecdsa.sign_r = (uint8_t*)malloc(NVT_ECDSA_KEY_WIDTH_256 / 8 * sizeof(uint8_t));
			ecdsa.sign_s = (uint8_t*)malloc(NVT_ECDSA_KEY_WIDTH_256 / 8 * sizeof(uint8_t));
			break;
		default:
			ret = CMD_RET_FAILURE;
			goto exit;
	}
	ret = nvt_ecdsa_init(ecdsa.key_w);
	if (ret < 0) {
		printf("nvt ecdsa %s initial failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}
	ret = nvt_ecdsa_pio_normal(&ecdsa);
	if (ret < 0) {
		printf("nvt ecdsa %s encryption failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	/* Ciphertext Compare */
	switch (key_w) {
		case NVT_ECDSA_KEY_WIDTH_256:
			ret = memcmp(ecdsa.sign_r, signr, NVT_ECDSA_KEY_WIDTH_256/8);
			if (ret) {
				printf("nvt ecdsa %s encryption compare failed!\n", argv[1]);
				ret = CMD_RET_FAILURE;
				free(ecdsa.sign_r);
				free(ecdsa.sign_s);
				goto exit;
			}
			ret = memcmp(ecdsa.sign_s, signs, NVT_ECDSA_KEY_WIDTH_256/8);
			if (ret) {
				printf("nvt ecdsa %s encryption compare failed!\n", argv[1]);
				ret = CMD_RET_FAILURE;
				free(ecdsa.sign_r);
				free(ecdsa.sign_s);
				goto exit;
			}
			break;
		default:
			ret = CMD_RET_FAILURE;
			goto exit;
	}
	free(ecdsa.sign_r);
	free(ecdsa.sign_s);
	

	/* ECDSA Verify */
	switch (key_w) {
		// set hash + signr + signs + pubx + puby
		case NVT_ECDSA_KEY_WIDTH_256:
			ecdsa.key_w = NVT_ECDSA_KEY_WIDTH_256;
			ecdsa.mode = NVT_ECDSA_VERIFY;
			ecdsa.sign_r = signr;
			ecdsa.sign_s = signs;
			ecdsa.key_pub_x = pubx;
			ecdsa.key_pub_y = puby;
			ecdsa.src = hash;
			break;
		default:
			ret = CMD_RET_FAILURE;
			goto exit;
	}
	ret = nvt_ecdsa_pio_normal(&ecdsa);
	if (ret < 0) {
		printf("nvt ecdsa %s decryption failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	if (ecdsa.result) {
		printf("nvt ecdsa %s verify passed!\n", argv[1]);
	} else {
		printf("nvt ecdsa %s verify fail!\n", argv[1]);
	}
exit:
	/* Close ECDSA Device */
	nvt_ecdsa_close();

	return ret;
}

/**
 * do_nvt_ecdsa() - Handle the "nvt_ecdsa" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int perf_nvt_ecdsa(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret = 0;
	int key_w;
	int offset;
	int dst_size;
	struct nvt_ecdsa_pio_t ecdsa = {0};
	uint32_t start, end;

	/* Check argument */
	if (argc < 3)
		return CMD_RET_USAGE;

	/* Key Length */
	if (!strncmp(argv[1], "ecdsa256", 8))
		key_w = NVT_ECDSA_KEY_WIDTH_256;
	else
		return CMD_RET_USAGE;

	/* Command */
	if (strncmp(argv[2], "perf", 6))
		return CMD_RET_USAGE;

	/* Open ECDSA Device */
	ret = nvt_ecdsa_open();
	if (ret < 0) {
		printf("nvt ecdsa open failed!\n");
		return CMD_RET_FAILURE;
	}

	/* ECDSA Encryption */
	switch (key_w) {
		case NVT_ECDSA_KEY_WIDTH_256:
			ecdsa.key_w = NVT_ECDSA_KEY_WIDTH_256;
			ecdsa.mode = NVT_ECDSA_SIGN;
			ecdsa.key_priv = priv;
			ecdsa.randk = randk;
			ecdsa.src = hash;
			ecdsa.sign_r = (uint8_t*)malloc(NVT_ECDSA_KEY_WIDTH_256 / 8 * sizeof(uint8_t));
			ecdsa.sign_s = (uint8_t*)malloc(NVT_ECDSA_KEY_WIDTH_256 / 8 * sizeof(uint8_t));
			break;
		default:
			ret = CMD_RET_FAILURE;
			goto exit;
	}
	ret = nvt_ecdsa_init(ecdsa.key_w);
	if (ret < 0) {
		printf("nvt ecdsa %s initial failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}
	start = timer_get_us();
	ret = nvt_ecdsa_pio_normal(&ecdsa);
	end = timer_get_us();
	if (ret < 0) {
		printf("nvt ecdsa %s encryption failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	printf("nvt ecdsa encryption %d us \n", (end-start)); 

	/* Ciphertext Compare */
	switch (key_w) {
		case NVT_ECDSA_KEY_WIDTH_256:
			ret = memcmp(ecdsa.sign_r, signr, NVT_ECDSA_KEY_WIDTH_256/8);
			if (ret) {
				printf("nvt ecdsa %s encryption compare failed!\n", argv[1]);
				ret = CMD_RET_FAILURE;
				free(ecdsa.sign_r);
				free(ecdsa.sign_s);
				goto exit;
			}
			ret = memcmp(ecdsa.sign_s, signs, NVT_ECDSA_KEY_WIDTH_256/8);
			if (ret) {
				printf("nvt ecdsa %s encryption compare failed!\n", argv[1]);
				ret = CMD_RET_FAILURE;
				free(ecdsa.sign_r);
				free(ecdsa.sign_s);
				goto exit;
			}
			break;
		default:
			ret = CMD_RET_FAILURE;
			goto exit;
	}
	free(ecdsa.sign_r);
	free(ecdsa.sign_s);
	

	/* ECDSA Verify */
	switch (key_w) {
		// set hash + signr + signs + pubx + puby
		case NVT_ECDSA_KEY_WIDTH_256:
			ecdsa.key_w = NVT_ECDSA_KEY_WIDTH_256;
			ecdsa.mode = NVT_ECDSA_VERIFY;
			ecdsa.sign_r = signr;
			ecdsa.sign_s = signs;
			ecdsa.key_pub_x = pubx;
			ecdsa.key_pub_y = puby;
			ecdsa.src = hash;
			break;
		default:
			ret = CMD_RET_FAILURE;
			goto exit;
	}
	start = timer_get_us();
	ret = nvt_ecdsa_pio_normal(&ecdsa);
	end = timer_get_us();
	if (ret < 0) {
		printf("nvt ecdsa %s decryption failed!\n", argv[1]);
		ret = CMD_RET_FAILURE;
		goto exit;
	}

	if (ecdsa.result) {
		printf("nvt ecdsa %s verify passed!\n", argv[1]);
	} else {
		printf("nvt ecdsa %s verify fail!\n", argv[1]);
	}

	printf("nvt ecdsa verify %d us \n", (end-start)); 

exit:
	/* Close ECDSA Device */
	nvt_ecdsa_close();

	return ret;
}

/***************************************************/
U_BOOT_CMD(
	nvt_ecdsa, 3, 0, do_nvt_ecdsa,
	"nvt_ecdsa operation",
	"\nnvt_ecdsa ecdsa256 verify - for verify ecdsa256 encryption/decryption"
);

U_BOOT_CMD(
	nvt_ecdsa_perf, 3, 0, perf_nvt_ecdsa,
	"nvt_ecdsa operation",
	"\nnvt_ecdsa_perf ecdsa256 perf - for perf ecdsa256 encryption/decryption"
);
