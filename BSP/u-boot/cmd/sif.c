#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <config.h>
#include <string.h>
#include <sif_api.h>
#include <stdio.h>
#include <stdlib.h>

static ulong sifCh = SIF_CH0;
static ulong busMode = SIF_MODE_00;
static ulong busClk = 48000000;
static ulong dataLen = 32;
static ulong order = SIF_DIR_LSB;

static int do_sif_setChannel(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 2) {
		printf("do_sif_setChannel error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 2) {
		if (strcmp(argv[1], "0") == 0) {
			sifCh = SIF_CH0;
		} else if (strcmp(argv[1], "1") == 0) {
			sifCh = SIF_CH1;
		} else if (strcmp(argv[1], "2") == 0) {
			sifCh = SIF_CH2;
		} else if (strcmp(argv[1], "3") == 0) {
			sifCh = SIF_CH3;
		} else {
			printf("undefined SIF setting = %s\r\n", argv[1]);
			return CMD_RET_USAGE;
		}
	}

	return 0;
}


static int do_sif_getChannel(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 1) {
		printf("do_sif_getChannel error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 1) {
		printf("Current SIF CH = %ld\r\n", (ulong)sifCh);
	}

	return 0;
}


static int do_sif_setMode(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 2) {
		printf("do_sif_setMode error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 2) {
		if (strcmp(argv[1], "0") == 0) {
			busMode = SIF_MODE_00;
		} else if (strcmp(argv[1], "1") == 0) {
			busMode = SIF_MODE_01;
		} else if (strcmp(argv[1], "2") == 0) {
			busMode = SIF_MODE_02;
		} else if (strcmp(argv[1], "3") == 0) {
			busMode = SIF_MODE_03;
		} else if (strcmp(argv[1], "4") == 0) {
			busMode = SIF_MODE_04;
		} else if (strcmp(argv[1], "5") == 0) {
			busMode = SIF_MODE_05;
		} else if (strcmp(argv[1], "6") == 0) {
			busMode = SIF_MODE_06;
		} else if (strcmp(argv[1], "7") == 0) {
			busMode = SIF_MODE_07;
		} else if (strcmp(argv[1], "8") == 0) {
			busMode = SIF_MODE_08;
		} else if (strcmp(argv[1], "9") == 0) {
			busMode = SIF_MODE_09;
		} else if (strcmp(argv[1], "10") == 0) {
			busMode = SIF_MODE_10;
		} else if (strcmp(argv[1], "11") == 0) {
			busMode = SIF_MODE_11;
		} else if (strcmp(argv[1], "12") == 0) {
			busMode = SIF_MODE_12;
		} else if (strcmp(argv[1], "13") == 0) {
			busMode = SIF_MODE_13;
		} else if (strcmp(argv[1], "14") == 0) {
			busMode = SIF_MODE_14;
		} else if (strcmp(argv[1], "15") == 0) {
			busMode = SIF_MODE_15;
		} else {
			printf("undefined setting = %s\r\n", argv[1]);
			return CMD_RET_USAGE;
		}
	}

	return 0;
}

static int do_sif_getMode(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 1) {
		printf("do_sif_getMode error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 1) {
		printf("Current SIF Mode = %ld\r\n", (ulong)busMode);
	}

	return 0;
}


static int do_sif_setBusClk(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong temp;
	if (argc != 2) {
		printf("do_sif_setBusClk error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 2) {
		temp = (ulong)atoi(argv[1]);
		if (((ulong)temp >= (ulong)187500) && ((ulong)temp <= (ulong)48000000)) {
			busClk = (ulong)temp;
			printf("Current SIF clock = %ld\r\n", (ulong)busClk);
		} else {
			printf("undefined setting = %ld\r\n", (ulong)atoi(argv[1]));
			return CMD_RET_USAGE;
		}
	}

	return 0;
}


static int do_sif_getBusClk(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 1) {
		printf("do_sif_getBusClk error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 1) {
		printf("Current SIF clock = %ld\r\n", (ulong)busClk);
	}

	return 0;
}


static int do_sif_setDataBit(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong temp;
	if (argc != 2) {
		printf("do_sif_setDataBit error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 2) {
		temp = (ulong)atoi(argv[1]);
		if (((ulong)temp >= (ulong)1) && ((ulong)temp <= (ulong)96)) {
			dataLen = (ulong)temp;
			printf("Current SIF data Len = %ld\r\n", (ulong)dataLen);
		} else {
			printf("undefined setting = %ld\r\n", (ulong)atoi(argv[1]));
			return CMD_RET_USAGE;
		}
	}

	return 0;
}


static int do_sif_getDataBit(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 1) {
		printf("do_sif_getDataBit error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 1) {
		printf("Current SIF data Len = %ld\r\n", (ulong)dataLen);
	}

	return 0;
}


static int do_sif_setOrder(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 2) {
		printf("do_sif_setOrder error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 2) {
		if (((UINT32)atoi(argv[1]) < (UINT32)2)) {
			order = (UINT32)atoi(argv[1]);
			printf("Current SIF data order = %s\r\n", (order == 0) ? "LSB":"MSB");
		} else {
			printf("undefined setting = %s\r\n", argv[1]);
			return CMD_RET_USAGE;
		}
	}

	return 0;
}

static int do_sif_getOrder(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 1) {
		printf("do_sif_getOrder error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 1) {
		printf("Current SIF data order = %s\r\n", (order == 0) ? "LSB":"MSB");
	}

	return 0;
}

static int do_sif_send(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong dr0, dr1, dr2;

	if (argc != 4) {
		printf("do_sif_send error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 4) {
		printf("Select CH = 0x%ld\r\n", (ulong)sifCh);
		sif_open(sifCh);
 
 		dr0 = atoi(argv[1]);
		dr1 = atoi(argv[2]);
		dr2 = atoi(argv[3]);
		printf("dr0 = 0x%s, dr1 = 0x%s,  dr2 = 0x%s\r\n", argv[1], argv[2], argv[3]);

		sif_setConfig(sifCh, SIF_CONFIG_ID_BUSCLOCK,    busClk);
		sif_setConfig(sifCh, SIF_CONFIG_ID_BUSMODE,    busMode);
		sif_setConfig(sifCh, SIF_CONFIG_ID_LENGTH,     dataLen);
		sif_setConfig(sifCh, SIF_CONFIG_ID_DIR,          order);
		sif_setConfig(sifCh, SIF_CONFIG_ID_SEND,             0);
		sif_setConfig(sifCh, SIF_CONFIG_ID_SENS,             4);
		sif_setConfig(sifCh, SIF_CONFIG_ID_SENH,             4);
		sif_setConfig(sifCh, SIF_CONFIG_ID_DELAY,            0);

		sif_send(sifCh, dr0, dr1, dr2);
		sif_waitTransferEnd(sifCh);

		sif_close(sifCh);
	}

	return 0;
}

static int do_sif_dump(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc != 1) {
		printf("do_sif_dump error: not correct argc numbers\r\n");
		return CMD_RET_USAGE;
	}

	if (argc == 1) {
		printf("Select CH = %ld\r\n", (ulong)sifCh);
		printf("Select bus mode = %ld\r\n", (ulong)busMode);
		printf("Select bus clock = %ld\r\n", (ulong)busClk);
		printf("Select data len = %ld\r\n", (ulong)dataLen);
		printf("Select order = %s\r\n", (order == 0) ? "LSB":"MSB");
	}

	return 0;
}

static struct cmd_tbl cmd_sif_sub[] = {
	U_BOOT_CMD_MKENT(dump,       2, 1, do_sif_dump,           "", ""),
	U_BOOT_CMD_MKENT(setch,      3, 1, do_sif_setChannel,     "", ""),
	U_BOOT_CMD_MKENT(getch,      2, 1, do_sif_getChannel,     "", ""),
	U_BOOT_CMD_MKENT(setmode,    3, 1, do_sif_setMode,        "", ""),
	U_BOOT_CMD_MKENT(getmode,    2, 1, do_sif_getMode,        "", ""),
	U_BOOT_CMD_MKENT(setbusclk,  3, 1, do_sif_setBusClk,      "", ""),
	U_BOOT_CMD_MKENT(getbusclk,  2, 1, do_sif_getBusClk,      "", ""),
	U_BOOT_CMD_MKENT(setdatabit, 3, 1, do_sif_setDataBit,     "", ""),
	U_BOOT_CMD_MKENT(getdatabit, 2, 1, do_sif_getDataBit,     "", ""),
	U_BOOT_CMD_MKENT(setorder,   3, 1, do_sif_setOrder,       "", ""),
	U_BOOT_CMD_MKENT(getorder,   2, 1, do_sif_getOrder,       "", ""),
	U_BOOT_CMD_MKENT(send,       4, 1, do_sif_send,           "", ""),
};


/**
 * do_sif() - Handle the "SIF" command-line command
 * @cmdtp:	Command data struct pointer
 * @flag:	Command flag
 * @argc:	Command-line argument count
 * @argv:	Array of command-line arguments
 *
 * Returns zero on success, CMD_RET_USAGE in case of misuse and negative
 * on error.
 */
static int do_sif(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2) {
		return CMD_RET_USAGE;
	}
		
	/* Strip off leading 'sif' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_sif_sub[0], ARRAY_SIZE(cmd_sif_sub));

	if (c) {
		return c->cmd(cmdtp, flag, argc, argv);
	} else {
		return CMD_RET_USAGE;
	}
}


/***************************************************/
static char sif_help_text[] =
	"\n======================================================================\n"
	"SIF info cmd\n"
	"dump                            --- show current setting\n"
	"======================================================================\n"
	"SIF configs cmd(must call before send transfer cmd)\n"
	"setch       [ch]                --- set sif channel(0~3)\n"
	"getch                           --- get sif channel\n"
	"setmode     [mode]              --- set sif mode(0~15)\n"
	"getmode                         --- get sif mode\n"
	"setbusclk   [clk]               --- set sif bus clock(~48000000)\n"
	"getbusclk                       --- print current sif bus clock\n"
	"setdatabit  [size]              --- set sif transfer length(1~96)bits\n"
	"getdatabit                      --- get sif transfer length\n"
	"setorder    [order]             --- set sif order(0: LSB, 1: MSB)\n"
	"setorder                        --- get sif order\n"
	"======================================================================\n"
	"SIF transfer cmd\n"
	"send [data0] [data1] [data2]    --- pio write\n"
	"======================================================================\n";

U_BOOT_CMD(
	sif,             //name
	7,               //max_arg
	1,               //rep 
	do_sif,          //cmd
	"SIF utility",   //usage
	sif_help_text    //help
);
