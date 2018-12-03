
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

static char DEVICEID[] = "866971030386838";
static char MYMICCID[] = "89860404191880091918";
static char RSSI[] = "000000000000000000000000000     ";
static uint32_t timeflag =0;



char * make_sure(char *sendstr , char *backmark , int tst)
{	
		char *recbuf = malloc(RECV_BUF_LEN);		
		memset(recbuf,0x0,RECV_BUF_LEN);	
		int rets =0;	
		for(int i =0 ;i<tst;i++)
		{					
			uart_data_write(sendstr, strlen(sendstr), 0);	
			for(int i =0 ;i<20;i++)
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
					if(strstr(recbuf,"ERROR")!=NULL) printf("$$$ error\r\n");
				}
			}			
		}
		free(recbuf);
		return NULL;
}

char * recv_sure(char *mark , int tst)
{	
		char *recbuf = malloc(RECV_BUF_LEN);		
		int rets =0;	
		for(int i =0 ;i<tst;i++)
		{					
			memset(recbuf,0x0,RECV_BUF_LEN);					
			rets = uart_data_read(recbuf, RECV_BUF_LEN, 0, 200);
			if(rets)
			{
				char *restr = strstr(recbuf,mark);				
				if(restr!=NULL)
				{
					printf("recv %d: %s\r\n",tst,restr);	
					free(recbuf);
					return restr+strlen(mark);
				}
				if(strstr(recbuf,"ERROR")!=NULL) printf("error\r\n");
			}		
		}
		free(recbuf);
		return NULL;
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
	//init_task();
	init_mem();
	init_uart2_buffer();
	modem_poweron();	
	return 0;
}


int sleep(uint32_t mins)
{
	modem_poweroff();
	led0_off();
	uint32_t nowtime = RTC_GetCounter();
	printf("CurrentTim %d # duration %d\r\n",nowtime,nowtime-timeflag);
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
	char *res,*ptr;
	char *str;
	init();
	timeflag = RTC_GetCounter();	
		
		res = make_sure("AT+CGATT?\r\n", "+CGATT: 1", 40);
		res = make_sure("AT+CPSMS=1\r\n", "OK", 1);
		res = make_sure("AT+CPIN?\r\n", "+CPIN: READY", 1);
		if(res==NULL) printf("NO SIM card\r\n");

#ifdef DEBUG
		res = make_sure("ATE1\r\n", "OK", 1);
#else
		res = make_sure("ATE0\r\n", "OK", 1);
#endif
		res = make_sure("AT+CGSN=1\r\n", "+CGSN: ", 1);
		printf("IMEI : %s\r\n",res);
		memcpy(DEVICEID,res,strlen(DEVICEID));
		printf("DEVICEID:%s\r\n",DEVICEID);
	
		res = make_sure("AT*MICCID\r\n", "*MICCID: ", 1);
		memcpy(MYMICCID,res,strlen(MYMICCID));
		printf("MYMICCID:%s\r\n",MYMICCID);

//		res = make_sure("AT+CIMI\r\n", "OK", 1);
		
		res = make_sure("AT+CGPADDR=?\r\n", "+CGPADDR:", 1);
		ptr = strchr(res,'(');
		printf("*** CGPADDR:%c\r\n",ptr[1]);
		
//		sprintf(str,"AT+CGPADDR=%c\r\n",ptr[1]);
//		res = make_sure(str, "+CGPADDR:", 1);
		
		res = make_sure("AT+CSQ\r\n", "+CSQ:", 5);
		printf("CSQ : %s\r\n",res);
		memcpy(RSSI,res,5);	
		
		res = make_sure("AT+CESQ\r\n", "+CESQ:", 5);
		printf("CESQ : %s\r\n",res);	
		memcpy(RSSI+6,res,21);	
		char *p;
		while((p = strchr(RSSI,','))!=NULL)
		{
			*p=':';
		}
		printf("***** RSSI: %s\r\n",RSSI);
        

//		res = make_sure("AT+QPING=1,\"www.mcuyun.com\"\r\n", "+QPING:", 1);
		
		res = make_sure("AT+QIOPEN=1,0,\"TCP\",\"120.79.63.76\",17799,0,1\r\n", "+QIOPEN", 5);
	
		res = make_sure("AT+QISTATE=1,0\r\n", "+QISTATE", 1);

		res = make_sure("AT+QICFG=\"viewmode\",1\r\n", "OK", 1);

		res = make_sure("AT+QISEND=0,12,\"012345678910\"\r\n", "+QIURC:", 5);
        printf(" *****command:%s\r\n",strstr(res,"recv\",")+strlen("recv\","));
		
		res = make_sure("AT+QICLOSE=0\r\n", "CLOSE", 1);
		
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

