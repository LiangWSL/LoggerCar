#include "GPRSProcess.h"
#include "gprs.h"
#include "gps.h"
#include "rt.h"

#include "osConfig.h"
#include "MainProcess.h"

/*******************************************************************************
 *
 */
void GPRSPROCESS_Task(void)
{
	osEvent signal;
	GPRS_ModuleStatusEnum moduleStatus = MODULE_INVALID;		/* GPRS模块状态 */
	char* expectString;						/* 预期收到的字符串 */

	GPRS_StructTypedef sendStruct;			/* 发送结构 */
	GPS_LocateTypedef location;
	RT_TimeTypedef *etime;

	uint8_t moduleTimeoutCnt;				/* 模块超时计数 */
	uint16_t curPatchPack;					/* 本次上传条数 */

	GPRS_Init();
	/* 初始化发送结构体 */
	GPRS_StructInit(&sendStruct);

	while(1)
	{
		/* 发送部分 */
		switch (moduleStatus)
		{
		/* 如果模块无效，则先执行开机 */
		case MODULE_INVALID:
			printf("模块开机\r\n");
			/* 开机 */
			GPRS_PWR_CTRL_ENABLE();
			expectString = AT_CMD_POWER_ON_READY_RESPOND;
			moduleStatus = MODULE_VALID;
			break;

		/* 设置波特率 */
		case SET_BAUD_RATE:
			printf("设置波特率\r\n");
			GPRS_SendCmd(AT_CMD_SET_BAUD_RATE);
			expectString = AT_CMD_SET_BAUD_RATE_RESPOND;
			moduleStatus = SET_BAUD_RATE_FINISH;
			break;

		/* 查询SIM卡状态 */
		case CHECK_SIM_STATUS:
			printf("查询sim卡状态\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_SIM_STATUS);
			expectString = AT_CMD_CHECK_SIM_STATUS_RESPOND;
			moduleStatus = CHECK_SIM_STATUS_FINISH;
			break;

		/* 查找网络状态 */
		case SEARCH_NET_STATUS:
			printf("查找网络\r\n");
			GPRS_SendCmd(AT_CMD_SEARCH_NET_STATUS);
			expectString = AT_CMD_SEARCH_NET_STATUS_RESPOND;
			moduleStatus = SEARCH_NET_STATUS_FINISH;
			break;

		/* 查找GPRS状态 */
		case CHECK_GPRS_STATUS:
			printf("查找GPRS状态\r\n");
			GPRS_SendCmd(AT_CMD_CHECK_GPRS_STATUS);
			expectString = AT_CMD_CHECK_GPRS_STATUS_RESPOND;
			moduleStatus = CHECK_GPRS_STATUS_FINISH;
			break;

		/* 设置单连接方式 */
		case SET_SINGLE_LINK:
			printf("设置单连方式\r\n");
			GPRS_SendCmd(AT_CMD_SET_SINGLE_LINK);
			expectString = AT_CMD_SET_SINGLE_LINK_RESPOND;
			moduleStatus = SET_SINGLE_LINK_FINISH;
			break;

		/* 设置为透传模式 */
		case SET_SERIANET_MODE:
			printf("设置透传模式\r\n");
			GPRS_SendCmd(AT_CMD_SET_SERIANET_MODE);
			expectString = AT_CMD_SET_SERIANET_MODE_RESPOND;
			moduleStatus = SET_SERIANET_MODE_FINISH;
			break;

		/* 设置APN名称 */
		case SET_APN_NAME:
			printf("设置APN名称\r\n");
			GPRS_SendCmd(AT_CMD_SET_APN_NAME);
			expectString = AT_CMD_SET_APN_NAME_RESPOND;
			moduleStatus = SET_APN_NAME_FINISH;
			break;

		/* 激活PDP场景 */
		case ACTIVE_PDP:
			printf("激活PDP场景\r\n");
			GPRS_SendCmd(AT_CMD_ACTIVE_PDP);
			expectString = AT_CMD_ACTIVE_PDP_RESPOND;
			moduleStatus = ACTIVE_PDP_FINISH;
			break;

		/* 获取本机IP地址 */
		case GET_SELF_IP_ADDR:
			printf("获取本机IP地址\r\n");
			GPRS_SendCmd(AT_CMD_GET_SELF_IP_ADDR);
			expectString = AT_CMD_GET_SELF_IP_ADDR_RESPOND;
			moduleStatus = GET_SELF_IP_ADDR_FINISH;
			break;

		/* 使能GPS功能 */
		case ENABLE_GPS:
			/* 等待GPS使能信号 */
			signal = osSignalWait(GPRSPROCESS_GPS_ENABLE, 2000);
			if ((signal.value.signals & GPRSPROCESS_GPS_ENABLE) == GPRSPROCESS_GPS_ENABLE)
			{
				printf("使能GPS功能\r\n");
				GPRS_SendCmd(AT_CMD_GPS_ENABLE);
				expectString = AT_CMD_GPS_ENABLE_RESPOND;
				moduleStatus = ENABLE_GPS_FINISH;
			}
			else
			{
				printf("GPS 使能等待超时！\r\n");
			}
			break;

		/* 获取GNRMC定位值 */
		case GET_GPS_GNRMC:
			printf("获取GNRMC定位值\r\n");
			/* GPS功能使能比较慢，需要先延时一段时间 */
			GPRS_SendCmd(AT_CMD_GPS_GET_GNRMC);
			expectString = AT_CMD_GPS_GET_GNRMC_RESPOND;
			moduleStatus = GET_GPS_GNRMC_FINISH;
			break;

		/* 设置服务器地址 */
		case SET_SERVER_IP_ADDR:
			/* 等待发送使能信号 */
			signal = osSignalWait(GPRSPROCESS_SEND_DATA_ENABLE, 10000);
			if ((signal.value.signals & GPRSPROCESS_SEND_DATA_ENABLE) == GPRSPROCESS_SEND_DATA_ENABLE)
			{
				/* 获取本次发送的条数 */
				signal = osMessageGet(infoCntMessageQId, 100);
				curPatchPack = signal.value.v;

				/* 获取模拟量信息 */
				signal = osMessageGet(infoMessageQId, 100);
				memcpy(&sendStruct.dataPack, (uint32_t*)signal.value.v,
						curPatchPack * sizeof(FILE_InfoTypedef));

				/* 获取当前时间 */
				signal = osMessageGet(realtimeMessageQId, 100);
				etime = (RT_TimeTypedef*)signal.value.v;

				printf("获取服务器地址\r\n");
				GPRS_SendCmd(AT_CMD_SET_SERVER_IP_ADDR);
				expectString = AT_CMD_SET_SERVER_IP_ADDR_RESPOND;
				moduleStatus = SET_SERVER_IP_ADDR_FINISH;
			}
			break;

		/* 模块准备好了 */
		case READY:
			printf("模块准备好了，发送数据\r\n");
			/* 发送数据到平台 */
			GPRS_SendProtocol(&sendStruct, curPatchPack);
			expectString = AT_CMD_DATA_SEND_SUCCESS_RESPOND;
			moduleStatus = DATA_SEND_FINISH;
			break;

		/* 退出透传模式 */
		case EXTI_SERIANET_MODE:
			printf("退出透传模式\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_SERIANET_MODE);
			expectString = AT_CMD_EXIT_SERIANET_MODE_RESPOND;
			moduleStatus = EXTI_SERIANET_MODE_FINISH;
			break;

		/* 退出连接模式 */
		case EXTI_LINK_MODE:
			printf("退出连接模式\r\n");
			GPRS_SendCmd(AT_CMD_EXIT_LINK_MODE);
			expectString = AT_CMD_EXIT_LINK_MODE_RESPOND;
			moduleStatus = EXTI_LINK_MODE_FINISH;
			break;

		/* 关闭移动场景 */
		case SHUT_MODULE:
			printf("关闭移动场景\r\n");
			GPRS_SendCmd(AT_CMD_SHUT_MODELU);
			expectString = AT_CMD_SHUT_MODELU_RESPOND;
			moduleStatus = SHUT_MODULE_FINISH;
			break;

		default:
			break;
		}

		signal = osSignalWait(GPRS_PROCESS_TASK_RECV_ENABLE, 10000);
		/* 发送超时 */
		if (signal.status == osEventTimeout)
		{
			printf("GMS模块指令接收等待超时\r\n");
			switch (moduleStatus)
			{
			/* 发送到平台的数据没有收到答复,放弃本次数据发送，将模式切换到退出透传模式 */
			case DATA_SEND_FINISH:
				moduleStatus = EXTI_SERIANET_MODE;
				break;

			/* GPS启动过程比较慢，暂时忽略超时等待 */
			case ENABLE_GPS_FINISH:
				break;

			default:
				/* 可能因为看门狗等因素，导致单片机重启，也需要重启模块 */
				if (moduleStatus == MODULE_VALID)
				{
					GPRS_PWR_CTRL_DISABLE();
					osDelay(1000);
				}
				/* 模式切换到前一步再次触发发送 */
				moduleStatus--;

				/* 模块超时计数,如果超过3次，放弃本次发送，挂起任务 */
				moduleTimeoutCnt++;
				if (moduleTimeoutCnt > 3)
				{
					moduleTimeoutCnt = 0;
					printf("模块启动失败，放弃本次发送！\r\n");
					osThreadSuspend(NULL);
				}
				break;
			}
		}
		else if ((signal.value.signals & GPRS_PROCESS_TASK_RECV_ENABLE)
				== GPRS_PROCESS_TASK_RECV_ENABLE)
		{
			/* 接收到任意数据，则将超时计数清空，否则出错 */
			moduleTimeoutCnt = 0;

			/* 寻找预期接收的字符串是否在接收的数据中 */
			if (NULL != strstr((char*)GPRS_BufferStatus.recvBuffer, expectString))
			{
				switch (moduleStatus)
				{
				/* 模块可用 */
				case MODULE_VALID:
					printf("模块可用\r\n");
					/* 开机完成，断开power控制引脚 */
					GPRS_PWR_CTRL_DISABLE();

					/* 模块开机适当延时 */
					osDelay(500);
					moduleStatus = SET_BAUD_RATE;
					break;

					/* 模块启动 */
				case MODULE_START:
					printf("模块启动\r\n");
					/* 开始设置模块参数 */
					moduleStatus = SET_BAUD_RATE;
					break;

					/* 设置波特率完成 */
				case SET_BAUD_RATE_FINISH:
					printf("设置波特率完成\r\n");
					moduleStatus = CHECK_SIM_STATUS;
					break;

					/* 检测sim卡状态完成 */
				case CHECK_SIM_STATUS_FINISH:
					printf("检测sim卡状态完成\r\n");
					moduleStatus = SEARCH_NET_STATUS;
					break;

					/* 查找网络状态完成 */
				case SEARCH_NET_STATUS_FINISH:
					printf("查找网络状态完成\r\n");
					moduleStatus = CHECK_GPRS_STATUS;
					break;

					/* 查找GPRS状态完成 */
				case CHECK_GPRS_STATUS_FINISH:
					printf("查找GPRS状态完成\r\n");
					moduleStatus = SET_SINGLE_LINK;
					break;

					/* 设置单连方式完成 */
				case SET_SINGLE_LINK_FINISH:
					printf("设置单连方式完成\r\n");
					moduleStatus = SET_SERIANET_MODE;
					break;

					/* 设置透传模式完成 */
				case SET_SERIANET_MODE_FINISH:
					printf("设置透传模式完成\r\n");
					moduleStatus = SET_APN_NAME;
					break;

					/* 设置APN名称完成 */
				case SET_APN_NAME_FINISH:
					printf("设置APN名称完成\r\n");
					moduleStatus = ACTIVE_PDP;
					break;

					/* 激活PDP场景完成 */
				case ACTIVE_PDP_FINISH:
					printf("激活PDP场景完成\r\n");
					moduleStatus = GET_SELF_IP_ADDR;
					break;

					/* 获取本机IP地址完成 */
				case GET_SELF_IP_ADDR_FINISH:
					printf("获取本机IP地址完成\r\n");
					moduleStatus = ENABLE_GPS;
					break;

					/* 使能GPS功能完成 */
				case ENABLE_GPS_FINISH:
					printf("使能GPS功能完成\r\n");
					moduleStatus = GET_GPS_GNRMC;
					break;

					/* 获取GNRMC定位值完成 */
				case GET_GPS_GNRMC_FINISH:
					printf("获取GNRMC定位值完成\r\n");
					/* 转换定位数据 */
					GPS_GetLocation(GPRS_BufferStatus.recvBuffer, &location);
					printf("定位数据是%50s",GPRS_BufferStatus.recvBuffer);
					/* 传递定位信息 */
					osMessagePut(infoMessageQId, (uint32_t)&location, 100);
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPS_CONVERT_FINISH);
					/* GPS定位成功，将自己挂起 */
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

					/* 失能GPS功能完成 */
				case DISABLE_GPS_FINISH:
					printf("失能GPS功能完成\r\n");
					moduleStatus = SET_SERVER_IP_ADDR;
					break;

					/* 设置服务器地址完成 */
				case SET_SERVER_IP_ADDR_FINISH:
					printf("设置服务器地址完成\r\n");
					moduleStatus = READY;
					break;

					/* 数据发送完成 */
				case DATA_SEND_FINISH:
					printf("数据发送成功\r\n");
					printf("服务器返回数据是%50s\r\n",GPRS_BufferStatus.recvBuffer);

					/* 将本地时间与云时间对比，时间校准 */
					RT_TimeAdjustWithCloud(GPRS_BufferStatus.recvBuffer, etime);

					moduleStatus = EXTI_SERIANET_MODE;

					/* GPRS发送完成 */
					osSignalSet(mainprocessTaskHandle, MAINPROCESS_GPRS_SEND_FINISHED);
					break;

					/* 退出透传模式完成 */
				case EXTI_SERIANET_MODE_FINISH:
					printf("退出透传模式完成\r\n");
					moduleStatus = EXTI_LINK_MODE;
					break;

					/* 退出单连模式完成 */
				case EXTI_LINK_MODE_FINISH:
					printf("退出单连模式完成\r\n");
					moduleStatus = SHUT_MODULE;
					break;

					/* 关闭移动场景完成 */
				case SHUT_MODULE_FINISH:
					printf("关闭移动场景完成\r\n");
					/* 模块发送完成，把状态设置成使能GPS定位，下次启动直接连接服务器地址即可发送 */
					moduleStatus = GET_GPS_GNRMC;
					/* 将自己挂起 */
					osThreadSuspend(NULL);
					break;

				default:
					break;
				}
			}
			/* 不是接收数据，数据格式错误不会发生 */
			else if (moduleStatus != DATA_SEND_FINISH)
			{
				switch (moduleStatus)
				{
				case DATA_SEND_FINISH:
					break;

				case SET_SERVER_IP_ADDR_FINISH:
					/* 链接服务器地址出现“FAIL”或者“ERROR”，不能链接上服务器 */
					if (NULL != strstr((char*)GPRS_BufferStatus.recvBuffer, "FAIL ERROR"))
					{
						/* 放弃本次发送 */
						moduleStatus = GET_GPS_GNRMC;

						printf("不能链接上服务器，放弃本次发送\r\n");
						/* 将自己挂起 */
						osThreadSuspend(NULL);
					}
					break;

				default:
					/* 判断接收的数据是否错误 */
					/* 数据错误，必须重新初始化模块 */
					if (NULL != strstr((char*)GPRS_BufferStatus.recvBuffer, "Error"))
					{
						/* 复位模块 */
						GPRS_RstModule();

						moduleStatus = SET_BAUD_RATE;
						printf("模块配置错误，重新配置\r\n");
					}
					break;
				}
			}
			memset(GPRS_BufferStatus.recvBuffer, 0, GPRS_BufferStatus.bufferSize);
		}
	}
}



