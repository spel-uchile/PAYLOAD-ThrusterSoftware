/*                                 SUCHAI
 *                      NANOSATELLITE FLIGHT SOFTWARE
 *
 *      Copyright 2018, Carlos Gonzalez Cortes, carlgonz@uchile.cl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmdCOM.h>
#include "cmdTM.h"

static const char *tag = "cmdTM";

void cmd_tm_init(void)
{
    cmd_add("send_status", tm_send_status, "%d", 1);
    cmd_add("tm_parse_status", tm_parse_status, "", 0);
    cmd_add("send_payload", tm_send_pay_data, "%u %d", 2);
}

int tm_send_status(char *fmt, char *params, int nparams)
{
    if(params == NULL)
    {
        LOGE(tag, "params is null!");
        return CMD_ERROR;
    }

    int dest_node;
    //Format: <node>
    if(nparams == sscanf(params, fmt, &dest_node))
    {
        com_data_t data;
        memset(&data, 0, sizeof(data));
        data.node = (uint8_t)dest_node;
        data.frame.frame = 0;
        data.frame.type = TM_TYPE_STATUS;

        // Pack status variables to a structure
        dat_status_t status;
        dat_status_to_struct(&status);
        if(LOG_LEVEL >= LOG_LVL_DEBUG)
            dat_print_status(&status);

        // The total amount of status variables must fit inside a frame
        LOGD(tag, "sizeof(status) = %d", sizeof(status));
        LOGD(tag, "sizeof(data.frame) = %d", sizeof(data.frame));
        LOGD(tag, "sizeof(data.frame.data) = %d", sizeof(data.frame.data));
        assert(sizeof(status) < sizeof(data.frame.data));
        memcpy(data.frame.data.data8, &status, sizeof(status));

        return com_send_data("", (char *)&data, 0);
    }
    else
    {
        LOGW(tag, "Invalid args!");
        return CMD_FAIL;
    }
}

int tm_parse_status(char *fmt, char *params, int nparams)
{
    if(params == NULL)
        return CMD_ERROR;

    dat_status_t *status = (dat_status_t *)params;
    dat_print_status(status);
    return CMD_OK;
}

int tm_send_pay_data(char *fmt, char *params, int nparams)
{
    if(params == NULL)
    {
        LOGE(tag, "params is null!");
        return CMD_ERROR;
    }

    int dest_node;
    uint32_t payload;
    //Format: <node>
    if(nparams == sscanf(params, fmt, &payload, &dest_node))
    {
        com_data_t data;
        memset(&data, 0, sizeof(data));
        data.node = (uint8_t)dest_node;
        data.frame.frame = 0;
        data.frame.type = (uint16_t)(TM_TYPE_PAYLOAD + payload);

        temp_data_t data_temp;
        ads_data_t data_ads;

        switch(payload)
        {
            case temp_sensors:
                dat_get_recent_payload_sample(&data_temp, temp_sensors, 0);
                assert(sizeof(data_temp) < sizeof(data.frame.data));
                LOGI(tag, "data_temp.obc_temp_1: %f", data_temp.obc_temp_1)
                memcpy(data.frame.data.data8, &data_temp, sizeof(data_temp));
                break;
            case ads_sensors:
                dat_get_recent_payload_sample(&data_ads, ads_sensors, 0);
                assert(sizeof(data_ads) < sizeof(data.frame.data));
                LOGI(tag, "data_ads.acc_x %f", data_ads.acc_x)
                memcpy(data.frame.data.data8, &data_ads, sizeof(data_ads));
                break;
            default:
                break;
        }

        return com_send_data("", (char *)&data, 0);
    }
    else
    {
        LOGW(tag, "Invalid args!");
        return CMD_FAIL;
    }
}
