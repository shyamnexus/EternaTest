#include "kwrap/type.h"
#include <comm/kdrv_hwcopy.h>
#include "hwcopy_int.h"
//#include "kwrap/debug.h"

INT32 kdrv_hwcopy_open(UINT32 chip, UINT32 engine)
{
        ER ret;

        switch (engine) {
        case KDRV_GFX2D_HWCOPY:
                ret = hwcopy_open(HWCOPY_ID_0);
                break;
        default:
                DBG_ERR("invalid engine 0x%x\r\n", engine);
                return -1;
        }

        if (ret != E_OK) return -1;

        return 0;
}

/*!
 * @fn INT32 kdrv_hwcopy_close(UINT32 chip, UINT32 engine)
 * @brief close hardware engine
 * @param chip      the chip id of hardware
 * @param engine    the engine id of hardware
 *                  - @b KDRV_GFX2D_HWCOPY: hwcopy engine
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_hwcopy_close(UINT32 chip, UINT32 engine)
{
        ER ret;

        switch (engine) {
        case KDRV_GFX2D_HWCOPY:
                ret = hwcopy_close(HWCOPY_ID_0);
                break;
        default:
                DBG_ERR("invalid engine 0x%x\r\n", engine);
                return -1;
        }

        if (ret != E_OK) return -1;

        return 0;
}

/*!
 * @fn INT32 kdrv_hwcopy_trigger(INT32 handler, KDRV_HWCOPY_TRIGGER_PARAM *p_param,
                                 KDRV_CALLBACK_FUNC *p_cb_func,
                                 VOID *p_user_data);
 * @brief trigger hardware engine
 * @param id                    the id of hardware
 * @param p_param               the parameter for trigger
 * @param p_cb_func             the callback function
 * @param p_user_data           the private user data
 * @return return 0 on success, -1 on error
 */
INT32 kdrv_hwcopy_trigger(UINT32 id, KDRV_HWCOPY_TRIGGER_PARAM *p_param,
                                                KDRV_CALLBACK_FUNC *p_cb_func,
                                                VOID *p_user_data)
{
        switch (KDRV_DEV_ID_ENGINE(id)) {
        case KDRV_GFX2D_HWCOPY:
        {
                hwcopy_enqueue(HWCOPY_ID_0, p_param, p_cb_func);
//              ret = hwcopy_trigger(HWCOPY_ID_1, p_request, p_cb_func);
        }
                break;
        default:
                DBG_ERR("invalid id 0x%x\r\n", id);
                return -1;
        }

        return 0;

}

