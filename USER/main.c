
#include <stdio.h>
#include "main.h"
#include "ov2640api.h"
#include "BC26/BC26.h"
#include "BASE64/cbase64.h"
#include "JSON/cjson.h"
#include "common.h"
#include "wd.h"

#define 	RECV_BUF_LEN 	1024
//#define 	DEBUG

static char DEVICEID[] = "00000000000000000000";
static char MYMICCID[] = "00000000000000000000";
static char RSSI[] = "00000000000000000000000000000000";

//static int make_json_data(char *oustr)
//{
//	
//	char * p = 0;
//	cJSON * pJsonRoot = NULL;
//	char tmpstr[32];
//	pJsonRoot = cJSON_CreateObject();
//	if(NULL == pJsonRoot){return -1;}
//	cJSON_AddStringToObject(pJsonRoot, "NO.1 RSSI", RSSI);
//	cJSON_AddNumberToObject(pJsonRoot, "TIME SED", startcnt);
//	cJSON_AddNumberToObject(pJsonRoot, "TIME DIV", DURcnt);
//	cJSON_AddStringToObject(pJsonRoot, "DEVID",DEVICEID);
//	cJSON_AddStringToObject(pJsonRoot, "MICCID", MYMICCID);
//	cJSON_AddStringToObject(pJsonRoot, "TIME","2018.11.29");	
//	p = cJSON_Print(pJsonRoot);	
//	if(NULL == p)
//	{
//		cJSON_Delete(pJsonRoot);
//		return -1;
//	}
//	cJSON_Delete(pJsonRoot);
//	sprintf(oustr,"%s",p);
//	printf("JSON:%s\r\n",oustr);	
//	free(p);
//	return 0;
//}

//static int make_send_data_str(char *outstr , unsigned char *data , int length)
//{	
//	char *tmp = malloc(1024);
//	conv_hex_2_string((unsigned char*)data,length,tmp);
//	sprintf(outstr,"AT+QSOSEND=0,%d,%s\r\n",length,tmp);
//	free(tmp);
//	//printf("SEND: %s \r\n",outstr);
//	return 0;
//}


char * make_sure(char *sendstr , char *backmark , int tst)
{	
		char *recbuf = malloc(RECV_BUF_LEN);		
		memset(recbuf,0x0,RECV_BUF_LEN);	
		int rets =0;	
		for(int i =0 ;i<tst;i++)
		{					
			uart_data_write(sendstr, strlen(sendstr), 0);	
			for(int i =0 ;i<40;i++)
			{	
				memset(recbuf,0x0,RECV_BUF_LEN);					
				rets = uart_data_read(recbuf, RECV_BUF_LEN, 0, 200);
				if(rets)
				{
					char *restr = strstr(recbuf,backmark);				
					if(restr!=NULL)
					{
						printf("recv %d: %s\r\n",tst,restr);	
						free(recbuf);
						return restr+strlen(backmark);
					}				
				}
			}			
		}
		free(recbuf);
		
}


int init(void)
{
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	SysTick_Config(SystemCoreClock / 100);	
	init_uart1();
	init_uart2();	
	RTC_Init();
	SET_BOOTLOADER_STATUS(2);
	WKUP_Pin_Init();
	
	init_utimer();
	init_task();
	init_mem();
	init_uart2_buffer();
	modem_poweron();	
	return 0;
}


int innet(uint32_t tst)
{
	for(int i =0 ;i<tst;i++)
	{	
		if(neul_bc26_get_netstat()>0) return 0;
		utimer_sleep(200);
	}
}


int sleep(uint32_t mins)
{
	modem_poweroff();
	led0_off();
	int nowtime = RTC_GetCounter();
	printf("CurrentTim %d\r\n",nowtime);
	RTC_SetAlarm((uint32_t)(mins*60)+nowtime);
	return nowtime;
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int main(void)
{	
	int tst;
	char *res ;
	uint32_t DURcnt=0;	
	int ret=0,PTR=0;
	uint32_t startcnt= RTC_GetCounter();
	init();
	innet(40);	
		/*
		 * 分配内存
		 */
//		#define RECV_BUF_LEN 1024
//		char *recvbuf = malloc(RECV_BUF_LEN);
//		char *atbuf = malloc(1024);
//		char *jsonbuf = malloc(512);
		
		
		
//		/*
//		 * 发送ATI指令
//		 */
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("ATI\r\n", strlen("ATI\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
		/*
		 * 打开PSM
		 */ 
		
		res = make_sure("AT+CPSMS=1\r\n", "OK", 1);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+CPSMS=1\r\n", strlen("AT+CPSMS=1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QATWAKEUP=1\r\n", strlen("AT+QATWAKEUP=1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);

		res = make_sure("AT+CPIN?\r\n", "OK", 1);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+CPIN?\r\n", strlen("AT+CPIN?\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);			

#ifdef DEBUG
		/*
		 * echo mode
		 */
		printf("ECHO MODE\r\n");
		res = make_sure("ATE1\r\n", "OK", 1);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("ATE1\r\n", strlen("ATE1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
#else
		/*
		 * 关闭回显
		 */
		res = make_sure("ATE0\r\n", "OK", 1);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("ATE0\r\n", strlen("ATE0\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
#endif


		/*
		 * 获取设备ID
		 */
		res = make_sure("AT+CGSN=1\r\n", "OK", 1);
		printf("IMEI : [%s\r\n",res);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+CGSN=1\r\n", strlen("AT+CGSN=1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	
		{
//			char * __tmp = strstr(recvbuf,"OK");
//			if (__tmp > 0)
//			{
//				__tmp -= 19;
//				int i=0;
//				for(i=0;i<15;i++)
//				{
//					DEVICEID[i] = __tmp[i];
//				}
//				//DEVICEID[15] = 'f'; 
//				DEVICEID[16] = 0x0;
		}
	
		res = make_sure("AT*MICCID\r\n", "OK", 1);
		printf("MYMICCID : %s\r\n",res);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT*MICCID\r\n", strlen("AT*MICCID\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
//		{
//			char * __tmp = strstr(recvbuf,"OK");
//			if (__tmp > 0)
//			{
//				__tmp -= 24;
//				int i=0;
//				for(i=0;i<20;i++)
//				{
//					MYMICCID[i] = __tmp[i];
//				}
//				MYMICCID[20] = 0x0;
//		}	
		/*
		 * 获取设备ID
		 */
		
		res = make_sure("AT+CIMI\r\n", "OK", 1);
		printf("CIMI : %s\r\n",res);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+CIMI\r\n", strlen("AT+CIMI\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	
//		{
//			char * __tmp = strstr(recvbuf,"OK");
//			if (__tmp > 0)
//			{
//				__tmp -= 19;
//				int i=0;
//				for(i=0;i<15;i++)
//				{
//					DEVICEID[i] = __tmp[i];
//				}
//				//DEVICEID[15] = 'f'; 
//				DEVICEID[16] = 0x0;				
//		}
		
		res = make_sure("AT+CGPADDR=1\r\n", "OK", 1);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+CGPADDR=1\r\n", strlen("AT+CGPADDR=1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
		res = make_sure("AT+CSQ\r\n", "+CSQ", 5);
		printf("RSSI: %s\r\n",res);
//		{
			/*
			* 获取信号值
			*/
//			int cnt=0;
//			do{				
//				memset(recvbuf,0x0,RECV_BUF_LEN);
//				uart_data_write("AT+CSQ\r\n", strlen("AT+CSQ\r\n"), 0);
//				ret = uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
//				if(strstr(recvbuf,"OK"))
//				{	
//					memcpy(RSSI,uart2_rx_buffer+8,ret-16);	
//					PTR=ret-16;
//					RSSI[PTR++] =' ';
//					RSSI[PTR++] =' ';
//				}				
//				memset(recvbuf,0x0,RECV_BUF_LEN);
//				uart_data_write("AT+CESQ\r\n", strlen("AT+CESQ\r\n"), 0);
//				ret = uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
//				if(strstr(recvbuf,"OK"))
//				{	
//					memcpy(RSSI+PTR,uart2_rx_buffer+9,ret-17);		
//					PTR+=ret-17;
//					memset(RSSI+PTR,'.',32-PTR);							
//				}
//				cnt++;
//				if(cnt>1) utimer_sleep(500);
//			}while(ret==21 && cnt<5);		
//		}


//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QISTATE=1,0\r\n", strlen("AT+QISTATE=1,0\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	

//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QPING=1,\"www.mcuyun.com\"\r\n", strlen("AT+QPING=1,\"www.mcuyun.com\"\r\n"), 0);		
//		do{
//			utimer_sleep(100);
//			ret = uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	
//			tst++;
//		}while(ret==0 && tst < 50);
		
		
		res = make_sure("AT+QIOPEN=1,0,\"TCP\",\"120.79.63.76\",17799,0,1\r\n", "+QIOPEN", 10);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QIOPEN=1,0,\"TCP\",\"120.79.63.76\",17799,0,1\r\n", strlen("AT+QIOPEN=1,0,\"TCP\",\"120.79.63.76\",17799,0,1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	
		
		res = make_sure("AT+QISTATE=1,0\r\n", "+QISTATE", 10);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QISTATE=1,0\r\n", strlen("AT+QISTATE=1,0\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	


		res = make_sure("AT+QICFG=\"viewmode\",1\r\n", "OK", 10);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QICFG=\"viewmode\",1\r\n", strlen("AT+QICFG=\"viewmode\",1\r\n"), 0);
//		uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	

		res = make_sure("AT+QISEND=0,12,\"012345678910\"\r\n", "+QIURC", 10);
//		tst=0;
//		do
//		{
//			memset(recvbuf,0x0,RECV_BUF_LEN);
//			if(tst%10 == 0) uart_data_write("AT+QISEND=0,12,\"012345678910\"\r\n", strlen("AT+QISEND=0,12,\"012345678910\"\r\n"), 0);
//			utimer_sleep(100);
//			ret = uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);
//			if(ret)
//			{
//				char *res = strstr(recvbuf,"+QIURC");				
//				if(res!=NULL)
//				{
//					printf("AT+QISEND %d: %s\r\n",tst,res);	
//					break;
//				}				
//			}				
//			tst++;
//			
//		}while(tst < 20);	

		res = make_sure("AT+QICLOSE=0\r\n", "CLOSE", 3);
//		memset(recvbuf,0x0,RECV_BUF_LEN);
//		uart_data_write("AT+QICLOSE=0\r\n", strlen("AT+QICLOSE=0\r\n"), 0);
//		utimer_sleep(100);
//		ret = uart_data_read(recvbuf, RECV_BUF_LEN, 0, 200);	

		/*
		释放内存
		*/
//		free(recvbuf);
//		free(atbuf);
//		free(jsonbuf);
//	}
	sleep(1);	
	utimer_sleep(10);
	Sys_Enter_Standby();
	printf("pass Sys_Enter_Standby\r\n");
	return 0;
}





#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
	
  USART_SendData(USART1, (uint8_t) ch);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

