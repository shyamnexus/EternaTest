/*
    HDMI2 TX module driver

    HDMI2 TX module driver.

    @file       hdmitx2.c
    @ingroup    mIDrvDisp_HDMI2Tx
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#include "hdmi_port.h"
#include "hdmitx_int.h"
#include "hdmitx_reg.h"






#if HDMITX_DEBUG_FORCE_DVI
// FORCE_RGB / FORCE_UNDERSCAN / VERIFY_VID /FORCE_YUV / TIMING_FACTOR_EN
UINT32              hdmi2_ctrl_option[HDMICFG_OPTION_ALL] = {TRUE,  TRUE, FALSE, FALSE, FALSE};
#else
// FORCE_RGB / FORCE_UNDERSCAN / VERIFY_VID /FORCE_YUV / TIMING_FACTOR_EN
UINT32              hdmi2_ctrl_option[HDMICFG_OPTION_ALL] = {TRUE, TRUE, FALSE, FALSE, FALSE};
#endif


/*global resource define*/
//VK_TASK_HANDLE hdmi2_tsk_hdl = NULL;
//static SEM_HANDLE SEMID_HDMI2;
//static SEM_HANDLE SEMID_HDMI2_HPD;
#define FLGPTN_HDMI2			FLGPTN_BIT(0) 
#define DRV_INT_HDMI2        INT_ID_HDMI2
static ID       FLG_ID_HDMI2;

#if defined(__FREERTOS)

static  VK_DEFINE_SPINLOCK(hdmi_spinlock);
static unsigned long hdmi2_spinlock_flags;
#define loc_cpu() vk_spin_lock_irqsave(&hdmi_spinlock, hdmi2_spinlock_flags)
#define unl_cpu() vk_spin_unlock_irqrestore(&hdmi_spinlock, hdmi2_spinlock_flags)


extern void	nvt_enable_irq(int number);
extern void	nvt_disable_irq(int number);
#define drv_enable_int      nvt_enable_irq
#define drv_disable_int     nvt_disable_irq
#endif


/*global variable define*/
static BOOL         b_hdmi2_tx_open     = FALSE;
static UINT32       hdmi2_int_sts;
UINT32 		 		hdmi2_clock_rate = 148500000;
static BOOL 		hdmi2_init = FALSE;
BOOL				b_hdmi2_edid_msg			= DISABLE;
UINT32              hdmi2_trim_rsel			= HDMITX_RTRIM_DEFAULT;
UINT32              hdmi2_manufacturer_id;
BOOL                b_hdmi2_i2c_opened;
I2C_SESSION         hdmi2_i2c_ses;
BOOL                hdmi2_i2c_select = 0;

static hdmi_state_t hdmi2_state = STATE_NONE;
static int hdmi2_hpd_status;



#ifdef __KERNEL__
HDMI_I2C_OBJ        hdmi2_i2c_obj[1] = {
	{
		hdmitx2_i2c_open,
		hdmitx2_i2c_close,
		hdmitx2_i2c_set_config,
	}
};
#else
HDMI_I2C_OBJ        hdmi2_i2c_obj[1] = {
#if 1
	{
		i2c11_open,
		i2c11_close,
		i2c11_lock,
		i2c11_unlock,
		i2c11_setConfig,
		i2c11_transmit,
		i2c11_receive,
	}
#else
	NULL,
#endif
};
#endif
#if (defined __KERNEL__ || defined __FREERTOS)
//#ifdef __KERNEL__
ER hdmitx2_i2c_open(PI2C_SESSION p_session)
{
	if (b_hdmi2_i2c_opened)
		return E_OK;

	b_hdmi2_i2c_opened = 1;
	return E_OK;
}
ER hdmitx2_i2c_close(I2C_SESSION session)
{
	if (!b_hdmi2_i2c_opened)
		return E_OK;

	b_hdmi2_i2c_opened = 0;
	return E_OK;
}
void hdmitx2_i2c_set_config(I2C_SESSION session, I2C_CONFIG_ID config_id, UINT32 ui_config)
{}

#endif

#if (defined __UITRON || defined __ECOS)
//#ifndef __KERNEL__
extern UINT32       hdmi_sw_ddc_scl, hdmi_sw_ddc_sda;
#endif

ER hdmitx2_write_phy(UINT32 ui_offset, UINT32 ui_value);
ER hdmitx2_read_phy(UINT32 ui_offset, UINT32 *pui_value);



void hdmitx2_set_tmds_oe(BOOL b_en);

#if 0
THREAD_RETTYPE hdmi2_polling_tsk(void *param)
{
	int status;
	int hpg_change_cnt = 0;
	
	DBG_WRN("%s is running\n", __func__);
	
	do {		
		vos_sem_wait(SEMID_HDMI2_HPD);

		if (hdmi2_state & HDMI_THREAD_STOP){
			hdmi2_state &= ~HDMI_THREAD_STOP;
			THREAD_RETURN(0);
		}
	
		/* check hotplug */
		status = hdmitx2_check_hotplug();
			
		if (hdmi2_state & HDMI_OPEN) {
			if (status != hdmi2_hpd_status) {

				if (hdmi_cts_test == 0) {
					hpg_change_cnt ++;
					if (hpg_change_cnt < 20){
						//DBG_WRN("hdmi: hotplug chg cnt [%d]!\n",hpg_change_cnt);
						goto loop;
					} else {
						hpg_change_cnt = 0;
						//DBG_WRN("hdmi: hotplug chged! cnt = [%d]\n",hpg_change_cnt);
					}
				}	
				
				if (status == TRUE) {
					hdmitx2_hotplug_on();
					DBG_WRN("hdmi: hotplug is ON!\n");
				} else {
					hdmitx2_hotplug_off();
					DBG_WRN("hdmi: hotplug is OFF!\n");
				}
				hdmi2_state |= HPD_CHANGE;
				hdmi2_hpd_status = status;
			}
		}
loop:		
		vos_sem_sig(SEMID_HDMI2_HPD);
		vos_task_delay_ms(5);
#ifdef __KERNEL__
	} while(!nvt_hdmitx_drv_task_should_stop());
	
	DBG_WRN("%s is stopped\n", __func__);
	
	return 0;	
#else
	} while(1);
#endif

}
#endif

void hdmitx2_create_resource(void)
{
	//vos_flag_create(&FLG_ID_HDMI2, NULL, "FLG_ID_HDMI2");
	//vos_sem_create(&SEMID_HDMI2, 1, "SEMID_HDMI2");
	//vos_sem_create(&SEMID_HDMI2_HPD, 1, "SEMID_HDMI2_HPD");
	//hdmi2_tsk_hdl = vos_task_create(hdmi2_polling_tsk, NULL, "hdmi2_polling_tsk", 10, 4096);
	//vos_task_resume(hdmi2_tsk_hdl);
}

void hdmitx2_release_resource(void)
{
	//vos_flag_destroy(FLG_ID_HDMI2);
	//vos_sem_destroy(SEMID_HDMI2);
	//vos_sem_destroy(SEMID_HDMI2_HPD);
	//vos_task_destroy(hdmi2_tsk_hdl);
}


#if HDMITX_SCDC_RRTEST
static SWTIMER_ID   g_hdmi_sw_timer_id;
static void hdmitx2_read_request_sw_timer_cb(UINT32 ui_event)
{
	hdmitx2_scdc_write(0xC0, 0x00);
	hdmitx2_scdc_write(0x10, 0x07);
	hdmitx2_scdc_write(0xC0, 0xFF);
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_EN,        ENABLE);
	DBG_DUMP("KLINSKLINSKLINS ");
}

#endif


#if (defined __KERNEL__ || defined __FREERTOS)//#ifdef __KERNEL__

UINT32 hdmitx2_isr_check(void)
{
	T_HDMI_INTSTS_REG    reg_int_sts;

	reg_int_sts.reg = HDMI2_GETREG(HDMI_INTSTS_REG_OFS);
	reg_int_sts.reg &= HDMI2_GETREG(HDMI_INTEN_REG_OFS);

	return reg_int_sts.reg;
}
#endif

/*
    hdmi isr

    hdmi isr

    @return void

//void (void)
irqreturn_t hdmi2_isr(int irq, void *devid)
{
	T_HDMI_INTSTS_REG    reg_int_sts;
	T_HDMI_SYSCTRL_REG   reg_sys_ctrl;

	reg_int_sts.reg = HDMI2_GETREG(HDMI_INTSTS_REG_OFS);
	reg_int_sts.reg &= HDMI2_GETREG(HDMI_INTEN_REG_OFS);

	if (reg_int_sts.reg) {
		// Clear Interrupt Status
		HDMI2_SETREG(HDMI_INTSTS_REG_OFS, reg_int_sts.reg);

		hdmi2_int_sts |= reg_int_sts.reg;

		if (reg_int_sts.bit.RSEN_STS) {
			//hdmitx_debug(("HDMI2 RSEN!\r\n"));
			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			if (reg_sys_ctrl.bit.RSEN) {
				hdmitx2_set_tmds_oe(TRUE);
			} else {
				hdmitx2_set_tmds_oe(FALSE);
			}
		}

		iset_flg(FLG_ID_HDMI2, (FLGPTN) FLGPTN_HDMI2);


#if HDMITX_DEBUG
		if (reg_int_sts.bit.OVRUN_STS) {
			hdmitx_debug(("HDMI2 AUD OverRun!\r\n"));
		}
		if (reg_int_sts.bit.RSEN_STS) {
			hdmitx_debug(("HDMI2 RSEN!\r\n"));
		}
		if (reg_int_sts.bit.HPD_CHG_STS) {
			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			if (reg_sys_ctrl.bit.HPD) {
				hdmitx_debug(("HDMI2 HPD plug!\r\n"));
			} else {
				hdmitx_debug(("HDMI2 HPD unplug!\r\n"));
			}
		}
		if (reg_int_sts.bit.VSYNC_STS) {
			hdmitx_debug(("HDMI2 VSYNC!\r\n"));
		}
		if (reg_int_sts.bit.PIXLK_STBL_STS) {
			hdmitx_debug(("HDMI2 PCLK STBL!\r\n"));
		}
		if (reg_int_sts.bit.ACR_OVR_STS) {
			hdmitx_debug(("HDMI2 ACR!\r\n"));
		}
		if (reg_int_sts.bit.CTS_CHG_STS) {
			hdmitx_debug(("HDMI2 CTS-CHG!\r\n"));
		}
		if (reg_int_sts.bit.GCP_CHG_MUTE_STS) {
			hdmitx_debug(("HDMI2 GCP_CHG_MUTE_STS!\r\n"));
		}
#endif

#if HDMICEC_DEBUG
		if (reg_int_sts.bit.CEC_RX_FRM_STS) {
			hdmicec_debug(("HDMI2 CEC_RX_FRM_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_RXPOLLING_STS) {
			hdmicec_debug(("HDMI2 CEC_RXPOLLING_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_TX_FRM_STS) {
			hdmicec_debug(("HDMI2 CEC_TX_FRM_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_TX_RETRY_STS) {
			hdmicec_debug(("HDMI2 CEC_TX_RETRY_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_TX_AL_STS) {
			hdmicec_debug(("HDMI2 CEC_TX_AL_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_RX_TIMEOUT_STS) {
			hdmicec_debug(("HDMI2 CEC_RX_TIMEOUT_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_ERROR_STS) {
			hdmicec_debug(("HDMI2 CEC_ERROR_STS!\r\n"));
		}
#endif
	}
	return IRQ_HANDLED;
}
*/

#if 1

/*
    For TC72670 HDMI2 PHY Control at FPGA
*/
ER hdmitx2_write_phy(UINT32 ui_offset, UINT32 ui_value)
{
#if _FPGA_EMULATION_
#if !(HDMITX_DEBUG && HDMITX_DEBUG_SKIP_EDID)

	I2C_DATA    i2c_data;
	I2C_BYTE    i2c_byte[4];
	I2C_STS     i2c_sts;
	//UINT32      addr = 0xC0;
	UINT32      addr = 0xB0;

	//DBG_DUMP("SET TC670 REG[0x%02X] = 0x%02X\r\n",ui_offset,ui_value);

	//hdmiddc_lock(0);
	hdmi2_i2c_obj[hdmi2_i2c_select].lock(hdmi2_i2c_ses);

	//i2c_data.VersionInfo     = DRV_VER_96680;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].ui_value      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;

	i2c_byte[1].ui_value      = ui_offset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;

	i2c_byte[2].ui_value      = ui_value;
	i2c_byte[2].Param        = I2C_BYTE_PARAM_STOP;
#if 0
	//i2c_sts = hdmiddc_transmit(&i2c_data);
	i2c_sts = hdmi2_i2c_obj[hdmi2_i2c_select].transmit(&i2c_data);
	if (i2c_sts != I2C_STS_OK) {
		DBG_ERR("Tx Err. %d\r\n", i2c_sts);
	}
#else
	i2c_sts = i2c3_transmit(&i2c_data);
	if ((i2c_sts != I2C_STS_OK) && (i2c_sts != I2C_STS_STOP)) {
		DBG_ERR("Tx Err. %d\r\n", i2c_sts);
	}
#endif	
	//hdmiddc_unlock(0);
	hdmi2_i2c_obj[hdmi2_i2c_select].unlock(hdmi2_i2c_ses);
#endif
#else

#if _EMULATION_
	DBG_DUMP("HDMI2W[0x%02X]=0x%02X\r\n", ui_offset, ui_value);
#endif
	if (ui_offset >= 0x80) {
		HDMI2_SETREG(HDMI_PHYBASE_REG_OFS + 0x200 + (ui_offset << 2), ui_value);
	}
	else {
		HDMI2_SETREG(HDMI_PHYBASE_REG_OFS + (ui_offset << 2), ui_value);
	}
#endif /* _FPGA_EMULATION_ */
	return E_OK;
}

/*
    For TC72670 HDMI2 PHY Control at FPGA
*/
ER hdmitx2_read_phy(UINT32 ui_offset, UINT32 *pui_value)
{
#if _FPGA_EMULATION_
#if !(HDMITX_DEBUG && HDMITX_DEBUG_SKIP_EDID)

	I2C_DATA    i2c_data;
	I2C_BYTE    i2c_byte[4];
	I2C_STS     i2c_sts;
	//UINT32      addr = 0xC0;
	UINT32      addr = 0xB0;

	//hdmiddc_lock(0);
	hdmi2_i2c_obj[hdmi2_i2c_select].lock(hdmi2_i2c_ses);

	//i2c_data.VersionInfo     = DRV_VER_96680;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].ui_value      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;

	i2c_byte[1].ui_value      = ui_offset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;

	i2c_byte[2].ui_value      = addr | 0x1;
	i2c_byte[2].Param        = I2C_BYTE_PARAM_START;
#if 0
	//i2c_sts = hdmiddc_transmit(&i2c_data);
	i2c_sts = hdmi2_i2c_obj[hdmi2_i2c_select].transmit(&i2c_data);
	if (i2c_sts != I2C_STS_OK) {
		DBG_ERR("Tx Err1. %d\r\n", i2c_sts);
	}
#else
		i2c_sts = i2c3_transmit(&i2c_data);
		if ((i2c_sts != I2C_STS_OK) && (i2c_sts != I2C_STS_STOP)) {
			DBG_ERR("Tx Err. %d\r\n", i2c_sts);
		}
#endif

	i2c_data.ByteCount   = I2C_BYTE_CNT_1;
	i2c_byte[0].Param    = I2C_BYTE_PARAM_NACK | I2C_BYTE_PARAM_STOP;
	//if (hdmiddc_receive(&i2c_data) != I2C_STS_OK) {
	//	DBG_ERR("RX Err. %d\r\n", i2c_sts);
	//}
	if (hdmi2_i2c_obj[hdmi2_i2c_select].receive(&i2c_data) != I2C_STS_OK) {
		DBG_ERR("RX Err. %d\r\n", i2c_sts);
	}
	//hdmiddc_unlock(0);
	hdmi2_i2c_obj[hdmi2_i2c_select].unlock(hdmi2_i2c_ses);

	*pui_value = i2c_byte[0].uiValue;
#endif

#else
	if (ui_offset >= 0x80) {
		*pui_value = HDMI2_GETREG(HDMI_PHYBASE_REG_OFS  + 0x200 + (ui_offset << 2));
	}
	else {
		*pui_value = HDMI2_GETREG(HDMI_PHYBASE_REG_OFS + (ui_offset << 2));
	}
#endif

	return E_OK;
}


#if !_FPGA_EMULATION_
static void hdmitx2_validate_rtrim(void)
{
#if !_EMULATION_
	
#if HDMI_EFUSE_USED	
	u32  trim;
	int  code;
	BOOL is_found;
	u32  HDMI_resistor_1_4, HDMI_LANE_VL_1_4;
	u32  HDMI_resistor_2_0, HDMI_LANE_VL_2_0;
	u32 val_R;
	code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_V_PHY_RESISTOR_PRI);
	printf("otp_key_manager code = [%d]\n", code);
	if (code == -33) {
		hdmitx2_write_phy(0x0C, 0x0D); // default value
	    printf("[%d] data = NULL\n", EFUSE_TRIM_DATA_HDMI_V_PHY_RESISTOR_PRI);
	    return ;
	} else {
	    is_found = extract_trim_valid(code, (u32 *)&trim);
	    if (is_found) {
	        //bit[4..0]
		HDMI_resistor_1_4 = trim & 0x1F;
		//bit[9..5]
		HDMI_resistor_2_0 = ((trim >> 5) & 0x1F);

		
			if (HDMI_resistor_1_4 > 0x14) {
				hdmitx2_write_phy(0x0C, 0x0D); // default value
		  	    printf("1.4 HDMI2 resistor Trim data error 0x%04x > 0x14\r\n", (int)HDMI_resistor_1_4);
		    } else if (HDMI_resistor_1_4 < 0x8) {
		    	hdmitx2_write_phy(0x0C, 0x0D); // default value
				printf("1.4 HDMI2 resistor Trim data error 0x%04x < 0x8\r\n", (int)HDMI_resistor_1_4);
		    } else {
		    	if(clock_rate <= 297000000){
					hdmitx2_write_phy(0x0C, HDMI_resistor_1_4);
				}
				printf("  *1.4 HDMI2 resistor Trim data range success 0x8 <= [0x%04x] <= 0x14\r\n", (int)HDMI_resistor_1_4);
		    }
			
		    if (HDMI_resistor_2_0 > 0x14) {
				hdmitx2_write_phy(0x0C, 0x0D); // default value
		        printf("2.0 HDMI2 resistor Trim data error 0x%04x > 0x14\r\n", (int)HDMI_resistor_2_0);
			} else if (HDMI_resistor_2_0 < 0x8) {
				hdmitx2_write_phy(0x0C, 0x0D); // default value
				printf("2.0 HDMI2 resistor Trim data error 0x%04x < 0x8\r\n", (int)HDMI_resistor_2_0);
			} else {
				if (clock_rate >  297000000){
					hdmitx2_write_phy(0x0C, HDMI_resistor_2_0);
				}
				printf("  *2.0 HDMI2 resistor Trim data range success 0x8 <= [0x%04x] <= 0x14\r\n", (int)HDMI_resistor_2_0);
			} 
			
	    } else {
	        //!!!Please apply default value here!!!
	        hdmitx2_write_phy(0x0C, 0x0D); // default value
			printf("is found [%d][HDMI2 resistor] = 0x%08x\r\n", is_found, (int)code);
	    }			
	}

	if (clock_rate >  297000000){//HDMI2.0
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_2_0_V_PHY_VL_TRIM_PRI);
		if (code == -33) {
		    printf("[%d] data = NULL\n", EFUSE_TRIM_DATA_HDMI_2_0_V_PHY_VL_TRIM_PRI);
		    return ;
		} else {
		    is_found = extract_trim_valid(code, (u32 *)&trim);
		    if (is_found) {
		        printf("  HDMI2 2.0 VLane tunning Trim = 0x%04x\r\n", (int)trim);
			//bit[11..0]
			HDMI_LANE_VL_2_0 = trim & 0xFFF;
			val_R = ((HDMI_LANE_VL_2_0 >> 0) & (0x7)) | (((HDMI_LANE_VL_2_0 >> 3) & (0x7))<<4);
			hdmitx2_write_phy(0x0D, val_R|0x08);
			val_R = ((HDMI_LANE_VL_2_0 >> 6) & (0x7)) | (((HDMI_LANE_VL_2_0 >> 9) & (0x7))<<4);
			hdmitx2_write_phy(0x0E, val_R);		
			
			printf("  *2.0 HDMI2 VLane Trim data range success 0x0 <= [0x%04x] <= 0xFFF\r\n", (int)HDMI_LANE_VL_2_0);
		    } else {
		        //!!!Please apply default value here!!!
			hdmitx2_write_phy(0x0D, 0x44|0x08); // default value | bit3 reg_restrim_en = 1
			hdmitx2_write_phy(0x0E, 0x44); // default value			        
			printf("is found [%d][HDMI2 2.0 VLNE] = 0x%08x\r\n", is_found, (int)code);
		    }	
		}
	} else {
		code = otp_key_manager(EFUSE_TRIM_DATA_HDMI_1_4_V_PHY_VL_TRIM_PRI);
		if (code == -33) {
		    printf("[%d] data = NULL\n", EFUSE_TRIM_DATA_HDMI_1_4_V_PHY_VL_TRIM_PRI);
		    return ;
		} else {
		    is_found = extract_trim_valid(code, (u32 *)&trim);
		    if (is_found) {
		        printf("  HDMI2 1.4 VLane tunning Trim = 0x%04x\r\n", (int)trim);
			//bit[11..0]
			HDMI_LANE_VL_1_4 = trim & 0xFFF;
			
			val_R = ((HDMI_LANE_VL_1_4 >> 0) & (0x7)) | (((HDMI_LANE_VL_1_4 >> 3) & (0x7))<<4);
			hdmitx2_write_phy(0x0D, val_R|0x08);
			val_R = ((HDMI_LANE_VL_1_4 >> 6) & (0x7)) | (((HDMI_LANE_VL_1_4 >> 9) & (0x7))<<4);
			hdmitx2_write_phy(0x0E, val_R);			
			
			printf("  *1.4 HDMI2 VLane Trim data range success 0x0 <= [0x%04x] <= 0xFFF\r\n", (int)HDMI_LANE_VL_1_4);
		    } else {
		        //!!!Please apply default value here!!!
			hdmitx2_write_phy(0x0D, 0x44|0x08); // default value
			hdmitx2_write_phy(0x0E, 0x44); // default value			        
			printf("is found [%d][HDMI2 1.4 VLNE] = 0x%08x\r\n", is_found, (int)code);
		    }	
		}
	}

#endif

#endif
}
#endif


/*
    For TC72670 HDMI2 PHY Control at FPGA
*/
static void hdmitx2_init_phy(void)
{
#if !_FPGA_EMULATION_	
	// apply different phy setting for each clock rate

	if (hdmi2_clock_rate < 50000000) {
		/*27~54Mhz*/
		hdmitx2_write_phy(0x30, 0x00);
		hdmitx2_write_phy(0x09, 0x04);
		hdmitx2_write_phy(0x09, 0x84);
		hdmitx2_write_phy(0x09, 0x86);
		hdmitx2_write_phy(0x09, 0x87);
		DELAY_MS(1);
		hdmitx2_write_phy(0x05, 0x15);
		hdmitx2_write_phy(0x01, 0x01);
		
		hdmitx2_write_phy(0x07, 0x01);
		hdmitx2_write_phy(0x00, 0x10);
		hdmitx2_write_phy(0x05, 0x15);
		hdmitx2_write_phy(0x04, 0x9B);
		
		hdmitx2_write_phy(0x08, 0x00);
		hdmitx2_write_phy(0x08, 0x08); //PLL, power on
		DELAY_MS(10);
		
		hdmitx2_write_phy(0x01, 0x00);
		hdmitx2_write_phy(0x0B, 0x1D);
		/* trim apply flow start*/
		/* 0x0C, 0x0D, 0x0E*/
		// Set Calibrated resistor value
		hdmitx2_validate_rtrim();		
		/* trim apply flow end*/

		hdmitx2_write_phy(0x7C, 0x00);
		hdmitx2_write_phy(0x7D, 0x00);
		hdmitx2_write_phy(0x7E, 0x00);
		hdmitx2_write_phy(0x7F, 0x00);
		hdmitx2_write_phy(0x08, 0x38);
		hdmitx2_write_phy(0x98, 0x00);
		hdmitx2_write_phy(0x99, 0x00);
		hdmitx2_write_phy(0x7B, 0x0F);
		hdmitx2_write_phy(0x9B, 0x00);
		hdmitx2_write_phy(0x9C, 0x00);
		hdmitx2_write_phy(0x7B, 0xFF);
		hdmitx2_write_phy(0x9A, 0x00);
		hdmitx2_write_phy(0x7A, 0x0F);
		hdmitx2_write_phy(0x7A, 0xFF);
		
	} else if (hdmi2_clock_rate <  110000000){
		/*54~110Mhz*/
		hdmitx2_write_phy(0x30, 0x00);
		hdmitx2_write_phy(0x09, 0x04);
		hdmitx2_write_phy(0x09, 0x84);
		hdmitx2_write_phy(0x09, 0x86);
		hdmitx2_write_phy(0x09, 0x87);
		DELAY_MS(1);
		hdmitx2_write_phy(0x05, 0x15);
		hdmitx2_write_phy(0x01, 0x01);
		
		hdmitx2_write_phy(0x07, 0x01);
		hdmitx2_write_phy(0x00, 0x00);
		hdmitx2_write_phy(0x05, 0x17);
		hdmitx2_write_phy(0x04, 0x9B);
		
		hdmitx2_write_phy(0x08, 0x00);
		hdmitx2_write_phy(0x08, 0x08); //PLL, power on
		DELAY_MS(10);
		
		hdmitx2_write_phy(0x01, 0x00);
		hdmitx2_write_phy(0x0B, 0x1D);
		/* trim apply flow start*/
		/* 0x0C, 0x0D, 0x0E*/
		hdmitx2_validate_rtrim();
		/* trim apply flow end*/

		hdmitx2_write_phy(0x7C, 0x00);
		hdmitx2_write_phy(0x7D, 0x00);
		hdmitx2_write_phy(0x7E, 0x00);
		hdmitx2_write_phy(0x7F, 0x00);
		hdmitx2_write_phy(0x08, 0x38);
		hdmitx2_write_phy(0x98, 0x00);
		hdmitx2_write_phy(0x99, 0x00);
		hdmitx2_write_phy(0x7B, 0x0F);
		hdmitx2_write_phy(0x9B, 0x00);
		hdmitx2_write_phy(0x9C, 0x00);
		hdmitx2_write_phy(0x7B, 0xFF);
		hdmitx2_write_phy(0x9A, 0x00);
		hdmitx2_write_phy(0x7A, 0x0F);
		hdmitx2_write_phy(0x7A, 0xFF);	


		hdmitx2_write_phy(0xD7, 0xC0);
		hdmitx2_write_phy(0xD8, 0x06);
		hdmitx2_write_phy(0xD9, 0x00);
	} else if (hdmi2_clock_rate <  180000000){
		/*110~180Mhz*/
		hdmitx2_write_phy(0x30, 0x00);
		hdmitx2_write_phy(0x09, 0x04);
		hdmitx2_write_phy(0x09, 0x84);
		hdmitx2_write_phy(0x09, 0x86);
		hdmitx2_write_phy(0x09, 0x87);
		DELAY_MS(1);
		hdmitx2_write_phy(0x05, 0x15);
		hdmitx2_write_phy(0x01, 0x01);
		
		hdmitx2_write_phy(0x07, 0x01);
		hdmitx2_write_phy(0x00, 0x00);
		hdmitx2_write_phy(0x05, 0x17);
		hdmitx2_write_phy(0x04, 0x9B);
		
		hdmitx2_write_phy(0x08, 0x00);
		hdmitx2_write_phy(0x08, 0x08); //PLL, power on
		DELAY_MS(10);
		
		hdmitx2_write_phy(0x01, 0x00);
		hdmitx2_write_phy(0x0B, 0x1D);
		/* trim apply flow start*/
		/* 0x0C, 0x0D, 0x0E*/
		// Set Calibrated resistor value
		hdmitx2_validate_rtrim();	
		/* trim apply flow end*/

		hdmitx2_write_phy(0x7C, 0x00);
		hdmitx2_write_phy(0x7D, 0x00);
		hdmitx2_write_phy(0x7E, 0x00);
		hdmitx2_write_phy(0x7F, 0x00);
		hdmitx2_write_phy(0x08, 0x38);
		hdmitx2_write_phy(0x98, 0x00);
		hdmitx2_write_phy(0x99, 0x00);
		hdmitx2_write_phy(0x7B, 0x0F);
		hdmitx2_write_phy(0x9B, 0x00);
		hdmitx2_write_phy(0x9C, 0x00);
		hdmitx2_write_phy(0x7B, 0xFF);
		hdmitx2_write_phy(0x9A, 0x00);
		hdmitx2_write_phy(0x7A, 0x0F);
		hdmitx2_write_phy(0x7A, 0xFF);			
	} else if (hdmi2_clock_rate <= 297000000){
		/*180~297Mhz*/
		hdmitx2_write_phy(0x30, 0x00);
		hdmitx2_write_phy(0x09, 0x04);
		hdmitx2_write_phy(0x09, 0x84);
		hdmitx2_write_phy(0x09, 0x86);
		hdmitx2_write_phy(0x09, 0x87);
		DELAY_MS(1);
		hdmitx2_write_phy(0x05, 0x15);
		hdmitx2_write_phy(0x01, 0x01);
		
		hdmitx2_write_phy(0x07, 0x05);
		hdmitx2_write_phy(0x00, 0x00);
		hdmitx2_write_phy(0x05, 0x29);
		hdmitx2_write_phy(0x04, 0x9B);
		
		hdmitx2_write_phy(0x08, 0x00);
		hdmitx2_write_phy(0x08, 0x08); //PLL, power on
		DELAY_MS(10);
		
		hdmitx2_write_phy(0x01, 0x00);
		hdmitx2_write_phy(0x0B, 0x1D);
		/* trim apply flow start*/
		/* 0x0C, 0x0D, 0x0E*/
		// Set Calibrated resistor value
		hdmitx2_validate_rtrim();		
		/* trim apply flow end*/

		hdmitx2_write_phy(0x7C, 0x04);
		hdmitx2_write_phy(0x7D, 0x04);
		hdmitx2_write_phy(0x7E, 0x04);
		hdmitx2_write_phy(0x7F, 0x04);
		hdmitx2_write_phy(0x08, 0x38);
		hdmitx2_write_phy(0x98, 0x44);
		hdmitx2_write_phy(0x99, 0x44);
		hdmitx2_write_phy(0x7B, 0x0F);
		hdmitx2_write_phy(0x9B, 0x22);
		hdmitx2_write_phy(0x9C, 0x12);
		hdmitx2_write_phy(0x7B, 0xFF);
		hdmitx2_write_phy(0x9A, 0x00);
		hdmitx2_write_phy(0x7A, 0x0F);
		hdmitx2_write_phy(0x7A, 0xFF);			
	} else if (hdmi2_clock_rate >  297000000){
		/*297~594Mhz*/
		hdmitx2_write_phy(0x30, 0x06);
		hdmitx2_write_phy(0x09, 0x04);
		hdmitx2_write_phy(0x09, 0x84);
		hdmitx2_write_phy(0x09, 0x86);
		hdmitx2_write_phy(0x09, 0x87);
		DELAY_MS(1);
		hdmitx2_write_phy(0x05, 0x15);
		hdmitx2_write_phy(0x01, 0x01);
		
		hdmitx2_write_phy(0x07, 0x05);
		hdmitx2_write_phy(0x00, 0x00);
		hdmitx2_write_phy(0x05, 0x3B);
		hdmitx2_write_phy(0x04, 0x9B);
		
		hdmitx2_write_phy(0x08, 0x00);
		hdmitx2_write_phy(0x08, 0x08); //PLL, power on
		DELAY_MS(10);
		
		hdmitx2_write_phy(0x01, 0x00);
		hdmitx2_write_phy(0x0B, 0x05);
		/* trim apply flow start*/
		/* 0x0C, 0x0D, 0x0E*/
		// Set Calibrated resistor value
		hdmitx2_validate_rtrim();		
		/* trim apply flow end*/

		hdmitx2_write_phy(0x7C, 0x1B);
		hdmitx2_write_phy(0x7D, 0x1B);
		hdmitx2_write_phy(0x7E, 0x1B);
		hdmitx2_write_phy(0x7F, 0x1B);
		hdmitx2_write_phy(0x08, 0x38);
		hdmitx2_write_phy(0x98, 0xFF);
		hdmitx2_write_phy(0x99, 0xFF);
		hdmitx2_write_phy(0x7B, 0x0F);
		hdmitx2_write_phy(0x9B, 0x22);
		hdmitx2_write_phy(0x9C, 0x12);
		hdmitx2_write_phy(0x7B, 0xFF);
		hdmitx2_write_phy(0x9A, 0x15);
		hdmitx2_write_phy(0x7A, 0x0F);
		hdmitx2_write_phy(0x7A, 0xFF);	
	}
	DBG_WRN("=================================\n");
	DBG_WRN("hdmi2_clock_rate  = [%d Hz]\n", hdmi2_clock_rate);


#else //_FPGA_EMULATION_

#if !(HDMITX_DEBUG && HDMITX_DEBUG_SKIP_EDID)
	//I2C_SESSION sess = I2C_TOTAL_SESSION;

	DBG_DUMP("Init PHY Start\r\n");
	
	//hdmitx2_set_config(HDMI_CONFIG_ID_SWDDC_SCL, P_GPIO_24);
	//hdmitx2_set_config(HDMI_CONFIG_ID_SWDDC_SDA, P_GPIO_25);

	//hdmiddc_open(&sess);
	//hdmiddc_set_config(sess, I2C_CONFIG_ID_BUSCLOCK,      80000);
	//hdmiddc_set_config(sess, I2C_CONFIG_ID_HANDLE_NACK,   TRUE);
	//hdmi2_i2c_obj[hdmi2_i2c_select].open(&hdmi2_i2c_ses);
	if (b_hdmi2_i2c_opened == FALSE) {
		if (hdmi2_i2c_obj[hdmi2_i2c_select].open(&hdmi2_i2c_ses) == E_OK) {
			b_hdmi2_i2c_opened = TRUE;			
		} else {
			b_hdmi2_i2c_opened = FALSE;
		}
	}
	//hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_MODE,        I2C_MODE_MASTER);
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_BUSCLOCK,     80000);
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_HANDLE_NACK,  TRUE);

#if 1 //NT18034

	DBG_DUMP("Init TC18034 PHY Start\r\n");


#if 1//0:pattern gen, 1:normal flow
    hdmitx2_write_phy(0xFF, 0x00);
    hdmitx2_write_phy(0x02, 0x03);
    hdmitx2_write_phy(0x03, 0x81);
    hdmitx2_write_phy(0xFF, 0x02);
    hdmitx2_write_phy(0x30, 0x10);

    hdmitx2_write_phy(0xFF, 0x00);
    hdmitx2_write_phy(0xE2, 0x03);
    hdmitx2_write_phy(0xE2, 0x02);

    hdmitx2_write_phy(0xFF, 0x02);
    hdmitx2_write_phy(0x09, 0x43);
    hdmitx2_write_phy(0x05, 0x05);
    hdmitx2_write_phy(0x0B, 0x1D);
    hdmitx2_write_phy(0x0C, 0x10);
    hdmitx2_write_phy(0x0E, 0x65);

    hdmitx2_write_phy(0xFF, 0x00);
    hdmitx2_write_phy(0xE3, 0x01);
    hdmitx2_write_phy(0xFF, 0x01);
    hdmitx2_write_phy(0x88, 0x00);
    hdmitx2_write_phy(0x89, 0x60);
    hdmitx2_write_phy(0x8A, 0x0C);

#else
    hdmitx2_write_phy(0xFF, 0x00);
    hdmitx2_write_phy(0xE2, 0x03);
    hdmitx2_write_phy(0xE2, 0x02);

    hdmitx2_write_phy(0xFF, 0x02);
    hdmitx2_write_phy(0x09, 0x43);
    hdmitx2_write_phy(0x05, 0x05);
    hdmitx2_write_phy(0x0B, 0x1D);
    hdmitx2_write_phy(0x0C, 0x10);
    hdmitx2_write_phy(0x0E, 0x65);

    hdmitx2_write_phy(0xFF, 0x00);
    hdmitx2_write_phy(0xE3, 0x01);
    hdmitx2_write_phy(0xFF, 0x01);
    hdmitx2_write_phy(0x88, 0x00);
    hdmitx2_write_phy(0x89, 0x60);
    hdmitx2_write_phy(0x8A, 0x0C);

    // TG
    hdmitx2_write_phy(0xff, 0x02);// p // Page 9
    hdmitx2_write_phy(0x15, 0x72);// p
    hdmitx2_write_phy(0x16, 0x06);// p
    hdmitx2_write_phy(0x17, 0x28);// p
    hdmitx2_write_phy(0x18, 0x00);// p
    hdmitx2_write_phy(0x19, 0x04);// p
    hdmitx2_write_phy(0x1a, 0x01);// p
    hdmitx2_write_phy(0x1b, 0x00);// p
    hdmitx2_write_phy(0x1c, 0x05);// p
    hdmitx2_write_phy(0x1d, 0xee);// p
    hdmitx2_write_phy(0x1e, 0x02);// p
    hdmitx2_write_phy(0x1f, 0x05);// p
    hdmitx2_write_phy(0x20, 0x00);// p
    hdmitx2_write_phy(0x21, 0x19);// p
    hdmitx2_write_phy(0x22, 0x00);// p
    hdmitx2_write_phy(0x23, 0xd0);// p
    hdmitx2_write_phy(0x24, 0x02);// p
    hdmitx2_write_phy(0x11, 0x1a);// p
    hdmitx2_write_phy(0x10, 0x09);// p
#endif
#endif
#else // TC72670
	//// DDR Setting (Main)
	hdmitx2_write_phy(0xff, 0x05); // Page 0x05

	// 以下為DDR Initial (好像跟Default一樣, 但我們還是再設一次)
	hdmitx2_write_phy(0x8d, 0x02);
	hdmitx2_write_phy(0x8e, 0x07);
	hdmitx2_write_phy(0xd7, 0x03);
	hdmitx2_write_phy(0xd8, 0x00);
	hdmitx2_write_phy(0xd9, 0x0e);
	hdmitx2_write_phy(0x8d, 0x02);
	hdmitx2_write_phy(0x8e, 0x07);
	hdmitx2_write_phy(0x93, 0x90);
	hdmitx2_write_phy(0x93, 0xd0);
	hdmitx2_write_phy(0x95, 0x30);
	hdmitx2_write_phy(0x09, 0xf0);
	hdmitx2_write_phy(0xc2, 0x00);
	hdmitx2_write_phy(0xad, 0x1f);
	hdmitx2_write_phy(0xb0, 0x5b);
	hdmitx2_write_phy(0xb7, 0x1b);
	hdmitx2_write_phy(0xb1, 0xbf);
	hdmitx2_write_phy(0xb2, 0x2d);
	hdmitx2_write_phy(0xb3, 0x15);
	hdmitx2_write_phy(0xae, 0x2d);
	hdmitx2_write_phy(0xaf, 0x15);
	hdmitx2_write_phy(0xb4, 0x2d);
	hdmitx2_write_phy(0xb5, 0x15);
	hdmitx2_write_phy(0xbc, 0x2d);
	hdmitx2_write_phy(0xbd, 0x15);
	hdmitx2_write_phy(0xb8, 0x99);
	hdmitx2_write_phy(0xb9, 0x99);
	hdmitx2_write_phy(0xa9, 0xe0);
	hdmitx2_write_phy(0x9e, 0x03);
	hdmitx2_write_phy(0x24, 0xfc);
	hdmitx2_write_phy(0x20, 0x02);
	hdmitx2_write_phy(0x21, 0x02);
	hdmitx2_write_phy(0x22, 0x02);
	hdmitx2_write_phy(0x23, 0x02);
	hdmitx2_write_phy(0x07, 0x00);
	hdmitx2_write_phy(0x08, 0x00);
	hdmitx2_write_phy(0x2a, 0x00);
	hdmitx2_write_phy(0x2b, 0x00);
	hdmitx2_write_phy(0x2c, 0x00);
	hdmitx2_write_phy(0x2d, 0x00);
	hdmitx2_write_phy(0x14, 0x00);
	hdmitx2_write_phy(0x16, 0x00);
	hdmitx2_write_phy(0x03, 0x01);
	hdmitx2_write_phy(0x19, 0x00);
	hdmitx2_write_phy(0x51, 0x09);

	hdmitx2_write_phy(0x01, 0x80);
	hdmitx2_write_phy(0x15, 0x40);
	hdmitx2_write_phy(0x19, 0x40);


	// DDR Setting (TF)
	hdmitx2_write_phy(0xff, 0x07); // Page 0x07

	// 以下為DDR Initial (好像跟Default一樣, 但我們還是再設一次)
	hdmitx2_write_phy(0x8d, 0x02);
	hdmitx2_write_phy(0x8e, 0x07);
	hdmitx2_write_phy(0xd7, 0x03);
	hdmitx2_write_phy(0xd8, 0x00);
	hdmitx2_write_phy(0xd9, 0x0e);
	hdmitx2_write_phy(0x8d, 0x02);
	hdmitx2_write_phy(0x8e, 0x07);
	hdmitx2_write_phy(0x93, 0x90);
	hdmitx2_write_phy(0x93, 0xd0);
	hdmitx2_write_phy(0x95, 0x30);
	hdmitx2_write_phy(0x09, 0xf0);
	hdmitx2_write_phy(0xc2, 0x00);
	hdmitx2_write_phy(0xad, 0x1f);
	hdmitx2_write_phy(0xb0, 0x5b);
	hdmitx2_write_phy(0xb7, 0x1b);
	hdmitx2_write_phy(0xb1, 0xbf);
	hdmitx2_write_phy(0xb2, 0x2d);
	hdmitx2_write_phy(0xb3, 0x15);
	hdmitx2_write_phy(0xae, 0x2d);
	hdmitx2_write_phy(0xaf, 0x15);
	hdmitx2_write_phy(0xb4, 0x2d);
	hdmitx2_write_phy(0xb5, 0x15);
	hdmitx2_write_phy(0xbc, 0x2d);
	hdmitx2_write_phy(0xbd, 0x15);
	hdmitx2_write_phy(0xb8, 0x99);
	hdmitx2_write_phy(0xb9, 0x99);
	hdmitx2_write_phy(0xa9, 0xe0);
	hdmitx2_write_phy(0x9e, 0x03);
	hdmitx2_write_phy(0x24, 0xfc);
	hdmitx2_write_phy(0x20, 0x02);
	hdmitx2_write_phy(0x21, 0x02);
	hdmitx2_write_phy(0x22, 0x02);
	hdmitx2_write_phy(0x23, 0x02);
	hdmitx2_write_phy(0x07, 0x00);
	hdmitx2_write_phy(0x08, 0x00);
	hdmitx2_write_phy(0x2a, 0x00);
	hdmitx2_write_phy(0x2b, 0x00);
	hdmitx2_write_phy(0x2c, 0x00);
	hdmitx2_write_phy(0x2d, 0x00);
	hdmitx2_write_phy(0x14, 0x00);
	hdmitx2_write_phy(0x16, 0x00);
	hdmitx2_write_phy(0x03, 0x01);
	hdmitx2_write_phy(0x19, 0x00);
	hdmitx2_write_phy(0x51, 0x09);

	hdmitx2_write_phy(0x01, 0x00);
	hdmitx2_write_phy(0x15, 0x43);
	hdmitx2_write_phy(0x51, 0x80);
	hdmitx2_write_phy(0x19, 0x70);

	// 有Clock Path沒設定到
	hdmitx2_write_phy(0x15, 0x63);
	hdmitx2_write_phy(0xd2, 0x40);


	hdmitx2_write_phy(0x38, 0x18);
	hdmitx2_write_phy(0x39, 0x18);
	hdmitx2_write_phy(0x40, 0x1A);
	hdmitx2_write_phy(0x41, 0x1A);
	hdmitx2_write_phy(0x42, 0x1A);
	hdmitx2_write_phy(0x43, 0x1A);
	hdmitx2_write_phy(0x44, 0x1A);
	hdmitx2_write_phy(0x45, 0x1A);
	hdmitx2_write_phy(0x46, 0x1C);
	hdmitx2_write_phy(0x47, 0x1C);
	hdmitx2_write_phy(0x48, 0x1A);
	hdmitx2_write_phy(0x53, 0x06);
	hdmitx2_write_phy(0x54, 0x1A);
	hdmitx2_write_phy(0x55, 0x09);
	hdmitx2_write_phy(0x56, 0x88);
	hdmitx2_write_phy(0x57, 0x1A);
	hdmitx2_write_phy(0x58, 0x06);
	hdmitx2_write_phy(0x59, 0x1A);
	hdmitx2_write_phy(0x5A, 0x1A);
	hdmitx2_write_phy(0x5B, 0x09);
	hdmitx2_write_phy(0x64, 0x71);
	hdmitx2_write_phy(0x65, 0x04);
	hdmitx2_write_phy(0x66, 0x00);
	hdmitx2_write_phy(0x67, 0x70);
	hdmitx2_write_phy(0x68, 0x71);
	hdmitx2_write_phy(0x80, 0x1A);
	hdmitx2_write_phy(0x81, 0x1A);
	hdmitx2_write_phy(0x82, 0x1A);
	hdmitx2_write_phy(0x83, 0x1C);
	hdmitx2_write_phy(0x83, 0x1C);
	hdmitx2_write_phy(0x85, 0x99);
	hdmitx2_write_phy(0x86, 0x99);
	hdmitx2_write_phy(0x87, 0x99);
	hdmitx2_write_phy(0x88, 0x99);
	hdmitx2_write_phy(0x89, 0x00);
	hdmitx2_write_phy(0x8A, 0x00);
	hdmitx2_write_phy(0x8B, 0x00);
	hdmitx2_write_phy(0x8C, 0x00);


	// USB Path要再選, 但不影響HDMI TX, 故列上
	hdmitx2_write_phy(0xff, 0x00); // Page 0x00
	hdmitx2_write_phy(0x6f, 0x03);
	hdmitx2_write_phy(0xff, 0x04); // Page 0x04
	hdmitx2_write_phy(0x30, 0x50); // CHIP_SCAN_MODE = 1
	hdmitx2_write_phy(0xff, 0x06); // Page 0x06
	hdmitx2_write_phy(0x30, 0x50); // CHIP_SCAN_MODE = 1

	// PINMUX to HDMI TX
	hdmitx2_write_phy(0xfe, 0x07);

#if 1
	hdmitx2_write_phy(0xff, 0x00);//page 0

	// when using internal clock bit[1] shall be cleared.
	hdmitx2_write_phy(0x5f, 0x03);

	hdmitx2_write_phy(0xff, 0x09);//page 9

	// important notice: bit[5] is TCLK sample edge inverse. bit[1] is external clock
	hdmitx2_write_phy(0x00, 0x22);

	hdmitx2_write_phy(0x05, 0x05);//LVCK
	hdmitx2_write_phy(0x06, 0x20);//LPF_CAP2_EN, Default
	hdmitx2_write_phy(0x09, 0x43);//ALL Power
	hdmitx2_write_phy(0x0e, 0x01);//OE
	hdmitx2_write_phy(0x0c, 0x10);//VL
	hdmitx2_write_phy(0x0a, 0x00);//reg_rg, Default

	//sramble off  hdmi 1.4: Default off. use hdmi set_config driver to enable.
	hdmitx2_write_phy(0x30, 0x00);
	//sramble on  but divide by 4 off
	//hdmitx2_write_phy(0x30, 0x04);
	//sramble on  but divide by 4 on
	//hdmitx2_write_phy(0x30, 0x06);

	hdmitx2_write_phy(0x05, 0x05);//deep color

#elif 1
	//
	// HDMI PHY internal pattern gen 74.25MHz
	// no need to provide anything including clock is using phy internal pll
	//

	// HDMI TX
	hdmitx2_write_phy(0xff, 0x00);// p // Page 0
	// when using external clock bit[1] shall be set.
	hdmitx2_write_phy(0x5f, 0x01);// p // HDMI TX HI_RSTN
	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x05, 0x05);// p // LVCK
	hdmitx2_write_phy(0x09, 0x43);// p // ALL Power
	hdmitx2_write_phy(0x0e, 0x01);// p // OE
	hdmitx2_write_phy(0x0c, 0x00);// p // VL
	hdmitx2_write_phy(0x0a, 0x00);// p // reg_rg
	hdmitx2_write_phy(0x30, 0x00);// p // HDMI 1.4
	//s c0 w 30 06 p // HDMI 2.0
	hdmitx2_write_phy(0x05, 0x05);// p // Deep Color

	hdmitx2_write_phy(0x0e, 0x01);// p // 27M

	// MPLL Check
	hdmitx2_write_phy(0xff, 0x00);// p // Page 0
	hdmitx2_write_phy(0x05, 0x01);// p // MPLL Page B
	hdmitx2_write_phy(0xff, 0x01);// p // Page 1


#if 1
	// 74.25M
	hdmitx2_write_phy(0x76, 0x60);// p //
	hdmitx2_write_phy(0x75, 0x00);// p //
	hdmitx2_write_phy(0x77, 0x0C);// p //

	// TG
	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x15, 0x72);// p
	hdmitx2_write_phy(0x16, 0x06);// p
	hdmitx2_write_phy(0x17, 0x28);// p
	hdmitx2_write_phy(0x18, 0x00);// p
	hdmitx2_write_phy(0x19, 0x04);// p
	hdmitx2_write_phy(0x1a, 0x01);// p
	hdmitx2_write_phy(0x1b, 0x00);// p
	hdmitx2_write_phy(0x1c, 0x05);// p
	hdmitx2_write_phy(0x1d, 0xee);// p
	hdmitx2_write_phy(0x1e, 0x02);// p
	hdmitx2_write_phy(0x1f, 0x05);// p
	hdmitx2_write_phy(0x20, 0x00);// p
	hdmitx2_write_phy(0x21, 0x19);// p
	hdmitx2_write_phy(0x22, 0x00);// p
	hdmitx2_write_phy(0x23, 0xd0);// p
	hdmitx2_write_phy(0x24, 0x02);// p
	hdmitx2_write_phy(0x11, 0x1a);// p
	hdmitx2_write_phy(0x10, 0x09);// p
#else
	// 4KP30

	// 74.25M
	//hdmitx2_write_phy(0x76 ,0x60);// p //
	//hdmitx2_write_phy(0x75 ,0x00);// p //
	//hdmitx2_write_phy(0x77 ,0x0C);// p //

	// 27M
	hdmitx2_write_phy(0x76, 0x80);// p //
	hdmitx2_write_phy(0x75, 0x00);// p //
	hdmitx2_write_phy(0x77, 0x04);// p //

	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x15, 0x7C);// p
	hdmitx2_write_phy(0x16, 0x15);// p
	hdmitx2_write_phy(0x17, 0x58);// p
	hdmitx2_write_phy(0x18, 0x00);// p
	hdmitx2_write_phy(0x19, 0x80);// p
	hdmitx2_write_phy(0x1a, 0x01);// p
	hdmitx2_write_phy(0x1b, 0xFE);// p
	hdmitx2_write_phy(0x1c, 0x0F);// 0xFFE = 4094
	hdmitx2_write_phy(0x1d, 0xca);// p
	hdmitx2_write_phy(0x1e, 0x08);// p
	hdmitx2_write_phy(0x1f, 0x0a);// p
	hdmitx2_write_phy(0x20, 0x00);// p
	hdmitx2_write_phy(0x21, 0x52);// p
	hdmitx2_write_phy(0x22, 0x00);// p
	hdmitx2_write_phy(0x23, 0x70);// p
	hdmitx2_write_phy(0x24, 0x08);// p
	hdmitx2_write_phy(0x11, 0x10);// p
	hdmitx2_write_phy(0x10, 0x09);// p
#endif

	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x0c, 0x10);// TRIM to 0x10


	/*
	    important Notice: Below is using HDMI TC72670 PHY embedded patgen with embedded PLL 74.25MHz.
	    Below is the option of using embedded patgen but using external clock.
	    When FPGA stage, we can use this to verify our pixel clock quality.
	*/
#if 0
	//use external clock
	hdmitx2_write_phy(0xff, 0x00);
	hdmitx2_write_phy(0x5f, 0x03);// p // HDMI TX HI_RSTN
	hdmitx2_write_phy(0xff, 0x09);
	hdmitx2_write_phy(0x00, 0x02);// external clock
#endif

#elif 1
	// HDMI TX
	hdmitx2_write_phy(0xff, 0x00);// p // Page 0
	hdmitx2_write_phy(0x5f, 0x01);// p // HDMI TX HI_RSTN
	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x05, 0x05);// p // LVCK
	hdmitx2_write_phy(0x06, 0x20);
	hdmitx2_write_phy(0x09, 0x43);// p // ALL Power
	hdmitx2_write_phy(0x0e, 0x01);// p // OE
	hdmitx2_write_phy(0x0c, 0x00);// p // VL
	hdmitx2_write_phy(0x0a, 0x00);// p // reg_rg

	// HDMI 1.4 / 2.0 select
	hdmitx2_write_phy(0x30, 0x00);// p // HDMI 1.4
	//hdmitx2_write_phy(0x30 ,0x04);// p //sramble on   hdmi 2.0  bit[1]: Divide by 4.  bit[2]: Scramble EN


	hdmitx2_write_phy(0x05, 0x05);// p // Deep Color
	//s c0 w 0e 01 p // 148.5M
	hdmitx2_write_phy(0x0e, 0x15);// p // 297M

	// MPLL Check
	hdmitx2_write_phy(0xff, 0x00);// p // Page 0
	hdmitx2_write_phy(0x05, 0x01);// p // MPLL Page B
	hdmitx2_write_phy(0xff, 0x01);// p // Page 1

	// 594M
	//hdmitx2_write_phy(0x0e ,0x3d);// p // 297M
	//hdmitx2_write_phy(0x76 ,0x00);// p //
	//hdmitx2_write_phy(0x75 ,0x00);// p //
	//hdmitx2_write_phy(0x77 ,0x63);// p //

	// 297M
	hdmitx2_write_phy(0x76, 0x80);// p //
	hdmitx2_write_phy(0x75, 0x00);// p //
	hdmitx2_write_phy(0x77, 0x31);// p //

	// 74.25M
	//hdmitx2_write_phy(0x76 ,0x60);// p //
	//hdmitx2_write_phy(0x75 ,0x00);// p //
	//hdmitx2_write_phy(0x77 ,0x0C);// p //

	// 148.5M
	//hdmitx2_write_phy(0x76 ,0xc0);// p //
	//hdmitx2_write_phy(0x75 ,0x00);// p //
	//hdmitx2_write_phy(0x77 ,0x18);// p //

	// HDMI TX 297MHz/594MHz
	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x15, 0x7C);// p
	hdmitx2_write_phy(0x16, 0x15);// p
	hdmitx2_write_phy(0x17, 0x58);// p
	hdmitx2_write_phy(0x18, 0x00);// p
	hdmitx2_write_phy(0x19, 0x80);// p
	hdmitx2_write_phy(0x1a, 0x01);// p
	hdmitx2_write_phy(0x1b, 0x00);// p
	hdmitx2_write_phy(0x1c, 0x10);// 0x1000 = 4096
	hdmitx2_write_phy(0x1d, 0xca);// p
	hdmitx2_write_phy(0x1e, 0x08);// p
	hdmitx2_write_phy(0x1f, 0x0a);// p
	hdmitx2_write_phy(0x20, 0x00);// p
	hdmitx2_write_phy(0x21, 0x52);// p
	hdmitx2_write_phy(0x22, 0x00);// p
	hdmitx2_write_phy(0x23, 0x70);// p
	hdmitx2_write_phy(0x24, 0x08);// p
	hdmitx2_write_phy(0x11, 0x1A);// p
	hdmitx2_write_phy(0x10, 0x09);// p

	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x0c, 0x10);// TRIM to 0x10
#elif 1

	// HDMI TX
	hdmitx2_write_phy(0xff, 0x00);// p // Page 0
	hdmitx2_write_phy(0x5f, 0x01);// p // HDMI TX HI_RSTN

	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x05, 0x05);// p // LVCK
	hdmitx2_write_phy(0x06, 0x22);
	hdmitx2_write_phy(0x09, 0x43);// p // ALL Power
	hdmitx2_write_phy(0x0e, 0x01);// p // OE
	hdmitx2_write_phy(0x0c, 0x00);// p // VL
	hdmitx2_write_phy(0x0a, 0x02);// p // reg_rg
	//s c0 w 30 00 p // HDMI 1.4
	hdmitx2_write_phy(0x30, 0x00);// p // HDMI 1.4
	//hdmitx2_write_phy(0x30 ,0x06);// p // HDMI 2.0
	hdmitx2_write_phy(0x05, 0x05);// p // Deep Color

	//s c0 w 0e 01 p // 148.5M
	//s c0 w 0e 15 p // 297M
	hdmitx2_write_phy(0x0e, 0x1d);// p // 594M

	// MPLL Check
	hdmitx2_write_phy(0xff, 0x00);// p // Page 0
	hdmitx2_write_phy(0x05, 0x01);// p // MPLL Page B
	hdmitx2_write_phy(0xff, 0x01);// p // Page 1

	// 594M
	hdmitx2_write_phy(0x76, 0x00);// p //
	hdmitx2_write_phy(0x75, 0x00);// p //
	hdmitx2_write_phy(0x77, 0x63);// p //

	// HDMI TX 297MHz/594MHz
	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x15, 0x30);// p
	hdmitx2_write_phy(0x16, 0x11);// p
	hdmitx2_write_phy(0x17, 0x58);// p
	hdmitx2_write_phy(0x18, 0x00);// p
	hdmitx2_write_phy(0x19, 0x80);// p
	hdmitx2_write_phy(0x1a, 0x01);// p
	hdmitx2_write_phy(0x1b, 0x00);// p
	hdmitx2_write_phy(0x1c, 0x0f);// p
	hdmitx2_write_phy(0x1d, 0xca);// p
	hdmitx2_write_phy(0x1e, 0x08);// p
	hdmitx2_write_phy(0x1f, 0x0a);// p
	hdmitx2_write_phy(0x20, 0x00);// p
	hdmitx2_write_phy(0x21, 0x52);// p
	hdmitx2_write_phy(0x22, 0x00);// p
	hdmitx2_write_phy(0x23, 0x70);// p
	hdmitx2_write_phy(0x24, 0x08);// p
	hdmitx2_write_phy(0x11, 0x18);// p
	hdmitx2_write_phy(0x10, 0x09);// p

	hdmitx2_write_phy(0xff, 0x09);// p // Page 9
	hdmitx2_write_phy(0x0c, 0x10);// TRIM to 0x10

#endif
#endif
	DBG_DUMP("Init PHY END\r\n");
#endif

}

#endif

/*
    Wait HDMI2 interrupt event flag

    Wait HDMI2 interrupt event flag

    @param[in] waiting_int   OR of waited interrupt flag ID

    @return OR of waited interrupt flag ID
*/
HDMI_INT_TYPE hdmitx2_wait_flag(HDMI_INT_TYPE waiting_int)
{
	FLGPTN  ui_flag;
	UINT32  int_sts;

	while (1) {
		wai_flg(&ui_flag, FLG_ID_HDMI2, FLGPTN_HDMI2, TWF_ORW | TWF_CLR);
		int_sts = hdmi2_int_sts;
		if (int_sts & waiting_int) {
			hdmi2_int_sts &= ~waiting_int;
			return (int_sts & waiting_int);
		}
	}
}

/*
    Reset hdmi Tx module

    Reset hdmi Tx module's analog & digital logic to default state

    @param[in] b_en     Reset hdmi Tx module
       -@b TRUE: reset
       -@b FALSE: unreset

    @return void
*/
static void hdmitx2_reset(BOOL b_en)
{
	T_HDMI_SYSCTRL_REG reg_sys_ctrl;

	reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);

	if (b_en) {
		reg_sys_ctrl.bit.TXD_RST      = 1;
		reg_sys_ctrl.bit.TXPHY_RST    = 1;
		reg_sys_ctrl.bit.TXFIFO_EN    = 0;
	} else {
		reg_sys_ctrl.bit.TXD_RST      = 0;
		reg_sys_ctrl.bit.TXPHY_RST    = 0;
		reg_sys_ctrl.bit.TXFIFO_EN    = 1;
	}

	HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);

}

/*
    Set HDMI2 TMDS Output Enable

    Set HDMI2 TMDS Output Enable. Disable TMDS output can save power during HDMI2 power down.

    @param[in] b_en
     - @b TRUE:  Enable.
     - @b FALSE: Disable

    @return void
*/
void hdmitx2_set_tmds_oe(BOOL b_en)
{

}

/*
    Set HDMI2 controller's Power ON/OFF

    Enable HDMI2 Power Down would also Disable TMDS Output Enable in this API.

    @param[in] b_pd
     - @b TRUE:   Set HDMI2 Power OFF
     - @b FALSE:  Set HDMI2 Power ON

    @return void
*/
static void hdmitx2_set_power_down(BOOL b_pd)
{
	
// 530 new power down flow todo.
#if _FPGA_EMULATION_
	//hdmitx2_write_phy(0xff, 0x09);// page 9
#endif

#if !_EMULATION_
	UINT32 value;
	if (b_pd) {
		hdmitx2_write_phy(0x7A, 0x00);
		hdmitx2_write_phy(0x9A, 0x00);
		hdmitx2_write_phy(0x9B, 0x00);
		hdmitx2_write_phy(0x9C, 0x00);
		hdmitx2_write_phy(0x7B, 0x00);
		hdmitx2_write_phy(0x98, 0x00);
		hdmitx2_write_phy(0x99, 0x00);

		hdmitx2_write_phy(0x7C, 0x00);
		hdmitx2_write_phy(0x7D, 0x00);
		hdmitx2_write_phy(0x7E, 0x00);
		hdmitx2_write_phy(0x7F, 0x00);

		hdmitx2_read_phy(0x08,&value);
		hdmitx2_write_phy(0x08, value&0xFFFFFFF7);

		hdmitx2_read_phy(0x05,&value);
		hdmitx2_write_phy(0x05, value&0xFFFFFFFE);

		hdmitx2_read_phy(0x04,&value);
		hdmitx2_write_phy(0x04, value&0xFFFFFFFE);		

		hdmitx2_read_phy(0x09,&value);
		hdmitx2_write_phy(0x09, value&0xFFFFFFFC);
	} else {
		hdmitx2_write_phy(0x09, 0x87);
	}
#endif
}

/*
    Set HDMI2 Interrupt Enable/Disable

    Set HDMI2 Interrupt Enable/Disable

    @param[in] int_type  Interrupt ID to be ENABLE/DISABLE
    @param[in] enable
     - @b TRUE:  ENABLE interrupt
     - @b FALSE: DISABLE interrupt

    @return void
*/
void hdmitx2_set_int_en(HDMI_INT_TYPE int_type, BOOL enable)
{
	T_HDMI_INTEN_REG    reg_int_en;
	//unsigned long      flags;

	loc_cpu();
	reg_int_en.reg = HDMI2_GETREG(HDMI_INTEN_REG_OFS);

	if (enable == TRUE) {
		reg_int_en.reg |= int_type;
	} else {
		reg_int_en.reg &= ~int_type;
	}
	HDMI2_SETREG(HDMI_INTEN_REG_OFS, reg_int_en.reg);
	unl_cpu();

	return;
}

void _hdmitx2_enableclk_platform(int id){
	CG_SETREG(0x80,CG_GETREG(0x80)|0x100);
}

void _hdmitx2_disableclk_platform(int id){
	CG_SETREG(0x80,CG_GETREG(0x80)&0xFFFFFEFF);
}


/*
    HDMI Pad contorl 

    This api is used to pull-none pad HPD gpio to get correct status from pin
*/
static void hdmitx2_hpd_pad_control(void)
{
 	//PAD_SETREG(0x10,PAD_GETREG(0x10)&0xFCFFFFFF);
	DELAY_MS(5);
}


/*
    HDMI2 Driver Attach

    This api would open DDC's pinmux and then enable HDMI2's source clock.
*/
static void hdmitx2_attach(void)
{
	_hdmitx2_enableclk_platform(0);
}

/*
    HDMI2 Driver Detach

    This api would close DDC's pinmux and disable HDMI2's source clock.
*/
static void hdmitx2_detach(void)
{
	_hdmitx2_disableclk_platform(0);
}

/*
    Enable HDMI2 InfoFrames Transmission.
*/
void hdmitx2_enable_infofrm_tx(HDMIINFO_EN  info_en, HDMIINFO_RPT info_rpt_en)
{
	T_HDMI_PKTCTRL_REG reg_pkt_ctrl;

	reg_pkt_ctrl.reg = HDMI2_GETREG(HDMI_PKTCTRL_REG_OFS);
	reg_pkt_ctrl.reg |= (info_en | info_rpt_en);
	HDMI2_SETREG(HDMI_PKTCTRL_REG_OFS, reg_pkt_ctrl.reg);
}

/*
    Disable HDMI2 InfoFrames Transmission.
*/
void hdmitx2_disable_infofrm_tx(HDMIINFO_EN  info_dis, HDMIINFO_RPT info_rpt_dis)
{
	T_HDMI_PKTCTRL_REG reg_pkt_ctrl;

	reg_pkt_ctrl.reg = HDMI2_GETREG(HDMI_PKTCTRL_REG_OFS);
	reg_pkt_ctrl.reg &= ~(info_dis | info_rpt_dis);
	HDMI2_SETREG(HDMI_PKTCTRL_REG_OFS, reg_pkt_ctrl.reg);
}

/*
    Setup HDMI2 AUDIO's configurations
*/
static UINT32 hdmitx2_config_audio(HDMI_AUDIOFMT ADOFMT)
{
	T_HDMI_AUDCTRL_REG  reg_aud_ctrl;
	T_HDMI_PCMHDR0_REG  reg_phdr0;
	T_HDMI_PCMHDR1_REG  reg_phdr1;
	T_HDMI_INFO0_REG    reg_info0;
	T_HDMI_INFO1_REG    reg_info1;
	T_HDMI_INFO2_REG    reg_info2;
	T_HDMI_INFO3_REG    reg_info3;

	reg_aud_ctrl.reg = HDMI2_GETREG(HDMI_AUDCTRL_REG_OFS);
	reg_aud_ctrl.bit.CTS_SEL = 0;// Use HW-CTS Value

	reg_phdr0.reg    = HDMI2_GETREG(HDMI_PCMHDR0_REG_OFS);

	switch (ADOFMT) {
	case HDMI_AUDIO48KHZ: {
			//N = 6144 = 0x1800
			reg_aud_ctrl.bit.N_SVAL = HDMI_NVAL6144;
			reg_phdr0.bit.FS = HDMI_PCM_FS48KHZ;
			DBG_WRN("AUD_SR      = [48Khz]\r\n");
			break;
		}
	case HDMI_AUDIO44_1KHZ: {
			//N = 6272 = 0x1880
			reg_aud_ctrl.bit.N_SVAL = HDMI_NVAL6272;
			reg_phdr0.bit.FS = HDMI_PCM_FS44_1KHZ;
			DBG_WRN("AUD_SR      = [44.1Khz]\r\n");
			break;
		}
	case HDMI_AUDIO32KHZ: {
			//N = 4096 = 0x1000
			reg_aud_ctrl.bit.N_SVAL = HDMI_NVAL4096;
			reg_phdr0.bit.FS = HDMI_PCM_FS32KHZ;
			DBG_WRN("AUD_SR      = [32Khz]\r\n");
			break;
		}
	default: {
			DBG_WRN("Not HDMI support Audio mode\r\n");
			return 1;
		}

	}
	HDMI2_SETREG(HDMI_AUDCTRL_REG_OFS, reg_aud_ctrl.reg);
	HDMI2_SETREG(HDMI_PCMHDR0_REG_OFS, reg_phdr0.reg);


	// Set PCM Header: Audio 16bits per sample
	reg_phdr1.reg = HDMI2_GETREG(HDMI_PCMHDR1_REG_OFS);
	reg_phdr1.bit.MAXLEN  = 0;//MaxLen = 20bits
	reg_phdr1.bit.WORDLEN = 1;//16bits
	HDMI2_SETREG(HDMI_PCMHDR1_REG_OFS, reg_phdr1.reg);


	//
	//  Audio infoframe Configuration
	//

	/* Header Word */
	reg_info0.reg = 0;//HDMI2_GETREG(HDMI_AUDINFO_REG_OFS+HDMI_INFO0_REG_OFS);
	reg_info0.bit.TYPE = HDMIINFO_AUD_TYPE;
	reg_info0.bit.VERS = HDMIINFO_AUD_VER;
	reg_info0.bit.LEN  = HDMIINFO_AUD_LEN;

	reg_info0.bit.CHSUM  = (HDMIINFO_AUD_TYPE + HDMIINFO_AUD_VER + HDMIINFO_AUD_LEN + HDMIINFO_AUD_D1_CHCNT_2CH);
	reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
	HDMI2_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

	/* Data Word 1*/
	reg_info1.reg = 0;
	reg_info1.bit.BYTE1 = HDMIINFO_AUD_D1_CHCNT_2CH;// Channel Cnt = 2ch;  CodingType = Refer to Streaming Header
	HDMI2_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

	/* Data Word 2*/
	reg_info2.reg = 0;
	HDMI2_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);

	/* Data Word 3*/
	reg_info3.reg = 0;
	HDMI2_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO3_REG_OFS, reg_info3.reg);

	return 0;
}

static UINT32 hdmitx2_get_picture_aspect(HDMI_VIDEOID vid_code)
{
	UINT32 pic_aspect_ratio = HDMIINFO_AVI_D1_SAMEASPIC_ASPECTRATIO;

	switch (vid_code) {		
	case HDMI_720X480P60_16X9:
	case HDMI_1280X720P60:      /* VID4     1280X720P   @59.94/60Hz */
	case HDMI_1920X1080I60:
	case HDMI_720X480I60_16X9:
	case HDMI_720X240P60_16X9:	
	case HDMI_1440X480I60_16X9:	
	case HDMI_1440X240P60_16X9:	
	case HDMI_1440X480P60_16X9:	
	case HDMI_1920X1080P60:     /* VID16    1920X1080P  @59.94/60Hz */
	case HDMI_720X576P50_16X9:	
	case HDMI_1280X720P50:
	case HDMI_1920X1080I50:	
	case HDMI_720X576I50_16X9:
	case HDMI_720X288P50_16X9:	
	case HDMI_1440X576I50_16X9:	
	case HDMI_1440X288P50_16X9:	
	case HDMI_1440X576P50_16X9:
	case HDMI_1920X1080P50:	
	case HDMI_1920X1080P24:	
	case HDMI_1920X1080P25:
	case HDMI_1920X1080P30:     /* VID34    1920X1080P  @29.97/30Hz */
	case HDMI_1920X1080I50_VT1250:
	case HDMI_1920X1080I100:	
	case HDMI_1280X720P100:
	case HDMI_720X576P100_16X9:	
	case HDMI_720X576I100_16X9:
	case HDMI_1920X1080I120:
	case HDMI_1280X720P120:
	case HDMI_720X480P120_16X9:
	case HDMI_720X480I120_16X9:
	case HDMI_720X576P200_16X9:	
	case HDMI_720X576I200_16X9:	
	case HDMI_720X480P240_16X9:	
	case HDMI_720X480I240_16X9:	
	/*CEA861-D end, HDMI 14B*/	
		
	case HDMI_2560X1440P60:
	case HDMI_1920X1200P60:
	case HDMI_3840X2160P24:
	case HDMI_3840X2160P25:
	case HDMI_3840X2160P30:
	case HDMI_2560X1440P30:
	case HDMI_3840X2160P50:
	case HDMI_3840X2160P60:
	case HDMI_4096X2160P24:
	case HDMI_4096X2160P30:
	case HDMI_4096X2160P50:
	case HDMI_4096X2160P60:
		pic_aspect_ratio = HDMIINFO_AVI_16_9_SAMEASPIC_ASPECTRATIO;
		break;
	default:
		break;
	}
	return pic_aspect_ratio;
}


/*
    Setup HDMI VIDEO's configurations
*/
static UINT32 hdmitx2_config_video(HDMI_VIDEOID vid_code)
{
	T_HDMI_TMDSCTRL0_REG    reg_tmds_ctrl0;
	T_HDMI_INFO0_REG        reg_info0;
	T_HDMI_INFO1_REG        reg_info1;
	T_HDMI_INFO2_REG        reg_info2;
	T_HDMI_INFO3_REG        reg_info3;
	T_HDMI_INFO4_REG        reg_info4;
	UINT8                   under_scan, pr = 0x0;
	HDMI_INTF_MODE          mode_fmt,mode_tmp;
	UINT32					pic_aspect_ratio;
	char 					*mode_name[] = {
		"RGB888",
		"YCbCr422",
		"YCbCr444",
		"YCbCr420",
	};


	
	DBG_WRN("vid_code    = [%d]\r\n",vid_code);
	//
	// Handle the pixel replication
	//
	reg_tmds_ctrl0.reg = HDMI2_GETREG(HDMI_TMDSCTRL0_REG_OFS);
	switch (vid_code) {
#if 1//!_FPGA_EMULATION_
	/* Pixel Repeat Once */
	case HDMI_720X480I60:      /* VID6     720X480i   @59.94/60Hz */
	case HDMI_720X480I60_16X9: /* VID7     720X480i   @59.94/60Hz */
	case HDMI_720X240P60:      /* VID8     720X240p   @59.94/60Hz */
	case HDMI_720X240P60_16X9: /* VID9     720X240p   @59.94/60Hz */
	case HDMI_1440X480I60:     /* VID10    1440X480I  @59.94/60Hz */
	case HDMI_1440X480I60_16X9:/* VID11    1440X480I  @59.94/60Hz */
	case HDMI_1440X240P60:     /* VID12    1440X240P  @59.94/60Hz */
	case HDMI_1440X240P60_16X9:/* VID13    1440X240P  @59.94/60Hz */
	case HDMI_720X576I50:      /* VID21    720X576I   @50Hz       */
	case HDMI_720X576I50_16X9: /* VID22    720X576I   @50Hz       */
	case HDMI_720X288P50:      /* VID23    720X288P   @50Hz       */
	case HDMI_720X288P50_16X9: /* VID24    720X288P   @50Hz       */
	case HDMI_1440X576I50:     /* VID25    1440X576I  @50Hz       */
	case HDMI_1440X576I50_16X9:/* VID26    1440X576I  @50Hz       */
	case HDMI_1440X288P50:     /* VID27    1440X288P  @50Hz       */
	case HDMI_1440X288P50_16X9:/* VID28    1440X288P  @50Hz       */
	case HDMI_720X576I100:     /* VID44    720X576I   @100Hz      */
	case HDMI_720X576I100_16X9:/* VID45    720X576I   @100Hz      */
	case HDMI_720X480I120:     /* VID50    720X480I   @120Hz      */
	case HDMI_720X480I120_16X9:/* VID51    720X480I   @120Hz      */
	case HDMI_720X576I200:     /* VID54    720X576I   @200Hz      */
	case HDMI_720X576I200_16X9:/* VID55    720X576I   @200Hz      */
	case HDMI_720X480I240:     /* VID58    720X480I   @240Hz      */
	case HDMI_720X480I240_16X9: { /* VID59    720X480I   @240Hz      */
			//reg_tmds_ctrl0.bit.TCLKSEL = HDMI_TCLK_2X;
#if _FPGA_EMULATION_
			//hdmitx2_write_phy(0xff, 0x09);// page 9
#endif
#if !_EMULATION_  /*need check todo*/
			//hdmitx2_write_phy(0x05, 0x09);// tclk x2
#endif
			pr = 0x1;
			break;
		}
#endif

	/* Pixel Repeat None */
	default: {
			reg_tmds_ctrl0.bit.TCLKSEL = HDMI_TCLK_1X;
#if _FPGA_EMULATION_
			//hdmitx2_write_phy(0xff, 0x09);// page 9
#endif
#if !_EMULATION_ /*need check todo*/
			//hdmitx2_write_phy(0x05, 0x05);// tclk x1
#endif
			pr = 0x0;
			break;
		}

	}
	//reg_tmds_ctrl0.bit.STERM       = 1;
	HDMI2_SETREG(HDMI_TMDSCTRL0_REG_OFS, reg_tmds_ctrl0.reg);

	hdmitx2_disable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
	pic_aspect_ratio = hdmitx2_get_picture_aspect(vid_code);

	switch (vid_code) { /*extended VIC need to Send Vendor Specific Infoframe*/
		
	case HDMI_3840X2160P24:     /* VID93    3840x2160P  @24Hz*/
	case HDMI_3840X2160P25:         /* VID94 */
	case HDMI_3840X2160P30:         /* VID95 */
	case HDMI_4096X2160P24:         /* VID98 */
	case HDMI_4096X2160P25:         /* VID99 */
	case HDMI_4096X2160P30: 		/* VID100*/
	case HDMI14B_3840X2160P24:
	case HDMI14B_3840X2160P25:
	case HDMI14B_3840X2160P30:
	//coverity[unterminated_case]
	case HDMI14B_4096X2160P24:
		{
			T_HDMI_INFO0_REG reg_info0;
			T_HDMI_INFO1_REG reg_info1;
			T_HDMI_INFO2_REG reg_info2;


			/* Header Word */
			reg_info0.reg = 0;
			reg_info0.bit.TYPE = HDMIINFO_VSI_TYPE;
			reg_info0.bit.VERS = HDMIINFO_VSI_VER;
			reg_info0.bit.LEN  = 0x05;

			/* Data Word 1*/
			reg_info1.reg = 0;
			reg_info1.bit.BYTE1 = HDMIINFO_VSI_DB1;
			reg_info1.bit.BYTE2 = HDMIINFO_VSI_DB2;
			reg_info1.bit.BYTE3 = HDMIINFO_VSI_DB3;
			reg_info1.bit.BYTE4 = HDMIINFO_VSI_EXTEND_RESOLUTION;
			HDMI2_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

			/* Data Word 2*/
			reg_info2.reg = 0;

			if (vid_code == HDMI14B_3840X2160P30 || vid_code == HDMI_3840X2160P30)
				reg_info2.bit.BYTE5 = 1;
			else if (vid_code == HDMI14B_3840X2160P25 || vid_code == HDMI_3840X2160P25)
				reg_info2.bit.BYTE5 = 2;
			else if (vid_code == HDMI14B_3840X2160P24 || vid_code == HDMI_3840X2160P24)
				reg_info2.bit.BYTE5 = 3;
			else if (vid_code == HDMI14B_4096X2160P24 || vid_code == HDMI_4096X2160P24)
				reg_info2.bit.BYTE5 = 4;


			reg_info2.bit.BYTE6 = 0x00;
			reg_info2.bit.BYTE7 = 0x00;
			reg_info2.bit.BYTE8 = 0x00;
			HDMI2_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);


			reg_info0.bit.CHSUM  = (UINT8)(HDMIINFO_VSI_TYPE + HDMIINFO_VSI_VER + reg_info0.bit.LEN + HDMIINFO_VSI_DB1 + HDMIINFO_VSI_DB2 + HDMIINFO_VSI_DB3 + 0x20 + reg_info2.bit.BYTE5 + reg_info2.bit.BYTE6);
			reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
			HDMI2_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

			hdmitx2_enable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
			}break;
		
		default:
			break;
	}


	switch (vid_code) {
		
	case HDMI_640X480P60:       /* VID1     640X480P    @59.94/60Hz */
	case HDMI_720X480P60_16X9:  /* VID2     720X480P    @59.94/60Hz */
	case HDMI_720X480P60:       /* VID3     720X480P    @59.94/60Hz */
	case HDMI_1280X720P60:      /* VID4     1280X720P   @59.94/60Hz */
	case HDMI_1280X720P30:      /* VID132   1280X720P   @30Hz */
	case HDMI_1280X720P25:
	case HDMI_1920X1080I60:     /* VID5     1920X1080I  @59.94/60Hz */
	case HDMI_720X480I60:       /* VID6     720X480i    @59.94/60Hz */
	case HDMI_720X480I60_16X9:  /* VID7     720X480i    @59.94/60Hz */
	case HDMI_720X240P60:       /* VID8     720X240p    @59.94/60Hz */
	case HDMI_720X240P60_16X9:  /* VID9     720X240p    @59.94/60Hz */
	case HDMI_1440X480I60:      /* VID10    1440X480I   @59.94/60Hz */
	case HDMI_1440X480I60_16X9: /* VID11    1440X480I   @59.94/60Hz */
	case HDMI_1440X240P60:      /* VID12    1440X240P   @59.94/60Hz */
	case HDMI_1440X240P60_16X9: /* VID13    1440X240P   @59.94/60Hz */
	case HDMI_1440X480P60:      /* VID14    1440X480P   @59.94/60Hz */
	case HDMI_1440X480P60_16X9: /* VID15    1440X480P   @59.94/60Hz */
	case HDMI_1920X1080P60:     /* VID16    1920X1080P  @59.94/60Hz */
	case HDMI_720X576P50:       /* VID17    720X576P    @50Hz       */
	case HDMI_720X576P50_16X9:  /* VID18    720X576P    @50Hz       */
	case HDMI_1280X720P50:      /* VID19    1280X720P   @50Hz       */
	case HDMI_1920X1080I50:     /* VID20    1920X1080i  @50Hz       */
	case HDMI_720X576I50:       /* VID21    720X576I    @50Hz       */
	case HDMI_720X576I50_16X9:  /* VID22    720X576I    @50Hz       */
	case HDMI_720X288P50:       /* VID23    720X288P    @50Hz       */
	case HDMI_720X288P50_16X9:  /* VID24    720X288P    @50Hz       */
	case HDMI_1440X576I50:      /* VID25    1440X576I   @50Hz       */
	case HDMI_1440X576I50_16X9: /* VID26    1440X576I   @50Hz       */
	case HDMI_1440X288P50:      /* VID27    1440X288P   @50Hz       */
	case HDMI_1440X288P50_16X9: /* VID28    1440X288P   @50Hz       */
	case HDMI_1440X576P50:      /* VID29    1440X576P   @50Hz       */
	case HDMI_1440X576P50_16X9: /* VID30    1440X576P   @50Hz       */
	case HDMI_1920X1080P50:     /* VID31    1920X1080P  @50Hz       */
	case HDMI_1920X1080P24:     /* VID32    1920X1080P  @23.98/24Hz */
	case HDMI_1920X1080P25:     /* VID33    1920X1080P  @25Hz       */
	case HDMI_1920X1080P30:     /* VID34    1920X1080P  @29.97/30Hz */
	case HDMI_1920X1080I50_VT1250:/*VID39   1920X1080I  @50Hz       */
	case HDMI_1920X1080I100:    /* VID40    1920X1080I  @100Hz      */
	case HDMI_1280X720P100:     /* VID41    1280X720P   @100Hz      */
	case HDMI_720X576P100:      /* VID42    720X576P    @100Hz      */
	case HDMI_720X576P100_16X9: /* VID43    720X576P    @100Hz      */
	case HDMI_720X576I100:      /* VID44    720X576I    @100Hz      */
	case HDMI_720X576I100_16X9: /* VID45    720X576I    @100Hz      */
	case HDMI_1920X1080I120:    /* VID46    1920X1080I  @119.8/120Hz*/
	case HDMI_1280X720P120:     /* VID47    1280X720P   @119.8/120Hz*/
	case HDMI_720X480P120:      /* VID48    720X480P    @119.8/120Hz*/
	case HDMI_720X480P120_16X9: /* VID49    720X480P    @119.8/120Hz*/
	case HDMI_720X480I120:      /* VID50    720X480I    @119.8/120Hz*/
	case HDMI_720X480I120_16X9: /* VID51    720X480I    @119.8/120Hz*/
	case HDMI_720X576P200:      /* VID52    720X576P    @200Hz      */
	case HDMI_720X576P200_16X9: /* VID53    720X576P    @200Hz      */
	case HDMI_720X576I200:      /* VID54    720X576I    @200Hz      */
	case HDMI_720X576I200_16X9: /* VID55    720X576I    @200Hz      */
	case HDMI_720X480P240:      /* VID56    720X480P    @239.7/240Hz*/
	case HDMI_720X480P240_16X9: /* VID57    720X480P    @239.7/240Hz*/
	case HDMI_720X480I240:      /* VID58    720X480I    @239.7/240Hz*/
	case HDMI_720X480I240_16X9: /* VID59    720X480I    @239.7/240Hz*/

	case HDMI_1024X768P60:
	case HDMI_1280X1024P60:
	case HDMI_1600X1200P60:
	case HDMI_2560X1440P60:

	case HDMI_1680X720P60:

	case HDMI_3840X2160P24:     /* VID93    3840x2160P  @24Hz*/
	case HDMI_3840X2160P25:         /* VID94 */
	case HDMI_3840X2160P30:         /* VID95 */
	case HDMI_3840X2160P50:         /* VID96 */
	case HDMI_3840X2160P60:         /* VID97 */
	case HDMI_4096X2160P24:         /* VID98 */
	case HDMI_4096X2160P25:         /* VID99 */
	case HDMI_4096X2160P30:         /* VID100*/
	case HDMI_4096X2160P50:         /* VID101*/
	case HDMI_4096X2160P60: {       /* VID102*/

			if ((hdmitx2_get_edid_info() & HDMITX_TV_UNDERSCAN) || (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_UNDERSCAN])) {
				// Support under_scan
				under_scan = 0x2;
			} else if (hdmitx2_get_edid_info()) {
				// Support OverScan
				under_scan = 0x1;
			} else {
				// EDID Parsing Fail, Use NoData
				under_scan = 0x0;
			}


			// Output Format RGB444 or YUV444 depends on EDID
			if ((hdmitx2_get_edid_info()&HDMITX_TV_YCBCR444) && (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_RGB] == 0)) {
				mode_fmt = (HDMI_INTF_YCBCR444 << 5);
			} else if ((hdmitx2_get_edid_info()&HDMITX_TV_YCBCR422) && (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_RGB] == 0)) {
				mode_fmt = (HDMI_INTF_YCBCR422 << 5);
			} else {
				mode_fmt = (HDMI_INTF_RGB444  << 5);
			}

			if (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] & 0x1) {
				mode_fmt = (HDMI_INTF_YCBCR444 << 5);
			} else if (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] & 0x2) {
				mode_fmt = (HDMI_INTF_YCBCR422 << 5);
			} else if (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] & 0x4) {
				mode_fmt = (HDMI_INTF_YCBCR420 << 5);
			}


			mode_tmp = mode_fmt >> 5;
	
			DBG_WRN("AVI_mode    = [%s]\r\n",mode_name[mode_tmp]);

		
			//
			//  AVI InfoFrame Configuration
			//

			/* Header Word */
			reg_info0.reg = 0;
			reg_info0.bit.TYPE = HDMIINFO_AVI_TYPE;
			reg_info0.bit.VERS = HDMIINFO_AVI_VER;
			reg_info0.bit.LEN  = HDMIINFO_AVI_LEN;

			if (vid_code == HDMI_3840X2160P24 ||
				vid_code == HDMI_3840X2160P25||
				vid_code == HDMI_3840X2160P30 ||
				vid_code == HDMI_1024X768P60 ||
				vid_code == HDMI_1280X1024P60 ||
				vid_code == HDMI_1600X1200P60 ||
				vid_code == HDMI_2560X1440P60 ||
				vid_code == HDMI_1680X720P60) {
				vid_code = 0;
			}


			//reg_info0.bit.CHSUM  = (HDMIINFO_AVI_TYPE + HDMIINFO_AVI_VER + HDMIINFO_AVI_LEN + under_scan + mode_fmt + pic_aspect_ratio + (vid_code&0x7F) + pr);
			reg_info0.bit.CHSUM  = (HDMIINFO_AVI_TYPE + HDMIINFO_AVI_VER + HDMIINFO_AVI_LEN + HDMI_COLORIMETRY_ITUBT709 + HDMI_AFI_PRESENT + HDMIINFO_AVI_YCC_FULL_RANGE + under_scan + mode_fmt + pic_aspect_ratio + vid_code + pr);
			reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
			HDMI2_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

			/* Data Word 1*/
			reg_info1.reg = 0;
			reg_info1.bit.BYTE1 = under_scan + mode_fmt + HDMI_AFI_PRESENT;
			reg_info1.bit.BYTE2 = pic_aspect_ratio + HDMI_COLORIMETRY_ITUBT709;
			reg_info1.bit.BYTE3 = 0x00;
			reg_info1.bit.BYTE4 = vid_code&0x7F;
			HDMI2_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

			/* Data Word 2*/
			reg_info2.reg = 0;
			reg_info2.bit.BYTE5 = pr + HDMIINFO_AVI_YCC_FULL_RANGE;
			HDMI2_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);

			/* Data Word 3*/
			reg_info3.reg = 0;
			HDMI2_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO3_REG_OFS, reg_info3.reg);

			/* Data Word 4*/
			reg_info4.reg = 0;
			HDMI2_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO4_REG_OFS, reg_info4.reg);

			break;
		}
	default:
		DBG_ERR("No Support VID number!\r\n");
		return 1;
	}

	return 0;

}



static void hdmitx2_scdc_dump(void)
{
	UINT32 i, data = 0;

	DBG_DUMP("      0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F");
	for (i = 0; i < 256; i++) {
		if ((i & 0xF) == 0x0) {
			DBG_DUMP("\r\n0x%02X: ", i);
		}

		if(hdmitx2_scdc_read(i, &data) == E_OK){
			DBG_DUMP("0x%02X  ", data);
		}
	}
	DBG_DUMP("\r\n");
}

#if HDMITX_SCDC_READREQUEST
static void hdmitx2_scdc_rr_callback(UINT32 ui_event)
{
#if 1
	{
		UINT8 UPDATE_0 = 0, UPDATE_1 = 0;

		i2c5_get_data(&UPDATE_0, &UPDATE_1);

		DBG_WRN("0x%08X UPDATE0=0x%02X  UPDATE1=0x%02X\r\n", ui_event, UPDATE_0, UPDATE_1);
	}
#else
	// STOP CONDITION TEST
	{
		UINT32 data1, data2;

		while (hdmitx2_scdc_read(0x10, &data1) != E_OK) {
			DBG_DUMP("Q");
		}

		if (data1) {
			while (hdmitx2_scdc_write(0x10, data1) != E_OK) {
				DBG_DUMP("W");
			}
		}

		while (hdmitx2_scdc_read(0x10, &data2) != E_OK) {
			DBG_DUMP("E");
		}

		DBG_WRN("0x%08X R10=0x%02X  0x%02X\r\n", ui_event, data1, data2);

	}
#endif

#if !HDMITX_SCDC_RRTEST
	hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_EN,        ENABLE);
#endif

}
#endif

#if 1

/**
    Open HDMI2 Transmitter Driver

   Open HDMI2 Transmitter and then intialize the source clock / DDC / interrupt for usage.
   Besides, this would also read the TV/Display's EDID information from DDC channel and parsing the EDID.
   The user can get the EDID information after hdmitx2_open() by the api hdmitx2_get_edid_info().
   The driver is opened or not can be checked by the hdmitx2_is_opened.
   The behavior of the re-open of the HDMI2 driver in under save protection in the driver design.\n
   In the system design, the DDC channel is using the I2C2 controller in normal operation.
   The user must remember opening the I2C2 pinmux in the systemInit phase.

    @return
     - @b E_OK: Done and success
*/
ER hdmitx2_open(void)
{
#if _FPGA_EMULATION_
	T_HDMI_TMDSCTRL1_REG    reg_tmds_ctrl1;
#endif
	T_HDMI_SYSCTRL_REG      reg_sys_ctrl;

#if 1//defined(__FREERTOS)
	DBG_WRN("[uboot]hdmitx2_open: version [%s]\r\n", HDMI_VERSION);
#endif	



	if (!hdmi2_init) {
		hdmi2_init = TRUE;
		hdmitx2_hpd_pad_control();
		hdmi2_hpd_status = hdmitx2_check_hotplug();
#if defined(__FREERTOS)
		hdmitx2_create_resource();
#endif
	}


	if (b_hdmi2_tx_open == TRUE) {
		return E_OK;
	}


#if (defined __UITRON || defined __ECOS)
	pll_disable_system_reset(HDMI2_RSTN);
#endif
#if defined(__FREERTOS)/*todo check*/
	//pll_enable_bus_clock(LCD310_CLK);
	//pll_enable_bus_clock(HDMI2_CLK);
	//pll_enable_apb_clock(HDMI2_PCLK);
   // pll_disable_apb_reset(HDMI2_PRSTN);
#endif  

#if defined(__FREERTOS)
	// clear the interrupt flag
	clr_flg(FLG_ID_HDMI2, FLGPTN_HDMI2);
	//request_irq(INT_ID_HDMI2, hdmi2_isr, IRQF_TRIGGER_HIGH, "hdmi", 0);//INT_ID_HDMI2//94forHDMI2
#endif 

	hdmitx2_attach();
	

#if _FPGA_EMULATION_
	hdmitx2_reset_sil9002();
#endif

#if !_EMULATION_
	hdmitx2_reset(TRUE);
#endif
	hdmitx2_reset(FALSE);
	
	//hdmitx2_init_phy();


	// Enable Digital Logic to Validate interrupt status
	//hdmitx2_set_power_down(FALSE);

	hdmi2_trim_rsel = HDMITX_RTRIM_DEFAULT;

	
#if 0//!_FPGA_EMULATION_
	// TC72670B HDMI PHY Bug. The RSEN would couple clock signal and keep toggling.
	// The root cause the connect to wrong power source.
	// So we disable RSEN interrupt when using TC72670B
	hdmitx2_set_int_en(RSEN | ACR_OVR,  ENABLE);
#endif

	hdmitx2_enable_infofrm_tx(0, HDMIINFO_RPT_AVI | HDMIINFO_RPT_AUD | HDMIINFO_RPT_GCP | HDMIINFO_RPT_GEN1 | HDMIINFO_RPT_GEN2);

	
	//
	//  Enable HDMI2 interrupt after Tx starts
	//
	hdmi2_int_sts = 0;
	drv_enable_int(DRV_INT_HDMI2);
	
#if 1//!(HDMITX_DEBUG && HDMITX_DEBUG_SKIP_EDID)
	if (b_hdmi2_i2c_opened == FALSE) {
		if (hdmi2_i2c_obj[hdmi2_i2c_select].open(&hdmi2_i2c_ses) == E_OK) {
			b_hdmi2_i2c_opened = TRUE;
		} else {
			b_hdmi2_i2c_opened = FALSE;
		}
	}
#endif
	hdmitx2_init_ddc(b_hdmi2_i2c_opened);
	
	hdmitx2_set_config(HDMI_CONFIG_ID_3D_FMT, HDMI3D_NONE);
	hdmitx2_set_config(HDMI_CONFIG_ID_AV_MUTE, FALSE);

#if _FPGA_EMULATION_
	// PHY Default Config Values
	reg_tmds_ctrl1.reg = HDMI2_GETREG(HDMI_TMDSCTRL1_REG_OFS);
	reg_tmds_ctrl1.bit.SAFE_START = 0x1D0;//default value is 0x1D0
	HDMI2_SETREG(HDMI_TMDSCTRL1_REG_OFS, reg_tmds_ctrl1.reg);
#endif

	// Default as HDMI 2.0
	reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
	reg_sys_ctrl.bit.HDMI2      = 1;
	HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
	
	hdmi2_state |= HDMI_OPEN;
	b_hdmi2_tx_open = TRUE;
	return E_OK;
}

/**
    Check the HDMI2-Tx driver is opened or not

    Check the HDMI2-Tx driver is opened or not

    @return
     - @b TRUE:  Already opened.
     - @b FALSE: Have not opened yet.
*/
BOOL hdmitx2_is_opened(void)
{
	return b_hdmi2_tx_open;
}

/**
    Close the HDMI2-Tx module

    Close the HDMI2-Tx module, this api would power down the hdmi-tx module and also close the I2C2(DDC) usage.

    @return void
     - @b E_OK: Done and success
*/
ER  hdmitx2_close(void)
{
	if (b_hdmi2_tx_open == FALSE) {
		return E_OK;
	}

	drv_disable_int(DRV_INT_HDMI2);

	// Close HDMI2 Digital Clock & DDC-Pinmux
	hdmitx2_detach();

	// Power Down HDMI2 PHY Logic
	hdmitx2_set_power_down(TRUE);

	hdmitx2_set_int_en(HDMI_INTALL, DISABLE);

	// Disable HDMI2 Audio IN/OUT Stream
	hdmitx2_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, FALSE);
	hdmitx2_set_tmds_oe(DISABLE);
	hdmitx2_set_config(HDMI_CONFIG_ID_3D_FMT, HDMI3D_NONE);

	
#if defined(__FREERTOS) /*todo check*/
	//pll_disable_bus_clock(HDMI_CLK);
	//pll_disable_apb_clock(HDMI2_PCLK);
    //pll_enable_apb_reset(HDMI2_PRSTN);

	free_irq(INT_ID_HDMI2, 0);
	hdmitx2_release_resource();
#endif    


	// Signal semaphore
	//	SEM_SIGNAL(SEMID_HDMI2);

	if (b_hdmi2_i2c_opened) {
		if (hdmi2_i2c_obj[hdmi2_i2c_select].close(hdmi2_i2c_ses) == E_OK) {
			b_hdmi2_i2c_opened = FALSE;
		} else {
			DBG_ERR("DDC close fail\r\n");
		}
	}

	hdmi2_state &= ~(HDMI_OPEN);
	b_hdmi2_tx_open = FALSE;
	hdmi2_init = FALSE;
	return E_OK;
}


/**
    Check hotplug or not

    This is used to check if the HDMI2 cable is plugged or not.

    @return
     - @b TRUE:  Hotplug attached.
     - @b FALSE: Hotplug detached.
*/
BOOL hdmitx2_check_hotplug(void)
{
	T_HDMI_SYSCTRL_REG  reg_sys_ctrl;
	
	reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
	
	return reg_sys_ctrl.bit.HPD;
}

/**
    Check if HDMI2's video Clock is stable or not

    This api is used in the HDMI2 display device object to check if the hdmi input video clock is stable or not after
    opening the IDE Pixel clock. If the none of the video clock or the clock is not stable, this api would output the
    warning message "HDMI2 Video Clock Not Stable! Timeout!!" on console.

    @return void
*/
void hdmitx2_check_video_stable(void)
{
	T_HDMI_SYSCTRL_REG  reg_sys_ctrl;
	UINT32              count;

	count = HDMITX_CHECK_TIMEOUT;
	do {
		count--;
		reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
		DBG_WRN("HDMI2 Clk polling!\r\n");
	} while ((reg_sys_ctrl.bit.PIXCLK_STABLE == 0x0) && count);

	if (count == 0) {
		DBG_WRN("HDMI2 Clk No Stable!Timeout!\r\n");
	}
}
#endif

/**
    Set HDMI2 functionality configuration

    This is used to configure the main functions of the HDMI2 controller, including the Video/Audio formats, output-enable,
    video-source, 3D-format, ...etc. Please reference to the "HDMI2_CONFIG_ID" and the application note for the control function details.

    @param[in] config_id     Control function selection ID.
    @param[in] ui_config     Please reference to the "HDMI_CONFIG_ID" and the application note for the control function parameter details.

    @return void
*/
void hdmitx2_set_config(HDMI_CONFIG_ID config_id, UINT32 ui_config)
{
#if HDMITX_SCDC_SUPPORT	
	int timeout = 20;
#endif


	/* Would Not Blocking by the b_hdmi2_tx_open */
	switch (config_id) {
	case HDMI_CONFIG_ID_EDIDMSG: {
			b_hdmi2_edid_msg = ui_config;
		}
		return;

	case HDMI_CONFIG_ID_AUDIO_MUTE: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.AUD_MUTE = (ui_config > 0);
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		return;

	case HDMI_CONFIG_ID_SOURCE: {
#if (defined __UITRON || defined __ECOS)//#ifndef __KERNEL__
			T_HDMI_SYSCTRL_REG reg_ctrl;

			reg_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_ctrl.bit.SRC_SEL = ui_config;
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_ctrl.reg);
#else
			T_HDMI_TOP_REG reg_ctrl;

			reg_ctrl.reg = HDMI2_GETREG(HDMI_TOP_REG_OFS);
			reg_ctrl.bit.LCD_SRC_SEL = ui_config;
			HDMI2_SETREG(HDMI_TOP_REG_OFS, reg_ctrl.reg);
#endif
		}
		return;


	/* Control Options */
	case HDMI_CONFIG_ID_FORCE_RGB: {
			hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_RGB]        = (ui_config > 0);
		}
		return;

	case HDMI_CONFIG_ID_FORCE_UNDERSCAN: {
			hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_UNDERSCAN]  = (ui_config > 0);
		}
		return;

	case HDMI_CONFIG_ID_VERIFY_VIDEOID: {
			hdmi2_ctrl_option[HDMICFG_OPTION_VERIFY_VID]       = (ui_config > 0);
		}
		return;

	case HDMI_CONFIG_ID_HWDDC: {
			#ifdef __KERNEL__
			hdmi2_i2c_select = 0;
			#else
			hdmi2_i2c_select = !ui_config;
			#endif
		}
		return;

#if (defined __UITRON || defined __ECOS)//#ifndef __KERNEL__

	case HDMI_CONFIG_ID_SWDDC_SCL: {
			hdmi_sw_ddc_scl  = ui_config;
		}
		return;

	case HDMI_CONFIG_ID_SWDDC_SDA: {
			hdmi_sw_ddc_sda  = ui_config;
		}
		return;
#endif

	case HDMI_CONFIG_ID_TRIM_RSEL: {
			// This config is only valid for emulation.
			// In normal mode, hdmi2_trim_rsel should comes from eFuse.
#if _EMULATION_
			hdmi2_trim_rsel = ui_config;

			if (b_hdmi2_tx_open) {
				// Set Calibrated resistor value
				//hdmitx2_validate_rtrim();
			}
#endif
		}
		return;

	case HDMI_CONFIG_ID_FORCE_YUV444: {
			if(ui_config > 0)
				hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] |= 0x1;
			else
				hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] &= ~0x1;
		}
		return;

	case HDMI_CONFIG_ID_FORCE_YUV422: {
			if(ui_config > 0)
				hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] |= 0x2;
			else
				hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] &= ~0x2;
		}
		return;

	case HDMI_CONFIG_ID_FORCE_YUV420: {
			if(ui_config > 0)
				hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] |= 0x4;
			else
				hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] &= ~0x4;
		}
		return;

	case HDMI_CONFIG_ID_TIMING_FACTOR_EN: {
			if(ui_config > 0)
				hdmi2_ctrl_option[HDMICFG_OPTION_TIMING_DIV_1P001] = 0x1;
			else
				hdmi2_ctrl_option[HDMICFG_OPTION_TIMING_DIV_1P001] = 0x0;		
		}
		return;		

	default: {
			break;
		}

	}



	if (b_hdmi2_tx_open == FALSE) {
		return;
	}

	switch (config_id) {
	case HDMI_CONFIG_ID_OE: {
			T_HDMI_CTRL_REG         reg_ctrl;

			if (ui_config) {
				// If EDID Read fail, default act as HDMI2 device.
				reg_ctrl.reg = HDMI2_GETREG(HDMI_CTRL_REG_OFS);
#if !HDMITX_DEBUG_FORCE_DVI
				if ((hdmitx2_get_edid_info()&HDMITX_TV_HDMI) || (hdmitx2_get_edid_info() == 0x0)) {
					reg_ctrl.bit.HDMI_MODE = 1;//HDMI Mode
				} else
#endif
				{
					reg_ctrl.bit.HDMI_MODE = 0;//DVI Mode
				}

				HDMI2_SETREG(HDMI_CTRL_REG_OFS, reg_ctrl.reg);
				hdmitx2_enable_infofrm_tx(HDMIINFO_EN_AVI | HDMIINFO_EN_AUD | HDMIINFO_EN_GCP, HDMIINFO_RPT_AVI | HDMIINFO_RPT_AUD | HDMIINFO_RPT_GCP);
			}

			hdmitx2_set_tmds_oe((ui_config > 0));
		}
		break;

	case HDMI_CONFIG_ID_SCDC_ENABLE: {
		
#if HDMITX_SCDC_SUPPORT		
			if (ui_config > 0) {
				// SCDC Cap
				if (hdmitx2_get_hdmi_2_vsdb_cap() & HDMI2VSDB_SCDC_PRESENT) {
					UINT32 data;

					if (hdmitx2_scdc_read(0x01, &data) != E_OK) {
						DBG_ERR("Read SCDC Sink Version failed\r\n");
						return;
					}

					if (data != HDMI2_SCDC_VERSION) {
						DBG_ERR("Sink's SCDC version not matched.(%d)\r\n", data);
					} else {
						DBG_WRN("SCDC Enable!\r\n");
					}

					if (hdmitx2_scdc_write(0x02, HDMI2_SCDC_VERSION) != E_OK) {
						DBG_ERR("Write SCDC Src Version failed\r\n");
						return;
					}

					if(hdmi2_clock_rate > 340000000){ //when TMDS > 340Mhz need 1:40 rate and scramble on
						if(hdmitx2_scdc_write(0x20, 0x03) != E_OK){
							DBG_ERR("Write SCDC scramble, tmds 1:40 enable fail\r\n");
							return;
						}
						hdmitx2_write_phy(0x34, 0x02);
						hdmitx2_write_phy(0x30, 0x06);/*1:40 tmds rate & scramble on*/
						DBG_WRN("Write SCDC scramble, tmds 1:40 enable success\r\n");	

						do {
							DELAY_MS(5);
							if (hdmitx2_scdc_read(0x21, &data) != E_OK) {
	                            DBG_ERR("Read SCDC scramble sts , no support SCDC deviced\r\n");
	                            return;
	                        }
	                        if (data & 0x1) {
		                        break;
	                        }

						} while(timeout--);

						if(timeout == 0){
							DBG_ERR("Read SCDC scramble sts timeout\r\n");
							return;
						}
					
					} else {
						if(hdmitx2_scdc_write(0x20, 0x00) != E_OK){
							DBG_WRN("Write SCDC scramble, tmds 1:40 disable fail\r\n");
							return;
						}
						hdmitx2_write_phy(0x34, 0x00);
						hdmitx2_write_phy(0x30, 0x00); /*1:40 tmds rate & scramble dis*/
						DBG_WRN("Write SCDC scramble, tmds 1:40 disable success\r\n");
					}


					do {
						DELAY_MS(5);
						if (hdmitx2_scdc_read(0x40, &data) != E_OK) {
							DBG_ERR("Read SCDC clock detect sts , no support SCDC deviced\r\n");
							return;
						}
						if (data & 0x1) {
							break;
						}

					} while(timeout--);

					if(timeout == 0){
						DBG_ERR("Read SCDC clock detect sts timeout\r\n");
						return;
					}
					


#if HDMITX_SCDC_READREQUEST
					// RR Cap
					if (hdmitx2_get_hdmi_2_vsdb_cap() & HDMI2VSDB_RR_CAP) {
						// Set SCDC RR Enable
						hdmitx2_scdc_write(0x30, 0x01);

						hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_SAR,       HDMI2_SCDC_SLAVEADDR >> 1);
						hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_MODE,      I2C_RR_MODE_READ_UPDATE_FLAG);
						hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_CALLBACK, (UINT32)hdmitx2_scdc_rr_callback);

#if HDMITX_SCDC_RRTEST
/*
						if (SwTimer_Open(&g_hdmi_sw_timer_id, hdmitx2_read_request_sw_timer_cb) == E_OK) {
							SwTimer_Cfg(g_hdmi_sw_timer_id, 2000, SWTIMER_MODE_FREE_RUN);
							SwTimer_Start(g_hdmi_sw_timer_id);
						}
						DBG_DUMP("RR TEST RR\r\n");
*/
#else
						hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_EN,        ENABLE);
#endif
					}
#endif
				} else {
					DBG_WRN("HDMI2.0 VSDB SCDC not present!\r\n");
				}

			} else {

				hdmitx2_write_phy(0x30, 0x00); /*1:40 tmds rate & scramble dis*/
				
				//hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_EN,                DISABLE);
				//hdmi2_i2c_obj[hdmi2_i2c_select].set_config(hdmi2_i2c_ses, I2C_CONFIG_ID_RR_CALLBACK,          0);
			}
#endif

		}
		break;

	case HDMI_CONFIG_ID_VIDEO: {
			hdmitx2_config_video(ui_config);
		}
		break;

	case HDMI_CONFIG_ID_AUDIO: {
			hdmitx2_config_audio(ui_config);
		}
		break;

	case HDMI_CONFIG_ID_AV_MUTE: {
			T_HDMI_GENCTRL_REG  reg_gcp_ctrl;
			T_HDMI_CTRL_REG     reg_ctrl;
			UINT32              count;

			reg_gcp_ctrl.reg = 0;
			if (ui_config > 0) {
				reg_gcp_ctrl.bit.SETAVM = 1;
			} else {
				reg_gcp_ctrl.bit.CLRAVM = 1;
			}
			HDMI2_SETREG(HDMI_GENCTRL_REG_OFS, reg_gcp_ctrl.reg);


			count = HDMITX_CHECK_TIMEOUT;
			do {
				count--;
				reg_ctrl.reg = HDMI2_GETREG(HDMI_CTRL_REG_OFS);
			} while ((reg_ctrl.bit.GCP_MUTE_STS == reg_gcp_ctrl.bit.CLRAVM) && count);

			if (count == 0) {
				DBG_WRN("HDMI2 MUTE STS!Timeout!\r\n");
			}
		}
		break;

	case HDMI_CONFIG_ID_AUDIO_STREAM_EN: {
			T_HDMI_AUDCTRL_REG  reg_aud_ctrl;

			reg_aud_ctrl.reg = HDMI2_GETREG(HDMI_AUDCTRL_REG_OFS);
			reg_aud_ctrl.bit.AUD_EN = (ui_config > 0);
			HDMI2_SETREG(HDMI_AUDCTRL_REG_OFS, reg_aud_ctrl.reg);
		}
		break;

	case HDMI_CONFIG_ID_3D_FMT: {
			if (ui_config > (UINT32)HDMI3D_TOPANDBOTTOM) {
				break;
			}

			if (ui_config) {
				T_HDMI_INFO0_REG reg_info0;
				T_HDMI_INFO1_REG reg_info1;
				T_HDMI_INFO2_REG reg_info2;

				/* Header Word */
				reg_info0.reg = 0;
				reg_info0.bit.TYPE = HDMIINFO_VSI_TYPE;
				reg_info0.bit.VERS = HDMIINFO_VSI_VER;

				/* Data Word 1*/
				reg_info1.reg = 0;
				reg_info1.bit.BYTE1 = HDMIINFO_VSI_DB1;
				reg_info1.bit.BYTE2 = HDMIINFO_VSI_DB2;
				reg_info1.bit.BYTE3 = HDMIINFO_VSI_DB3;
				reg_info1.bit.BYTE4 = HDMIINFO_VSI_3D_PRESENT;
				HDMI2_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

				/* Data Word 2*/
				reg_info2.reg = 0;
				if (ui_config == (UINT32) HDMI3D_SIDEBYSIDE_HALF) {
					reg_info0.bit.LEN   = 0x06;
					reg_info2.bit.BYTE5 = HDMIINFO_VSI_SIDEBYSIDE_HALF;
					reg_info2.bit.BYTE6 = HDMIINFO_VSI_HORI_SUBSAMPLE;
				} else if (ui_config == (UINT32) HDMI3D_TOPANDBOTTOM) {
					reg_info0.bit.LEN   = 0x05;
					reg_info2.bit.BYTE5 = HDMIINFO_VSI_TOP_N_BOTTOM;
					reg_info2.bit.BYTE6 = 0x00;
				}
				reg_info2.bit.BYTE7 = 0x00;
				reg_info2.bit.BYTE8 = 0x00;
				HDMI2_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);

				reg_info0.bit.CHSUM  = (UINT8)(HDMIINFO_VSI_TYPE + HDMIINFO_VSI_VER + reg_info0.bit.LEN + HDMIINFO_VSI_DB1 + HDMIINFO_VSI_DB2 + HDMIINFO_VSI_DB3 + HDMIINFO_VSI_3D_PRESENT + reg_info2.bit.BYTE5 + reg_info2.bit.BYTE6);
				reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
				HDMI2_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

				hdmitx2_enable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
			} else {
				hdmitx2_disable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
			}
		}
		break;

	case HDMI_CONFIG_ID_SWING_CTRL:
	case HDMI_CONFIG_ID_BGR_CTRL:
		break;

	case HDMI_CONFIG_ID_VSYNC_INVERT: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			if (ui_config > 0) {
				// SSCP counting from VSYNC Rising Edge
				hdmitx2_write_phy(0x35, 0x00);
			} else {
				// SSCP counting from VSYNC Falling Edge
				hdmitx2_write_phy(0x35, 0x01);
			}

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.VD_INV = (ui_config > 0);
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_HSYNC_INVERT: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.HD_INV = (ui_config > 0);
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_COMPONENT0: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.COMP0_SEL = ui_config;
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_COMPONENT1: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.COMP1_SEL = ui_config;
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_COMPONENT2: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.COMP2_SEL = ui_config;
			HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;

	case HDMI_CONFIG_ID_SCRAMBLE_EN: {
			UINT32 value = 0, value2 = 0;

			if (!(hdmitx2_get_hdmi_2_vsdb_cap() & HDMI2VSDB_SCDC_PRESENT)) {
				DBG_ERR("SCDC not support. SCRAMBLE_EN failed\r\n");
				return;
			}
			
			if (hdmitx2_scdc_read(0x20,  &value2) != E_OK){
				DBG_ERR("SCDC read failed\r\n");
				return;
			}

#if _FPGA_EMULATION_
			hdmitx2_write_phy(0xFF, 0x09);
#endif
			hdmitx2_read_phy(0x30,  &value);
#if 1
			if (ui_config) {
#if _FPGA_EMULATION_
				//TC72670B Bug: SSCP Delay count to 255
				hdmitx2_write_phy(0x32, 0xFF);
#else
				hdmitx2_write_phy(0x34, 0x02);//MSb
				hdmitx2_write_phy(0x33, 0x20);//LSb
#endif

				// Enable PHY Scramble
				hdmitx2_write_phy(0x30,  value | 0x04);

				// Enable SCDC Scramble
				hdmitx2_scdc_write(0x20, value2 | 0x01);
			} else {
				// Disable PHY Scramble
				hdmitx2_write_phy(0x30, (value  & (~0x04)));

				// Disable SCDC Scramble
				hdmitx2_scdc_write(0x20, (value2 & (~0x01)));
			}
#endif
		}
		break;
	case HDMI_CONFIG_ID_PIXCLK_DIVIDE4: {
			UINT32 value = 0;

#if _FPGA_EMULATION_
			hdmitx2_write_phy(0xFF, 0x09);//page9
#endif
			hdmitx2_read_phy(0x30,  &value);
#if 0
			if (ui_config) {
				// Enable PHY HDMI2.0 and Divide by 4
				hdmitx2_write_phy(0x30,  value | 0x06);

				// Enable SCDC Scramble and Divide by 4
				hdmitx2_scdc_write(0x20, 0x03);
			} else {
				// Disable PHY HDMI2.0 and Divide by 4
				hdmitx2_write_phy(0x30, (value  & (~0x06)));

				// Disable SCDC Scramble and Divide by 4
				hdmitx2_scdc_write(0x20, 0x00);
			}
#endif
		}
		break;

	case HDMI_CONFIG_ID_SCDC_DUMP: {
			if (hdmitx2_get_hdmi_2_vsdb_cap() & HDMI2VSDB_SCDC_PRESENT) {
				hdmitx2_scdc_dump();
			}
		}
		break;
	case HDMI_CONFIG_ID_SCDC_WRITE: {
			DBG_DUMP("SCDC WRITE R[0x%02X]=0x%02X\r\n", (ui_config >> 8) & 0xFF, ui_config & 0xFF);
			hdmitx2_scdc_write((ui_config >> 8) & 0xFF, ui_config & 0xFF);
		}
		break;

	case HDMI_CONFIG_ID_PHY_INIT: {
			hdmitx2_init_phy();
			hdmitx2_set_power_down(FALSE);
		}
		break;

	case HDMI_CONFIG_ID_THREAD_STOP: {
			hdmi2_state |= HDMI_THREAD_STOP;
		}break;		

	default:
		DBG_WRN("Set cfg ID err\r\n");
		break;

	}

}

/**
    Get HDMI functionality configuration

    Use this api to get the current HDMI2 function configuration.

    @param[in] config_id     Get which Control function configuration selection.

    @return Please reference to the "HDMI_CONFIG_ID" for the control function parameter details.
*/
UINT32 hdmitx2_get_config(HDMI_CONFIG_ID config_id)
{
	UINT32 ret = 0;

	switch (config_id) {
	case HDMI_CONFIG_ID_OE: {
			UINT32 value = 0;

			if (b_hdmi2_tx_open) {
				hdmitx2_read_phy(0x09, &value);
			}
			ret = value & 0x1;
		}
		break;

	case HDMI_CONFIG_ID_VIDEO: {
			T_HDMI_INFO1_REG        reg_info1;

			reg_info1.reg = HDMI2_GETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO1_REG_OFS);
			ret = reg_info1.bit.BYTE4;
		}
		break;

	case HDMI_CONFIG_ID_AUDIO: {
			T_HDMI_PCMHDR0_REG  reg_phdr0;

			reg_phdr0.reg = HDMI2_GETREG(HDMI_PCMHDR0_REG_OFS);

			if (reg_phdr0.bit.FS == HDMI_PCM_FS48KHZ) {
				ret = HDMI_AUDIO48KHZ;
			} else if (reg_phdr0.bit.FS == HDMI_PCM_FS32KHZ) {
				ret = HDMI_AUDIO32KHZ;
			} else {
				ret = HDMI_AUDIO44_1KHZ;
			}
		}
		break;

	case HDMI_CONFIG_ID_AV_MUTE: {
			T_HDMI_CTRL_REG     reg_ctrl;

			reg_ctrl.reg = HDMI2_GETREG(HDMI_CTRL_REG_OFS);
			ret = reg_ctrl.bit.GCP_MUTE_STS;
		}
		break;

	case HDMI_CONFIG_ID_SOURCE: {
#if (defined __UITRON || defined __ECOS)//#ifndef __KERNEL__
			T_HDMI_SYSCTRL_REG reg_ctrl;

			reg_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			ret = reg_ctrl.bit.SRC_SEL;
#else
			T_HDMI_TOP_REG reg_ctrl;

			reg_ctrl.reg = HDMI2_GETREG(HDMI_TOP_REG_OFS);
			ret = reg_ctrl.bit.LCD_SRC_SEL;
#endif

		}
		break;


	/* Control Options */

	case HDMI_CONFIG_ID_FORCE_RGB: {
			ret = hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_RGB];
		}
		break;

	case HDMI_CONFIG_ID_FORCE_UNDERSCAN: {
			ret = hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_UNDERSCAN];
		}
		break;

	case HDMI_CONFIG_ID_VERIFY_VIDEOID: {
			ret = hdmi2_ctrl_option[HDMICFG_OPTION_VERIFY_VID];
		}
		break;

	case HDMI_CONFIG_ID_MANUFACTURER_ID: {
			ret = hdmi2_manufacturer_id;
		}
		break;

	case HDMI_CONFIG_ID_SWING_CTRL: {




		}
		break;

	case HDMI_CONFIG_ID_SEL_BGR: {




		}
		break;

	case HDMI_CONFIG_ID_REG_BGR_EN: {
			//ret = HDMI2_GETREG(HDMI_TTXCFG0D_REG_OFS) & 0x1;
		}
		break;

	case HDMI_CONFIG_ID_BGR_CTRL: {




		}
		break;

	case HDMI_CONFIG_ID_BGR_TEST: {




		}
		break;

	case HDMI_CONFIG_ID_CLKDETECT: {




		}
		break;

	case HDMI_CONFIG_ID_PHY_PDB: {




		}
		break;

	case HDMI_CONFIG_ID_TERM_EN: {




		}
		break;

	case HDMI_CONFIG_ID_BW_CTRL: {




		}
		break;

	case HDMI_CONFIG_ID_CKSW_CTRL: {



		}
		break;

	case HDMI_CONFIG_ID_PHYREG_PD: {
			//T_HDMI_TTXCFG0A_REG      RegTxCfgA;
			//RegTxCfgA.reg    = HDMI2_GETREG(HDMI_TTXCFG0A_REG_OFS);
			//ret = RegTxCfgA.bit.REG_PD;
		}
		break;

	case HDMI_CONFIG_ID_RXSENSE: {
			T_HDMI_SYSCTRL_REG   reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
			ret = reg_sys_ctrl.bit.RSEN;
		}
		break;

	case HDMI_CONFIG_ID_FORCE_YUV444: {
			ret = (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] & 0x1) > 0;
		}
		break;
	case HDMI_CONFIG_ID_FORCE_YUV422: {
			ret = (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] & 0x2) > 0;
		}
		break;
	case HDMI_CONFIG_ID_FORCE_YUV420: {
			ret = (hdmi2_ctrl_option[HDMICFG_OPTION_FORCE_YUV] & 0x4) > 0;
		}
		break;		

	default:
		break;

	}


	return ret;
}

#if 1

void hdmitx2_hotplug_on(void)
{
#if _FPGA_EMULATION_
		T_HDMI_TMDSCTRL1_REG	reg_tmds_ctrl1;
#endif
		T_HDMI_SYSCTRL_REG		reg_sys_ctrl;

			
			
#if !_EMULATION_
	hdmitx2_reset(TRUE);
#endif
	hdmitx2_reset(FALSE);
					
#if 1
	//hdmitx2_set_int_en(RSEN | ACR_OVR,  ENABLE);
#endif
			
	hdmitx2_enable_infofrm_tx(0, HDMIINFO_RPT_AVI | HDMIINFO_RPT_AUD | HDMIINFO_RPT_GCP | HDMIINFO_RPT_GEN1 | HDMIINFO_RPT_GEN2);
			
	//	Enable HDMI interrupt after Tx starts
	hdmi2_int_sts = 0;

				

	if (b_hdmi2_i2c_opened == FALSE) {
		if (hdmi2_i2c_obj[hdmi2_i2c_select].open(&hdmi2_i2c_ses) == E_OK) {
			b_hdmi2_i2c_opened = TRUE;
		} else {
			b_hdmi2_i2c_opened = FALSE;
		}
	}

	hdmitx2_init_ddc(b_hdmi2_i2c_opened);
				
	hdmitx2_set_config(HDMI_CONFIG_ID_3D_FMT, HDMI3D_NONE);
	hdmitx2_set_config(HDMI_CONFIG_ID_AV_MUTE, FALSE);
			
#if _FPGA_EMULATION_
				// PHY Default Config Values
	reg_tmds_ctrl1.reg = HDMI2_GETREG(HDMI_TMDSCTRL1_REG_OFS);
	reg_tmds_ctrl1.bit.SAFE_START = 200;//default value is 0x1D0
	HDMI2_SETREG(HDMI_TMDSCTRL1_REG_OFS, reg_tmds_ctrl1.reg);
#endif
			
	// Default as HDMI 2.0
	reg_sys_ctrl.reg = HDMI2_GETREG(HDMI_SYSCTRL_REG_OFS);
	reg_sys_ctrl.bit.HDMI2		= 1;
	HDMI2_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);

				

	// init HDMI phy after clock ready
	hdmitx2_set_config(HDMI_CONFIG_ID_PHY_INIT, TRUE);

			
	hdmitx2_check_video_stable();
	hdmitx2_set_config(HDMI_CONFIG_ID_OE,			 TRUE);
	hdmitx2_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, TRUE);
	hdmitx2_set_config(HDMI_CONFIG_ID_SCDC_ENABLE,	 ENABLE);
	
}

void hdmitx2_hotplug_off(void)
{
	hdmitx2_set_config(HDMI_CONFIG_ID_OE, FALSE);

	hdmitx2_set_config(HDMI_CONFIG_ID_SCDC_ENABLE,    FALSE);
		
	// Power Down HDMI PHY Logic
	hdmitx2_set_power_down(TRUE);
		
	//hdmitx2_set_int_en(HDMI_INTALL, DISABLE);
		
	// Disable HDMI Audio IN/OUT Stream
	hdmitx2_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, FALSE);
	hdmitx2_set_tmds_oe(DISABLE);
	hdmitx2_set_config(HDMI_CONFIG_ID_3D_FMT, HDMI3D_NONE);
}

#endif

#if _FPGA_EMULATION_
/*
    HDMI2 Pattern Gen Enable
*/
void hdmitx2_set_pg_enable(BOOL b_en)
{
	T_HDMI_PG0_REG  reg_pg0;

	reg_pg0.reg = HDMI2_GETREG(HDMI_PG0_REG_OFS);
	reg_pg0.bit.EN       = b_en;
	reg_pg0.bit.UPDATE   = 1;
	HDMI2_SETREG(HDMI_PG0_REG_OFS, reg_pg0.reg);
}

/*
    HDMI Pattern Gen Test Pattern Selection
*/
void hdmitx2_set_pg_pattern(UINT32 ui_pat_id, UINT32 pxl_rpt_cnt)
{
	T_HDMI_PG0_REG  reg_pg0;

	reg_pg0.reg = HDMI2_GETREG(HDMI_PG0_REG_OFS);
	reg_pg0.bit.PG_TYPE = ui_pat_id;
	reg_pg0.bit.PR_CNT  = pxl_rpt_cnt;
	HDMI2_SETREG(HDMI_PG0_REG_OFS, reg_pg0.reg);
}

/*
    Set HDMI Pattern Gen Register 1
*/
void hdmitx2_set_pg_1(UINT32 gray, UINT32 HTOT)
{
	T_HDMI_PG1_REG  reg_pg1;

	reg_pg1.reg          = 0;
	reg_pg1.bit.GRAY     = gray;
	reg_pg1.bit.SG_HTOT  = HTOT;
	HDMI2_SETREG(HDMI_PG1_REG_OFS, reg_pg1.reg);
}

/*
    Set HDMI Pattern Gen Register 2
*/
void hdmitx2_set_pg_2(UINT32 HSW, UINT32 HAB)
{
	T_HDMI_PG2_REG  reg_pg2;

	reg_pg2.reg          = 0;
	reg_pg2.bit.SG_HSW   = HSW;
	reg_pg2.bit.SG_HAB   = HAB;
	HDMI2_SETREG(HDMI_PG2_REG_OFS, reg_pg2.reg);
}

/*
    Set HDMI Pattern Gen Register 3
*/
void hdmitx2_set_pg_3(UINT32 HAW, UINT32 VTOT)
{
	T_HDMI_PG3_REG  reg_pg3;

	reg_pg3.reg          = 0;
	reg_pg3.bit.SG_HAW   = HAW;
	reg_pg3.bit.SG_VTOT   = VTOT;
	HDMI2_SETREG(HDMI_PG3_REG_OFS, reg_pg3.reg);
}

/*
    Set HDMI Pattern Gen Register 4
*/
void hdmitx2_set_pg_4(UINT32 VSW, UINT32 VAB)
{
	T_HDMI_PG4_REG  reg_pg4;

	reg_pg4.reg          = 0;
	reg_pg4.bit.SG_VSW   = VSW;
	reg_pg4.bit.SG_VAB   = VAB;
	HDMI2_SETREG(HDMI_PG4_REG_OFS, reg_pg4.reg);
}

/*
    Set HDMI Pattern Gen Register 5
*/
void hdmitx2_set_pg_5(UINT32 VAW)
{
	T_HDMI_PG5_REG  reg_pg5;

	reg_pg5.reg          = 0;
	reg_pg5.bit.SG_VAW   = VAW;
	HDMI2_SETREG(HDMI_PG5_REG_OFS, reg_pg5.reg);
}

/*
    Reset FPGA external PHY SIL9002
*/
void hdmitx2_reset_sil9002(void)
{
	T_HDMI_DEBUG_REG    reg_debug;

	reg_debug.reg = HDMI2_GETREG(HDMI_DEBUG_REG_OFS);
	reg_debug.bit.SIL9002_PD       = 0;
	reg_debug.bit.SIL9002_ISEL     = 0;
	HDMI2_SETREG(HDMI_DEBUG_REG_OFS, reg_debug.reg);
	DELAY_MS(10);
	reg_debug.bit.SIL9002_PD       = 1;
	HDMI2_SETREG(HDMI_DEBUG_REG_OFS, reg_debug.reg);
}
#endif

#if 0//def __KERNEL__
EXPORT_SYMBOL(hdmitx2_open);
EXPORT_SYMBOL(hdmitx2_is_opened);
EXPORT_SYMBOL(hdmitx2_close);
EXPORT_SYMBOL(hdmitx2_set_config);
EXPORT_SYMBOL(hdmitx2_get_config);
EXPORT_SYMBOL(hdmitx2_check_video_stable);
EXPORT_SYMBOL(hdmitx2_check_hotplug);
#endif

//VOS_MODULE_VERSION(hdmitx2, 1, 00, 005, 00);



//@}
