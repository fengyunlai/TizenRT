/******************************************************************************
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2016, Realtek Semiconductor Corporation. All rights reserved. 
  *
******************************************************************************/
#include "wifi_conf.h"
#include "wlan_intf.h"
#include "wifi_constants.h"
#include <platform/platform_stdlib.h>
#include "osdep_service.h"
#include <apps/shell/tash.h>
#ifndef WLAN0_NAME
  #define WLAN0_NAME		"wlan0"
#endif
static void init_thread(void *param)
{
	/* To avoid gcc warnings */
	( void ) param;
#if CONFIG_LWIP_LAYER
	/* Initilaize the LwIP stack */
	if (LwIP_Is_Init() < 0) {
		LwIP_Init_If();
	}
#endif
#if CONFIG_WIFI_IND_USE_THREAD
	wifi_manager_init();
#endif
#if CONFIG_WLAN
	wifi_on(RTW_MODE_STA);
#if CONFIG_AUTO_RECONNECT
	//setup reconnection flag
	wifi_set_autoreconnect(1);
#endif
	printf("\n\r%s(%d), Available heap 0x%x", __FUNCTION__, __LINE__, rtw_getFreeHeapSize());	
#endif

#if CONFIG_INTERACTIVE_MODE
 	/* Initial uart rx swmaphore*/
	vSemaphoreCreateBinary(uart_rx_interrupt_sema);
	xSemaphoreTake(uart_rx_interrupt_sema, 1/portTICK_RATE_MS);
	start_interactive_mode();
#endif	
}

static int ameba_atcmd(int argc, char **args)
{
	static int atcmd_started = 0;
	char buf[64];
	int len;
	int i = 0;
	for (i = 0; i < argc; i++)
		printf("%s: %d %s\r\n", __func__, i, args[i]);
	if (args[1]) {
		if (atcmd_started == 0) {
			if (strcmp(args[1], "start") == 0) {
				init_thread(NULL);
				log_service_init();
				atcmd_started = 1;
			} else {
				printf("Hello, please run \"ameba start\" first\r\n");
			}
			return 0;
		}
		memset(buf, 0, 64);
		len = (strlen(args[1]) <= 64)?(strlen(args[1])):63;
		strncpy(buf, args[1], len);
		if(log_handler((char *)buf) == NULL){
			if(mp_commnad_handler((char *)buf) < 0)                     
			{
				if(print_help_handler((char *)buf) < 0){
					printf("\r\nunknown command '%s'", buf);
				}
			}
		}
	}
	return 0;
}

const static tash_cmdlist_t ameba_at_cmds[] = {
	{"ameba",  ameba_atcmd,   TASH_EXECMD_SYNC},
	{NULL,    NULL,        0}
};

void tash_register_ameba_atcmds(void)
{
	tash_cmdlist_install(ameba_at_cmds);
}

int ameba_main(int argc, char *argv[])
{
	tash_register_ameba_atcmds();
	return 0;
}

