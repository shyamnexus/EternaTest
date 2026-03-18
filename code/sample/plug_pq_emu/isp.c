#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <kwrap/file.h>

#include "hd_type.h"
#include "vendor_common.h"
#include "vendor_isp.h"
#include "aiisp_pq.h"
#include "ai.h"
#include "isp.h"

///////////////////////////////////////////////////////////////////////////////
#define AIISP_JSON_NAME "aiisp_pq_param_os04a10"
#define ISP_FPN_NAME    "isp_os04a10_0_iso_204800"
#define SOCKET_ADDR     "192.168.0.3"

CHAR aiisp_json_name[64] = AIISP_JSON_NAME;
CHAR isp_fpn_name[64] = ISP_FPN_NAME;

static BOOL print_aiisp_param_en = FALSE;
static UINT32 init_total_gain = 100;

#if (AIISP_PARAM_SYNC)
static UINT32 iso_map_tab[AIISP_GAIN_MAX][2] = {
	{    100,      0},
	{    200,      1},
	{    400,      2},
	{    800,      3},
	{   1600,      4},
	{   3200,      5},
	{   6400,      6},
	{  12800,      7},
	{  25600,      8},
	{  51200,      9},
	{ 102400,     10},
	{ 204800,     11},
	{ 409600,     12},
	{ 819200,     13},
	{1638400,     14},
	{3276800,     15}
};

static AIISP_PQ_FINAL_PARAM aiisp_pq_final_param = {
	.reserved_common = 255,
	._2dnr_still_str = 128,
	.reserved_1 = 128,
	.reserved_2 = 128,
	.reserved_3 = 128,
	.coef_a = 128,
	.coef_b = 128
};
#endif

#if (AIISP_LOAD_JSON | AIISP_PARAM_SYNC | AIISP_PARAM_SOCKET)
static AIISP_PQ_PARAM aiisp_pq_param = {
	.version = 2,
	.mode = 0,
	.reserved_common = 0,
	._2dnr_still_str_manual = 128,
	.reserved_1_manual = 128,
	.reserved_2_manual = 128,
	.reserved_3_manual = 128,
	.coef_a_manual = 0,
	.coef_b_manual = 0,
	.coef_a_min = 0,
	.coef_a_max = 65535,
	.coef_b_min = 0,
	.coef_b_max = 65535
};
#endif

#if (AIISP_PARAM_SOCKET)
static UINT32 net_proc_exit;
pthread_t socket_thread_id;
#endif

#if (ISP_FPN_ENABLE)
ISP_DDR_INFO fpn_buffer      = {0};
#endif

//=============================================================================
// internal functions
//=============================================================================
#if (AIISP_PARAM_SYNC)
static INT32 isp_intpl(INT32 index, INT32 l_value, INT32 h_value, INT32 l_index, INT32 h_index)
{
	INT32 range = h_index - l_index;

	if (l_value == h_value) {
		return l_value;
	} else if (index <= l_index) {
		return l_value;
	} else if (index >= h_index) {
		return h_value;
	}
	if (h_value < l_value) {
		return l_value + ((h_value - l_value) * (index  - l_index) - (range >> 1)) / range;
	} else {
		return l_value + ((h_value - l_value) * (index  - l_index) + (range >> 1)) / range;
	}
}

static void isp_intpl_aiisp_param(UINT32 cur_gain, AIISP_PQ_PARAM *param, AIISP_PQ_FINAL_PARAM *final_param)
{
	UINT32 iso_idx_l = 0, iso_idx_h = 0;
	UINT32 iso_start = 0, iso_end = 0;
	UINT32 i;

	final_param->reserved_common = param->reserved_common;
	if (param->mode) {
		// NOTE: Manual
		final_param->_2dnr_still_str = param->_2dnr_still_str_manual;
		final_param->reserved_1 = param->reserved_1_manual;
		final_param->reserved_2 = param->reserved_2_manual;
		final_param->reserved_3 = param->reserved_3_manual;
		final_param->coef_a = param->coef_a_manual;
		final_param->coef_b = param->coef_b_manual;
	} else {
		// NOTE: Auto
		if (cur_gain >= iso_map_tab[AIISP_GAIN_MAX - 1][0]) {
			iso_idx_l = iso_map_tab[AIISP_GAIN_MAX - 1][1];
			iso_idx_h = iso_map_tab[AIISP_GAIN_MAX - 1][1];
			iso_start = iso_map_tab[AIISP_GAIN_MAX - 1][0];
			iso_end = iso_map_tab[AIISP_GAIN_MAX - 1][0];
		}
		for (i = 1; i < AIISP_GAIN_MAX; i++) {
			if (cur_gain < iso_map_tab[i][0]) {
				iso_idx_l = iso_map_tab[i - 1][1];
				iso_idx_h = iso_map_tab[i][1];
				iso_start = iso_map_tab[i - 1][0];
				iso_end = iso_map_tab[i][0];
				break;
			}
		}

		final_param->_2dnr_still_str = (UINT8)isp_intpl(cur_gain, param->_2dnr_still_str_auto[iso_idx_l], param->_2dnr_still_str_auto[iso_idx_h], iso_start, iso_end);
		final_param->reserved_1 = (UINT8)isp_intpl(cur_gain, param->reserved_1_auto[iso_idx_l], param->reserved_1_auto[iso_idx_h], iso_start, iso_end);
		final_param->reserved_2 = (UINT8)isp_intpl(cur_gain, param->reserved_2_auto[iso_idx_l], param->reserved_2_auto[iso_idx_h], iso_start, iso_end);
		final_param->reserved_3 = (UINT8)isp_intpl(cur_gain, param->reserved_3_auto[iso_idx_l], param->reserved_3_auto[iso_idx_h], iso_start, iso_end);
		final_param->coef_a = (UINT32)isp_intpl(cur_gain, param->coef_a_auto[iso_idx_l], param->coef_a_auto[iso_idx_h], iso_start, iso_end);
		final_param->coef_b = (UINT32)isp_intpl(cur_gain, param->coef_b_auto[iso_idx_l], param->coef_b_auto[iso_idx_h], iso_start, iso_end);
	}

	if (print_aiisp_param_en) {
		printf("\nFinal Parameters: \n");
		printf("reserved_common =         %d \n", final_param->reserved_common);
		printf("2dnr still =              %d \n", final_param->_2dnr_still_str);
		printf("reserved_1 =              %d \n", final_param->reserved_1);
		printf("reserved_2 =              %d \n", final_param->reserved_2);
		printf("reserved_1 =              %d \n", final_param->reserved_3);
		printf("coef_a =                  %d \n", final_param->coef_a);
		printf("coef_b =                  %d \n", final_param->coef_b);
	}
}
#endif

#if (AIISP_PARAM_SOCKET)
static int demon_client_recv(int socket_recv, void *p_data, unsigned int n_byte)
{
	int n, n_recv = 0, n_retry = 0;
	struct timeval tv;
	fd_set master, readfds;

	//printf("recv %d bytes.\n", n_byte);

	FD_ZERO(&master);
	FD_ZERO(&readfds);
	FD_SET(socket_recv, &master);

	while (n_recv != (int)n_byte) {
		readfds = master;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		int ret = select(socket_recv + 1, &readfds, NULL, NULL, &tv);

		switch (ret) {
		case (-1):
			printf("[AI] %s: SOCKET_ERROR\n", __FUNCTION__);
			FD_CLR(socket_recv, &master);

			return -1;
		case 0:    // timeout
			printf("[AI] socket time out!\n");
			FD_CLR(socket_recv, &master);

			return -1;
		default:
			if (FD_ISSET(socket_recv, &readfds)) {
				if ((n = recv(socket_recv, (char *)p_data + n_recv, n_byte - n_recv, 0)) == 0) {
					printf("[AI] socket close(%d)\n", socket_recv);
					FD_CLR(socket_recv, &master);

					return -1;
				} else if (n < 0) {
					if (n_retry++ > 3) {
						printf("[AI] socket fail to recv retry!\n");
						FD_CLR(socket_recv, &master);

						return -1;
					}
				} else {
					n_recv += n;
				}
			}
			break;
		}
	}

	FD_CLR(socket_recv, &master);

	return (n_recv == (int)n_byte) ? 0 : -1;
}

static int demon_client_send(int socket_send, void *p_data, unsigned int n_byte)
{
	int n, n_send = 0, n_retry = 0;
	struct timeval tv;
	fd_set master, writefds;
	int ret = 0;

	//printf("send %d bytes.\n", n_byte);

	FD_ZERO(&master);
	FD_ZERO(&writefds);
	FD_SET(socket_send, &master);

	while (n_send != (int)n_byte) {
		writefds = master;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		ret = select(socket_send + 1, NULL, &writefds, NULL, &tv);

		switch (ret) {
		case (-1):
			printf("[AI] %s: SOCKET_ERROR\n", __FUNCTION__);
			FD_CLR(socket_send, &master);

			return -1;
		case 0:    // timeout
			printf("[AI] socket time out!\n");
			FD_CLR(socket_send, &master);

			return -1;
		default:
			if (FD_ISSET(socket_send, &writefds)) {
				if ((n = send(socket_send, (char *)p_data + n_send, n_byte - n_send, 0)) == 0) {
					printf("[AI] socket send fail!\n");
					FD_CLR(socket_send, &master);

					return -1;
				} else if (n < 0) {
					if (n_retry++ > 3) {
						printf("[AI] socket fail to send retry!\n");
						FD_CLR(socket_send, &master);

						return -1;
					}
				} else {
					n_retry = 0;
					n_send += n;
				}
			}
			break;
		}
	}

	FD_CLR(socket_send, &master);

	return (n_send == (int)n_byte) ? 0 : -1;
}

UINT32 ai_process_cmd(UINT32 cmd_type, UINT32 input_cmd, UINT32 *data)
{
	UINT32 cmd_state = ISPD_CMD_PASS;
	UINT32 i;

	if (READ_AIISP_PQ_PARAM == input_cmd) {
		memcpy(data, &aiisp_pq_param, sizeof(AIISP_PQ_PARAM));
	} else if (WRITE_AIISP_PQ_PARAM == input_cmd ) {
		memcpy(&aiisp_pq_param, data, sizeof(AIISP_PQ_PARAM));

		if (print_aiisp_param_en) {
			printf("\nServer get: \n");
			printf("version             %3d \n", aiisp_pq_param.version);
			printf("mode                %3d \n", aiisp_pq_param.mode);
			printf("reserved_common:    %3d \n", aiisp_pq_param.reserved_common);

			printf("2dnr still:         %3d, ", aiisp_pq_param._2dnr_still_str_manual);
			for (i = 0 ; i < 16 ; i++) {
				printf("%3d ", aiisp_pq_param._2dnr_still_str_auto[i]);
			}
			printf("\n");

			printf("reserved_1:         %3d, ", aiisp_pq_param.reserved_1_manual);
			for (i = 0 ; i < 16 ; i++) {
				printf("%3d ", aiisp_pq_param.reserved_1_auto[i]);
			}
			printf("\n");

			printf("reserved_2:         %3d, ", aiisp_pq_param.reserved_2_manual);
			for (i = 0 ; i < 16 ; i++) {
				printf("%3d ", aiisp_pq_param.reserved_2_auto[i]);
			}
			printf("\n");

			printf("reserved_3:         %3d, ", aiisp_pq_param.reserved_3_manual);
			for (i = 0 ; i < 16 ; i++) {
				printf("%3d ", aiisp_pq_param.reserved_3_auto[i]);
			}
			printf("\n");

			printf("coef a:             %3d, ", aiisp_pq_param.coef_a_manual);
			for (i = 0 ; i < 16 ; i++) {
				printf("%3d ", aiisp_pq_param.coef_a_auto[i]);
			}
			printf("\n");

			printf("coef b:             %3d, ", aiisp_pq_param.coef_b_manual);
			for (i = 0 ; i < 16 ; i++) {
				printf("%3d ", aiisp_pq_param.coef_b_auto[i]);
			}
			printf("\n");

			printf("coef_a range :      [%d, %d] \n", aiisp_pq_param.coef_a_min, aiisp_pq_param.coef_a_max);
			printf("coef_b range :      [%d, %d] \n", aiisp_pq_param.coef_b_min, aiisp_pq_param.coef_b_max);
		}
	} else {
		cmd_state = ISPD_CMD_NONE;
	}

	return cmd_state;
}

static void *server_thread(void *arg)
{
	char* buf_addr;
	unsigned int cmd = 0;
	int recv_enable = 1;
	int send_enable = 0;
	ISPD_CBW CBW = { 0 };
	ISPD_CSW CSW = { CSW_SIGNATURE, 0, 0 ,0};
	int sockfd = 0,forClientSockfd = 0;
	struct sockaddr clientInfo = {0};
	socklen_t addrlen = sizeof(clientInfo);
    struct addrinfo *res = NULL;
	struct addrinfo server_address = {0};
    int port_num = 8700;
    int err = 0;
	fd_set readfds;
	struct timeval tv;
	BOOL connect = FALSE;
    
    memset(&server_address, 0, sizeof(server_address));
	server_address.ai_family = AF_INET;
	server_address.ai_socktype = SOCK_STREAM;
	server_address.ai_flags = AI_PASSIVE;
    
    char port_num_str[16];
	sprintf(port_num_str, "%u", port_num);
    
    // Auto Detection IP (192.168.0.3)
    err = getaddrinfo(NULL, port_num_str, &server_address, &res);
    if (err!=0 || res == NULL) {
		printf("getaddrinfo: %d\n", err);
		return NULL;
	}
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0){
		printf("Fail to create a socket.");
        freeaddrinfo(res);
		return NULL;
	}
    
    // setting SO_REUSEADDR
	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == -1) {
		printf("Fail to set SO_REUSEADDR option.");
        freeaddrinfo(res);
		close(sockfd);
		return NULL;
	}
    
    printf("[AIISPD] bind %d\n", port_num);
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
		printf("Fail to bind the socket.");
        freeaddrinfo(res);
		close(sockfd);
		return NULL;
	}
    
    printf("[AIISPD] listen ...\n");
	if (listen(sockfd, 5) == -1) {
		printf("Fail to listen on the socket.");
        freeaddrinfo(res);
		close(sockfd);
		return NULL;
	}

    freeaddrinfo(res);
	printf("Server is running... \n");
	while (net_proc_exit == 0) {

		FD_ZERO(&readfds);
		FD_SET(sockfd,&readfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		switch (select(sockfd+1, &readfds, NULL, NULL, &tv)) {
		case -1:
			break;

		case 0:  /* timeout */
			break;

		default:
			connect = TRUE;
			break;

		}

		if (connect) {
			forClientSockfd = accept(sockfd, &clientInfo, &addrlen);
			printf("[AI] Socket open(%d) \n", forClientSockfd);

			//ispq app connects to this server, ai information can be exchanged throught send()/recv()
			while(net_proc_exit == 0 && connect) {

				FD_ZERO(&readfds);
				FD_SET(forClientSockfd,&readfds);

				tv.tv_sec = 1;
				tv.tv_usec = 0;

				switch (select(forClientSockfd+1, &readfds, NULL, NULL, &tv)) {
				case -1:
					break;

				case 0:  /* timeout */
					break;

				default:
					recv_enable = 1;
					send_enable = 0;
					buf_addr = (char*)malloc(sizeof(AIISP_PQ_PARAM));
					if (demon_client_recv(forClientSockfd, &CBW, sizeof(ISPD_CBW)) != 0) {
						//printf("[AI] socket demon_client_recv err!\n");
						printf("[AI] Socket off \n");
						connect = 0;
						if (forClientSockfd) {
							close(forClientSockfd);
							forClientSockfd = 0;
						}
						break;
					} else if (CBW.dSignature != CBW_SIGNATURE) {
						printf("[AI] Socket client CBW_SIGNATURE Error \n");
						break;
					}

					CSW.dCSWSignature = CSW_SIGNATURE;
					CSW.dCSWTag = CBW.dTag;
					CSW.bCSWStatus = ISPD_CMD_PASS;
					CSW.dCSWDataResidue = 0;
					cmd = (CBW.CBWCB[3] << 24)|(CBW.CBWCB[2] << 16)|(CBW.CBWCB[1] << 8)|CBW.CBWCB[0];
					if ((CBW.bmFlags & CBW_TYPE_READ) > 0) {
						send_enable = 1;
						if ((CBW.bmFlags & CBW_TYPE_READ_ARG) == 0) {
							recv_enable = 0;
						}
					}

					{
						unsigned char bmFlags = CBW.bmFlags;
						UINT32 cmd_state = ISPD_CMD_PASS;
						if (recv_enable) {
							if (demon_client_recv(forClientSockfd, buf_addr, CBW.dDataTransferLength) < 0) {
								printf("[AI] %s : Fail to recieve parameters from client! size = %d \n", __FUNCTION__, CBW.dDataTransferLength);
								break;
							}
						}

						cmd_state = ai_process_cmd(bmFlags, cmd, (UINT32 *)buf_addr);
						CSW.bCSWStatus = cmd_state;
						if (CSW.bCSWStatus == ISPD_CMD_FAIL) {
							printf("[AI] Error from %s : CSW = %d %d %d, CMD=0x%.8x \n", __FUNCTION__, CSW.dCSWTag, CSW.dCSWDataResidue,CSW.bCSWStatus, cmd);
							break;
						}

						if (send_enable) {
							if (demon_client_send(forClientSockfd, (unsigned char *)buf_addr,CBW.dDataTransferLength) < 0) {
								printf("[AI] %s : Socket fail to send parameters to clent \n", __FUNCTION__);
								break;
							}
						}

						if ((bmFlags != CBW_TYPE_COMBO_CMD) &&(demon_client_send(forClientSockfd, (unsigned char *)&CSW, sizeof(ISPD_CSW)) < 0)) {
							printf("[AI] Socket fail to send parameters to clent \n");
							break;
						}

						#if (AIISP_PARAM_SYNC)
						isp_intpl_aiisp_param(init_total_gain, &aiisp_pq_param, &aiisp_pq_final_param);
						ai_set_param(&aiisp_pq_final_param);
						ai_model_trig();
						#endif
					}

					free(buf_addr);
					break;
				}
			}
		}
	}

	if (forClientSockfd) {
		close(forClientSockfd);
	}

	if (sockfd) {
		close(sockfd);
	}

	return NULL;
}
#endif

#if (ISP_FPN_ENABLE)
static void isp_fpn_set(UINT32 isp_id) {
	VENDOR_COMM_MAX_FREE_BLOCK max_free_block = {0};
	VOS_FILE fp;
	struct vos_stat stat;
	UINT32 buffer_size;
	IQT_FPN_PARAM fpn = {0};
	HD_RESULT file_ret, ret, fpn_ret = HD_OK;
	CHAR fpn_raw[128];

	snprintf(fpn_raw, 128, "/mnt/sd/%s.fpn", isp_fpn_name);

	// create buffer for FPN_RAW
	// read from SD
	file_ret = vos_file_stat((CHAR *)&fpn_raw, &stat);
	if (file_ret == 0) {
		buffer_size = stat.st_size;
		printf("get stat of %s OK, buffer_size = %d \n", &fpn_raw, stat.st_size);
	} else {
		buffer_size = 0;
		printf("get %s fail \n", &fpn_raw);
		fpn_ret |= HD_ERR_FAIL;
	}

	// allocate buffer
	max_free_block.ddr = 0;
	vendor_common_mem_get(VENDOR_COMMON_MEM_ITEM_MAX_FREE_BLOCK_SIZE, &max_free_block);
	printf("DDR0 max_free_block size = %d \n", max_free_block.size);
	if (max_free_block.size < buffer_size) {
		printf("DDR0 max_free_block size is too small \n");
		fpn_ret |= HD_ERR_FAIL;
	};
	ret = hd_common_mem_alloc("FPN", &fpn_buffer.pa, (void **)&fpn_buffer.va, buffer_size, 0);
	if (ret != HD_OK) {
		printf("memory allocate size %d NG \n", buffer_size);
		fpn_ret |= HD_ERR_FAIL;
	};

	// write to DRAM
	if (file_ret == HD_OK) {
		fp = vos_file_open((CHAR *)&fpn_raw, O_RDONLY, 0);
		if (fp == VOS_FILE_INVALID) {
			printf("open %s fail \n", &fpn_raw);
			fpn_ret |= HD_ERR_FAIL;
		}
		vos_file_read(fp, (UINT32 *)fpn_buffer.va, stat.st_size);
		vos_file_close(fp);
	}

	if (fpn_ret == HD_OK) {
		fpn.id = isp_id;
		fpn.fpn.enable = TRUE;
		fpn.fpn.mode = IQ_OP_TYPE_AUTO;
		fpn.fpn.auto_param[15].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[15].buf_size = buffer_size;
		fpn.fpn.auto_param[15].gain = 256;
		fpn.fpn.auto_param[14].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[14].buf_size = buffer_size;
		fpn.fpn.auto_param[14].gain = 256;
		fpn.fpn.auto_param[13].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[13].buf_size = buffer_size;
		fpn.fpn.auto_param[13].gain = 256;
		fpn.fpn.auto_param[12].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[12].buf_size = buffer_size;
		fpn.fpn.auto_param[12].gain = 256;
		fpn.fpn.auto_param[11].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[11].buf_size = buffer_size;
		fpn.fpn.auto_param[11].gain = 256;                                 // ISO204800
		fpn.fpn.auto_param[10].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[10].buf_size = buffer_size;
		fpn.fpn.auto_param[10].gain = fpn.fpn.auto_param[11].gain >> 1;    // ISO102400
		fpn.fpn.auto_param[9].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[9].buf_size = buffer_size;
		fpn.fpn.auto_param[9].gain = fpn.fpn.auto_param[10].gain >> 1;     // ISO51200
		fpn.fpn.auto_param[8].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[8].buf_size = buffer_size;
		fpn.fpn.auto_param[8].gain = fpn.fpn.auto_param[9].gain >> 1;      // ISO25600
		fpn.fpn.auto_param[7].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[7].buf_size = buffer_size;
		fpn.fpn.auto_param[7].gain = fpn.fpn.auto_param[8].gain >> 1;      // ISO12800
		fpn.fpn.auto_param[6].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[6].buf_size = buffer_size;
		fpn.fpn.auto_param[6].gain = fpn.fpn.auto_param[7].gain >> 1;      // ISO6400
		fpn.fpn.auto_param[5].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[5].buf_size = buffer_size;
		fpn.fpn.auto_param[5].gain = fpn.fpn.auto_param[6].gain >> 1;      // ISO3200
		fpn.fpn.auto_param[4].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[4].buf_size = buffer_size;
		fpn.fpn.auto_param[4].gain = fpn.fpn.auto_param[5].gain >> 1;      // ISO1600
		fpn.fpn.auto_param[3].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[3].buf_size = buffer_size;
		fpn.fpn.auto_param[3].gain = fpn.fpn.auto_param[4].gain >> 1;      // ISO800
		fpn.fpn.auto_param[2].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[2].buf_size = buffer_size;
		fpn.fpn.auto_param[2].gain = fpn.fpn.auto_param[3].gain >> 1;      // ISO400
		fpn.fpn.auto_param[1].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[1].buf_size = buffer_size;
		fpn.fpn.auto_param[1].gain = fpn.fpn.auto_param[2].gain >> 1;      // ISO200
		fpn.fpn.auto_param[0].buf_phyaddr = fpn_buffer.pa;
		fpn.fpn.auto_param[0].buf_size = buffer_size;
		fpn.fpn.auto_param[0].gain = fpn.fpn.auto_param[1].gain >> 1;      // ISO100
	} else {
		fpn.id = 0;
		fpn.fpn.enable = FALSE;
	}

	vendor_isp_set_iq(IQT_ITEM_FPN_PARAM, &fpn);

	printf("load %s \n", fpn_raw);

	#if (0)
	printf("fpn id = %d \n", fpn.id);
	printf("fpn enable = %d \n", fpn.fpn.enable);
	if (fpn_ret == HD_OK) {
		printf("fpn mode = %d \n", fpn.fpn.mode);
		printf("fpn auto_param[6].buf_size = %d \n", fpn.fpn.auto_param[6].buf_size);
		printf("fpn auto_param[6].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[6].buf_phyaddr);
		printf("fpn auto_param[6].gain = %d \n", fpn.fpn.auto_param[6].gain);
		printf("fpn auto_param[7].buf_size = %d \n", fpn.fpn.auto_param[7].buf_size);
		printf("fpn auto_param[7].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[7].buf_phyaddr);
		printf("fpn auto_param[7].gain = %d \n", fpn.fpn.auto_param[7].gain);
		printf("fpn auto_param[8].buf_size = %d \n", fpn.fpn.auto_param[8].buf_size);
		printf("fpn auto_param[8].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[8].buf_phyaddr);
		printf("fpn auto_param[8].gain = %d \n", fpn.fpn.auto_param[8].gain);
		printf("fpn auto_param[9].buf_size = %d \n", fpn.fpn.auto_param[9].buf_size);
		printf("fpn auto_param[9].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[9].buf_phyaddr);
		printf("fpn auto_param[9].gain = %d \n", fpn.fpn.auto_param[9].gain);
		printf("fpn auto_param[10].buf_size = %d \n", fpn.fpn.auto_param[10].buf_size);
		printf("fpn auto_param[10].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[10].buf_phyaddr);
		printf("fpn auto_param[10].gain = %d \n", fpn.fpn.auto_param[10].gain);
		printf("fpn auto_param[11].buf_size = %d \n", fpn.fpn.auto_param[11].buf_size);
		printf("fpn auto_param[11].buf_phyaddr = 0x%x \n", fpn.fpn.auto_param[11].buf_phyaddr);
		printf("fpn auto_param[11].gain = %d \n", fpn.fpn.auto_param[11].gain);

		printf("fpn auto_param[n].buf = {0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, ...} \n"
			, *((UINT32 *)fpn_buffer.va + 0), *((UINT32 *)fpn_buffer.va + 1), *((UINT32 *)fpn_buffer.va + 2), *((UINT32 *)fpn_buffer.va + 3)
			, *((UINT32 *)fpn_buffer.va + 4), *((UINT32 *)fpn_buffer.va + 5), *((UINT32 *)fpn_buffer.va + 6), *((UINT32 *)fpn_buffer.va + 7));
	}
	#endif
}

static HD_RESULT isp_fpn_free_mem(void) {
	HD_RESULT ret = HD_OK;

	if (fpn_buffer.va != NULL) {
		ret = hd_common_mem_free(fpn_buffer.pa, fpn_buffer.va);
		if (ret != HD_OK) {
			printf("DDR0 free pa = 0x%lx, va = 0x%lx fail \n", (UINTPTR)(fpn_buffer.pa), (UINTPTR)(fpn_buffer.va));
		}
	}

	return ret;
}
#endif

//=============================================================================
// external functions
//=============================================================================
void isp_set_total_gain(UINT32 gain)
{
	init_total_gain = gain;
}

BOOL isp_get_msg_print(void)
{
	return print_aiisp_param_en;
}

void isp_set_msg_print(BOOL enable)
{
	print_aiisp_param_en = enable;
}

HD_RESULT isp_enable_socket(void)
{
	HD_RESULT ret = HD_OK;

	#if (AIISP_PARAM_SOCKET)
	net_proc_exit = 0;

	ret = pthread_create(&socket_thread_id, NULL, server_thread, NULL);
	if (ret < 0) {
		net_proc_exit = 1;
		printf("create sever thread fail = %d \n", ret);
	}
	#endif

	return ret;
}

HD_RESULT isp_disable_socket(void)
{
	HD_RESULT ret = HD_OK;

	#if (AIISP_PARAM_SOCKET)
	net_proc_exit = 1;

	sleep(1);

	if (socket_thread_id) {
		pthread_join(socket_thread_id, NULL);
	}
	#endif

	return ret;
}

HD_RESULT isp_get_json_name(CHAR *json_name)
{
	HD_RESULT ret = HD_OK;

	memcpy(json_name, aiisp_json_name, sizeof(aiisp_json_name));

	return ret;
}

HD_RESULT isp_set_json_name(CHAR *json_name)
{
	HD_RESULT ret = HD_OK;

	memcpy(aiisp_json_name, json_name, sizeof(aiisp_json_name));

	return ret;
}

HD_RESULT isp_load_json(void)
{
	HD_RESULT ret = HD_OK;

	#if (AIISP_LOAD_JSON)
	CHAR aiisp_json_path[128];

	snprintf(aiisp_json_path, 128, "/etc/isp/%s.json", aiisp_json_name);
	aiisp_pq_parse_json(aiisp_json_path, &aiisp_pq_param);
	printf("load %s \n", aiisp_json_path);

	isp_intpl_aiisp_param(init_total_gain, &aiisp_pq_param, &aiisp_pq_final_param);
	ai_set_param(&aiisp_pq_final_param);
	#endif

	return ret;
}

HD_RESULT isp_get_fpn_name(CHAR *fpn_name)
{
	HD_RESULT ret = HD_OK;

	memcpy(fpn_name, isp_fpn_name, sizeof(isp_fpn_name));

	return ret;
}

HD_RESULT isp_set_fpn_name(CHAR *fpn_name)
{
	HD_RESULT ret = HD_OK;

	memcpy(isp_fpn_name, fpn_name, sizeof(isp_fpn_name));

	return ret;
}

HD_RESULT isp_enable_fpn(UINT32 isp_id)
{
	HD_RESULT ret = HD_OK;

	#if (ISP_FPN_ENABLE)
	isp_fpn_set(isp_id);
	#endif

	return ret;
}

HD_RESULT isp_disable_fpn(void)
{
	HD_RESULT ret = HD_OK;

	#if (ISP_FPN_ENABLE)
	isp_fpn_free_mem();
	#endif

	return ret;
}

