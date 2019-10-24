///////////////////////////////////////////////////////////////////////////////
//
//   onenet Э�鴦��  
//
///////////////////////////////////////////////////////////////////////////////
#include "app_protocol_onenet_edp.h"
int16_t hex2str(INT8U *hex, INT16U len, INT8U *str);
extern volatile INT8S tcp_client_socket_id;
uint8_t gprs_send_app_data(void);
extern tagWirelessObj GprsObj;
uint8_t gprs_read_app_data(void);
int16_t gprs_send_cmd_wait_OK(uint8_t *cmd,uint8_t *resp,uint16_t max_resp_len,uint16_t timeout_10ms);
void gprs_power_off(void);
void remote_send_app_frame(uint8_t *frame,uint16_t frame_len);

INT8U edp_version[7] ={((SOFT_VER_YEAR>>4)&0x0F)+'0',((SOFT_VER_YEAR)&0x0F)+'0',((SOFT_VER_MONTH>>4)&0x0F)+'0',((SOFT_VER_MONTH)&0x0F)+'0',
						((SOFT_VER_DAY>>4)&0x0F)+'0',((SOFT_VER_DAY)&0x0F)+'0',0};
						
						

void responseCommandToOnenet(const char* cmdid, uint16_t cmdid_len,INT8U commandIndex)
{
	EdpPacket* pkg = NULL;
	INT8U posStart =0;
	INT8U suc[6] ={0x90,0x04,0x40,0x00,0x01,0x00};
	INT8U res=0;//�ж�ƽ̨�Ƿ񷵻ؽ��
	INT8U tmp_has_dataToRead =0;
	//mem_set(EdpData,BUFFER_SIZE,0x00);

	INT32U resLen;
	
	//switch (commandIndex)
	//{
		//case  7:		//ʵʱ������Ϣ
		//while(!reportHoldDayEnergyToOnenet(0));
		//break;
		//case  11:		//ʵʱ�ն�����Ϣ
		//while(!reportHoldDayEnergyToOnenet(1));
		//break;
		//case 14:			 //��ȡ������Ϣ���ݿ�
		//while(!reportyEnergyBlockToOnenet());
		//break;
	//}
	//DelayNmSec(5000);
	while(!res)
	{
		mem_set(OneNetData,1024,0x00);
		if(commandIndex ==5)
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}
		if(commandIndex ==6)
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}

		if(commandIndex == 7) //��ʵʱ������Ϣ
		{
			resLen = cjsonEnergyInfoPack(0,OneNetData);;  //ʵʱ������Ϣ
		}
		if(commandIndex == 11) //��ʵʱ������Ϣ
		{
			resLen = cjsonEnergyInfoPack(1,OneNetData);  //ʵʱ�ն�����Ϣ
		}
		if(commandIndex == 12)
		{
			resLen = cjsonReadMeter(OneNetData);   //͸���������ݡ�
		}
#ifdef __ZHEJIANG_ONENET_EDP__		
		if(commandIndex ==13)	   //�¶��������ϱ���
		{
			resLen =cjsonEnergyBlockPackMonth(OneNetData);
		}
#endif		
		if(commandIndex == 14)
		{
			resLen = cjsonEnergyBlockPack(OneNetData);  //��ȡ������Ϣ���ݿ�
		}
		if(commandIndex == 32)  //��Ӧ���û�Ӧʱ�����ı��ġ�
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}		
		if(commandIndex == 243)
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}
		if(commandIndex ==100)  //����onenetƽ̨��������ַ�Ͷ˿�
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}
		if(commandIndex ==101) // ����������ת�������Ͷ˿�
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}
		if(commandIndex ==102)  //��ȡû���ϱ��ɹ�����������
		{
			mem_set(OneNetData,4,0x00);
			resLen =4;
		}
		pkg = PacketCmdResp(cmdid, cmdid_len, OneNetData,resLen);
		
		
//		hex2str(pkg->_data,pkg->_write_pos,oneNetstr);
//		sprintf((char*)oneNetstr2,"AT+NSOSD=%d,%d,%s\r\n", tcp_client_socket_id,pkg->_write_pos,oneNetstr);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
//		sprintf((char*)oneNetstr2,"AT+NSOSD=%d,%d,%s\r\n", tcp_client_socket_id,pkg->_write_pos,oneNetstr);
		//DelayNmSec(50);
//		gprs_send_cmd_wait_OK(oneNetstr2,gprs_cmd_buffer,sizeof(gprs_cmd_buffer),500);
		res =1;
	}
}

static int32_t find_first_num(INT8U *buf, INT16U len)
{
	INT16U i,find=0;
	INT32U num=0;
	for(i = 0; i<len; i++)
	{
		if(buf[i] >= '0' && buf[i]<= '9')
		{
			num = num*10;
			num += buf[i] - '0';
			find = 1;
		}
		else
		{
			if(find == 1)
			return num;
		}
	}
	return -1;
}
INT8U find_ip_port(INT8U *buf,INT16U len,INT8U *ip_port)
{
	INT16U i,find=0;
	INT16U num=0;
	INT8U ip_pos =0;
	INT8U transBegin =0;
	for(i = 0;i<len;i++)
	{
		if((buf[i] >= '0') && (buf[i]<= '9'))
		{
			transBegin =1;
			num = num*10;
			num += buf[i] - '0';
			continue;
		}
		if(transBegin)
		{
			if(buf[i]!='\"')
			{
				*(ip_port+ip_pos++) =(INT8U)num;
				num =0;
			}
			else 
			{
				*(ip_port+ip_pos++) =(INT8U)num;
				*(ip_port+ip_pos++) =(INT8U)(num>>8);
			}
		}
		
	}
	if(*ip_port !=0)
	{
		return 1;
	}
	return 0;
}
static uint8_t gprs_cmd_buffer2[200];
void registerDevice()
{    INT8U data[60]={0X10,0X2B,0X00,0X03,0X45,0X44,0X50,0X01,0XC0,0X01,0X2C,0X00,0X00,0X00,0X06,0X32,0X31,0X32,0X35,
					0X32,0X35,0X00,0X16};//0X31,0X31,0X31,0X31,0X31,0X31,0X31,0X31,0X31,0X31,0X31,0X31 0X31 0X31 0X31 0X31 0X31 0X31 0X31 0X31 0X31 0X31 };
	INT8U  str[200],str2[200];
	INT32U passMS=0;
	volatile INT8U tmp;
	INT8U* oneNetstr = NULL;
	INT8U* oneNetstr2 = NULL;
	INT8U idx=0,count=0;
	INT16U Gprs_recv_cnt =0 ;
//	mem_set(EdpData,BUFFER_SIZE,0x00);
    oneNetstr = ResponseApp.frame+500;
	oneNetstr2 =ResponseApp.frame+700;
	mem_set(oneNetstr,200,0x00);
	mem_set(oneNetstr2,200,0x00);
	for(idx=0;idx<22;idx++)
	{
		data[idx+23] = gSystemInfo.managementNum[idx];
	}

//	tcp_client_socket_id =0;   //9600����ǰ���Լ���õģ�һ����1
	gSystemInfo.edp_login_state = 0;
	gSystemInfo.dev_config_ack = 0;
	while(!gSystemInfo.edp_login_state)
	{
		GprsObj.send_len = 45;
		GprsObj.send_ptr = data;
		gprs_send_app_data();
		//remote_send_app_frame(data,45);	
		//gprs_send_app_data();
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<30)&&(!gSystemInfo.edp_login_state)) //С��10��
		{
#ifdef DEBUG			
			system_debug_info("in loop login");
#endif		
			tmp =	gprs_read_app_data();
			system_debug_info("\xff\xff\xff");
			system_debug_data(&tmp,1);
			system_debug_info("\xff\xff\xff");
			DelayNmSec(50);
			//if(GprsObj.recv_pos -Gprs_recv_cnt)
			//{
				//mem_cpy(RequestRemote.frame,GprsObj.recv_buf + Gprs_recv_cnt,  GprsObj.recv_pos -Gprs_recv_cnt);
				//RequestRemote.frame_len =GprsObj.recv_pos -Gprs_recv_cnt;
				//app_protocol_handler_edp(&RequestRemote,NULL);
			//}
		}
		if(count++>3)
		{
			app_softReset();
		}
	}
//�ϵ��ƽ̨�ϵĲ�Ʒ�Ž������Ӻ���ȶ�onenet���ŵ�һЩ���ݣ�������ա�Ȼ��ִ�к���Ĳ��衣
//{
	//passMS =system_get_tick10ms();
	//Gprs_recv_cnt = GprsObj.recv_pos ;
	//while((second_elapsed(passMS)<15))
	//{
		//gprs_read_app_data();
		//if(GprsObj.recv_pos != Gprs_recv_cnt)
		//{
			//passMS =system_get_tick10ms();
			//Gprs_recv_cnt = GprsObj.recv_pos;
		//}
		//
//#ifdef DEBUG
			//system_debug_data(&Gprs_recv_cnt,2);
			//system_debug_info("in 1111111");
			//system_debug_data(&GprsObj.recv_pos,2);
//#endif				
	//}
//}


}
/**
*��ʱ������������ʱ��2����Ϊ׼
*/
INT8U edp_ping()
{
	INT8U count =0 ;
	INT32U passMS=0;
	gSystemInfo.edp_ping_state =0;
	if(GprsObj.send_len)
	{
		return 0 ;
	}
	//
	mem_set(OneNetData,1024, 0x00);
	while(!gSystemInfo.edp_ping_state)
	{

		EdpPacket* pkg =PacketPing();
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		system_debug_info("quit find edp ");
		passMS =system_get_tick10ms();
#ifdef DEBUG
system_debug_info("in loop edp");
#endif		
		while((second_elapsed(passMS)<10)&&(!gSystemInfo.edp_ping_state)) //С��10��
		{
			DelayNmSec(5);
		}
#ifdef DEBUG
system_debug_info("out loop edp");
#endif		
		if(count++>3)
		{
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_ping_state =0;
			gSystemInfo.dev_config_ack = 0;
			break;
		}
			
	}
	return 1;
}

/**
 * �ϱ��豸������Ϣ��onenetƽ̨����ƽ̨�·���save_ack���д���
 */
BOOLEAN reportConfigToOnenet()
{
  	INT8U count =0 ;
  	INT32U passMS=0;
	EdpPacket* pkg;
  	gSystemInfo.save_ack =0;
  	mem_set(OneNetData,1024, 0x00);
	gSystemInfo.dev_config_ack = 0;
  	while(!gSystemInfo.save_ack)
  	{
		cjsonDeviceConfigPack(OneNetData);
	  	 pkg = PacketSavedataJson2(NULL, OneNetData, 1, 1);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
	  	passMS =system_get_tick10ms();
	  	while((second_elapsed(passMS)<30)&&(!gSystemInfo.save_ack)) //С��10��
	  	{
#ifdef DEBUG			 
		  	//system_debug_info("in loop configration");
#endif			
			DelayNmSec(2);
	  	}
	  	if(count++>3)
	  	{
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_login_state =0;
			gSystemInfo.dev_config_ack = 0;
			break;
	  	}
	  	 gSystemInfo.dev_config_ack = 1;
		 edp_ping_start_time =system_get_tick10ms();//

  	}
	 
	return 1;
}


/**
 * �ϱ��豸�ն��������й���Ϣ��onenetƽ̨����ƽ̨�·���save_ack���д���
 */
INT8U reportHoldDayEnergyToOnenet(INT8U flag)
{
	INT8U count =0 ;
	DateTime dt;
	tpos_datetime(&dt);
	INT64U report_time =0; //��ʱ֮����Ҫ�ϱ���ʱ�̣�����������м���һ����ʱʱ�䡣
	INT32U passMS=0;
	EdpPacket* pkg;
	gSystemInfo.save_ack =0;
	if(wait_report_flag)  //��Ҫֱ�Ӵ洢�������Ա���ʱʹ�á�
	{
#ifdef DEBUG
		system_debug_info("\r\n***************waite report modestore hold day energy****************\r\n");
#endif
		cjsonEnergyInfoPack(flag,OneNetData);
		pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		mem_cpy_right(pkg->_data+8,pkg->_data,pkg->_write_pos);
		if(flag == 1)
		{
			report_time = getPassedSeconds(&dt,2000)+(getrand()%60)*60;	
		}
		else if(flag == 0)
		{
			report_time = getPassedSeconds(&dt,2000); //+getrand()%(gSystemInfo.edp_rand_report_end -gSystemInfo.edp_rand_report_begin)*60 + getrand()%60;
		}

		mem_cpy(pkg->_data,(INT8U*)&report_time,8);
		handle_wait_report_data_write(pkg->_data,pkg->_write_pos+8);//���û�ϴ��ɹ���ô�洢������
		return 1;
	}
	if(!cmdType)   //����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
	{
		mem_set(OneNetData,1024, 0x00);  	
	}
	while(!gSystemInfo.save_ack)
	{
		if(!cmdType)//����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
		{
			cjsonEnergyInfoPack(flag,OneNetData);
		}

		EdpPacket* pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<30)&&(!gSystemInfo.save_ack)) //С��10��
		{
			DelayNmSec(2);
		}
#ifdef DEBUG
		system_debug_info("\r\n*********in report hold day energy **********");
#endif		
		if(count++>3)
		{
			handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������			
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_login_state =0;
			gSystemInfo.dev_config_ack = 0;
			break;	
		}
		  	
	}
	return 1;
}

/**
 * �ϱ��豸�¶��������й���Ϣ��onenetƽ̨����ƽ̨�·���save_ack���д���
 */
#ifdef __ZHEJIANG_ONENET_EDP__
INT8U reportMonthDayEnergyToOnenet()
{
	INT8U count =0 ;
	DateTime dt;
	tpos_datetime(&dt);
	INT64U report_time =0; //��ʱ֮����Ҫ�ϱ���ʱ�̣�����������м���һ����ʱʱ�䡣
	INT32U passMS=0;
	EdpPacket* pkg;
	gSystemInfo.save_ack =0;
	if(wait_report_flag)  //��Ҫֱ�Ӵ洢�������Ա���ʱʹ�á�
	{
#ifdef DEBUG
		system_debug_info("\r\n***************waite report modestore hold day energy****************\r\n");
#endif
		cjsonEnergyBlockPackMonth(OneNetData);
		pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		mem_cpy_right(pkg->_data+8,pkg->_data,pkg->_write_pos);
		report_time = getPassedSeconds(&dt,2000); //+getrand()%(gSystemInfo.edp_rand_report_end -gSystemInfo.edp_rand_report_begin)*60 + getrand()%60;
		mem_cpy(pkg->_data,(INT8U*)&report_time,8);
		handle_wait_report_data_write(pkg->_data,pkg->_write_pos+8);//���û�ϴ��ɹ���ô�洢������
		return 1;
	}
	if(!cmdType)   //����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
	{
		mem_set(OneNetData,1024, 0x00);  	
	}
	while(!gSystemInfo.save_ack)
	{
		if(!cmdType)//����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
		{
			cjsonEnergyBlockPackMonth(OneNetData);
		}

		EdpPacket* pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<30)&&(!gSystemInfo.save_ack)) //С��10��
		{
			DelayNmSec(2);
		}
#ifdef DEBUG
		system_debug_info("\r\n*********in report hold day energy **********");
#endif		
		if(count++>3)
		{
			handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������			
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_login_state =0;
			gSystemInfo.dev_config_ack = 0;
			break;	
		}
		  	
	}
	return 1;
}
#endif
/**
 * �ϱ���վ��͸�����ݵ�ƽ̨����ƽ̨�·���save_ack���д���
 */
INT8U report645CmdResultToOnenet()
{
	INT8U count =0 ;
	INT32U passMS=0;
	EdpPacket* pkg;
	gSystemInfo.save_ack =0;
	if(!gSystemInfo.tcp_link) //���û�е�¼�ɹ�����ôֱ�Ӵ洢
	{
#ifdef DEBUG			 
		  	system_debug_info("\r\n***************store hold day energy****************\r\n");
#endif			
		cjsonReadMeter(OneNetData);
		pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������
		return 1;
	}
	if(!cmdType)   //����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
	{
		mem_set(OneNetData,1024, 0x00);  	
	}
//	tcp_client_socket_id =0;   //9600����ǰ���Լ���õģ�һ����1
	while(!gSystemInfo.save_ack)
	{
		if(!cmdType)//����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
		{
			cjsonReadMeter(OneNetData);
		}

		EdpPacket* pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		//GprsObj.send_len =pkg->_write_pos;
		//GprsObj.send_ptr = pkg->_data;
		//gprs_send_app_data();
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<10)&&(!gSystemInfo.save_ack)) //С��10��
		{
			DelayNmSec(2);
			//gprs_read_app_data();
		}
#ifdef DEBUG
		system_debug_info("\r\n*********in report hold day energy **********");
#endif		
		if(count++>3)
		{
			handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������			
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_login_state =0;
			gSystemInfo.dev_config_ack = 0;
			break;
			//gprs_power_off();
			//app_softReset();
			
		}
		  	
	}
	return 1;
}
/**
 * �ϱ��豸�����й���Ϣ��onenetƽ̨����ƽ̨�·���save_ack���д���
 */
INT8U reportyEnergyBlockToOnenet()
{
	INT8U count =0 ;
	DateTime dt;
	tpos_datetime(&dt);
	INT64U report_time =0; //��ʱ֮����Ҫ�ϱ���ʱ�̣�����������м���һ����ʱʱ�䡣	
	INT32U passMS=0;
	EdpPacket* pkg;
	gSystemInfo.save_ack =0;
	if(wait_report_flag)  //��Ҫֱ�Ӵ洢�������Ա���ʱʹ�á�
	{
#ifdef DEBUG
		system_debug_info("\r\n***************waite report mode store energy block****************\r\n");
#endif
		cjsonEnergyBlockPack(OneNetData);
		pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		mem_cpy_right(pkg->_data+8,pkg->_data,pkg->_write_pos);
		report_time = getPassedSeconds(&dt,2000);//+getrand()%(gSystemInfo.edp_rand_report_end -gSystemInfo.edp_rand_report_begin)*60 + getrand()%60;
		mem_cpy(pkg->_data,(INT8U*)&report_time,8);
		handle_wait_report_data_write(pkg->_data,pkg->_write_pos+8);//�����ݴ洢�ڴ��ϴ��б��У�ʱ�䵽�����ϴ���
		return 1;
	}	
	if(!cmdType)   //����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
	{
		mem_set(OneNetData,1024, 0x00);
	}	  	
#ifdef DEBUG
		system_debug_info("\r\n***************in  energy block report1 ****************\r\n");
#endif	
	while(!gSystemInfo.save_ack)
	{
		if(!cmdType)   //����Ƕ������ٲ�Ļ�Ӧ����ô�Ͳ�Ҫ��ȥ���¶������ˣ���Ϊǰ������������Ѿ��Ȼظ��ˣ���ôֻ��Ҫ�ù���ת���Ϳ�����
		{		
		cjsonEnergyBlockPack(OneNetData);
		}
		pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<10)&&(!gSystemInfo.save_ack)) //С��10��
		{
			#ifdef DEBUG
			system_debug_info("\r\n*********in  energy block report2 **********");
			#endif
			DelayNmSec(2);
		}
		if(count++>3)
		{
			handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_login_state=0;
			gSystemInfo.dev_config_ack = 0;
			break;
			//gprs_power_off();
			//app_softReset();

		}
		  	
	}
	return 1;
}
/**
 * �ϱ��豸������Ϣ��onenetƽ̨����ƽ̨�·���save_ack���д���
 */
INT8U responseUpdataResToOnenet(INT8U flag)
{
	INT8U count =0 ;
	INT32U passMS=0;
	gSystemInfo.save_ack =0;
	if(!gSystemInfo.tcp_link)
	{
		return  0 ;
	}
	mem_set(OneNetData,1024, 0x00);  	  	
	while(!gSystemInfo.save_ack)
	{
		cjsonFirmwareUpdatePack(OneNetData,flag);
		EdpPacket* pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<10)&&(!gSystemInfo.save_ack)) //С��10��
		{
			DelayNmSec(10);
		}
		#ifdef DEBUG
		system_debug_info("\r\n*********in report updata info **********");
		#endif
		if(count++>3)
		{
			gSystemInfo.tcp_link = 0;
			break;
			gprs_power_off();
			app_softReset();
		}
	}
	return 1;
}
INT32U  str2hexGen(INT8S *str)
{
	INT32U value,idx;
	BOOLEAN  start;

	value = 0;

	//��ֵǰ��Ŀո�����ӹ�
	start = TRUE;
	for(idx=0;idx<10;idx++)
	{
		if(*str==' ')
		{
			if(!start) break;
		}
		else if((*str < '0')  || (*str > '9') )
		{
			break;
		}
		else
		{
			start = FALSE;
			value *= 16;
			value += *str-'0';
		}
		str++;
	}
	return value;
}
uint8_t update_ertu_datetime(uint8_t force_flag);

void app_protocol_handler_edp(objRequest* pRequest,objResponse *pResp)
{
	INT8U tmp_updata_verison[4] ={0};
	EdpPacket  pkg;
	INT8U  update_info_search_index =0,updata_mess_pos2 =0;
	INT8U cmdid[100],req[500]={0},idx,idx1=0,cs=0,flag =0,req_pos=0;
	INT8U str[50]={0},hex[50]={0},tmp[50]={0},pos1=0,pos2=0,tmpVersion;
	uint16 cmdid_len,req_len,serverDataPos,serverCommand=0,serverData=0;
	INT16S serverCommandPos =0,updata_mess_pos1=0;
	INT8U tmpPos=0,tmpPos2 = 0,rand_loop =0;
	INT8U rand_report_begin =0,rand_report_end =0;
	volatile INT8U data_tmp =0;
	INT8U updata_compare_tmp[10]={0};
	INT8U ip_port[6] ={0};
	//if((!gSystemInfo.tcp_link) &&(CMDREQ ==pRequest->frame[0]))  //��û�е�¼��������ִ������
	//{
//#ifdef DEBUG
		//system_debug_info("\r\n***********dont to handle command no register*********\r\n");
//#endif		
		//return;
	//}
#ifdef DEBUG
		system_debug_info("\r\n***********to handle command no register*********\r\n");
#endif
	pkg._data = pRequest->frame;
	pkg._write_pos = pRequest->recv_len;
	ertu_month_bytes_add(pRequest->recv_len);
	pkg._read_pos = 0;
	pkg._capacity = BUFFER_SIZE;
	switch(pkg._data[pkg._read_pos])
	{
		case CONNREQ: /* �������� */
		break;
		case CONNRESP: /* ������Ӧ */
			if(compare_string(pkg._data,"\x20\x02\x00\x00",4) ==0)
			{
				gSystemInfo.login_status = 1;
				gSystemInfo.edp_login_state =1;
			}
#ifdef DEBUG
			system_debug_info("\r\n***********already connect to onenet*********\r\n");
#endif		
			break;
		case PUSHDATA: /* ת��(͸��)���� */
		break;
		case SAVEDATA:/* �洢(ת��)���� */
		break;
		case SAVEACK: /* �洢ȷ�� */
			if(compare_string(pkg._data,"\x90\x04\x40\x00\x01\x00",6) ==0)
			{
				gSystemInfo.save_ack =1;
			}
#ifdef DEBUG			
			system_debug_info("\r\n***********already store ack*********\r\n");
#endif			
			break;
		case  CMDREQ:/* �������� */
			UnpackCmdReq(&pkg, cmdid, &cmdid_len,req, &req_len);
		if((serverCommandPos = str_find(req, req_len, "\"ServerCommand\":{\"int\":", 23)) >= 0)
			{
				serverCommand = find_first_num(req+serverCommandPos, req_len);
			}
			system_debug_info("\r\nMglget command server command");
			switch(serverCommand)
			{
				case 5: //�޸��豸�ɼ����ݵ�ʱ����
					responseCommandToOnenet(cmdid, cmdid_len,5);
					if((serverCommandPos = str_find(req, req_len, "\"ServerData\":{\"string\":", 20)) >= 0)
					{
						serverData = find_first_num(req+serverCommandPos, req_len);
						fwrite_ertu_params(EEADDR_TIME_INTERVAL,&serverData,2);
						gSystemInfo.edp_report_interval =(INT8U)serverData;
					}
					break;
					case 6: //ͬ���豸ʱ��
						responseCommandToOnenet(cmdid, cmdid_len,6);
						if((pos1 = str_find(req, req_len, "\"string\":\"", 10)) >= 0) //17-06-14 14:45:05
						{
							pos1+=10;
							for(idx=0;idx<17;)
							{
								if((req[pos1+idx]>='0')&&(req[pos1+idx]<='9'))
								{
									hex[idx1++] = (req[pos1+idx]-'0')*16+(req[pos1+idx+1]-'0');
									idx+=2;
								}
								else
								{
									idx++;
								}
							}
							idx = 0; //�鱨�ĳ�ʼλ��
							str[idx++] =0x68;
							str[idx++] =gSystemInfo.meter_no[0];str[idx++] =gSystemInfo.meter_no[1];str[idx++] =gSystemInfo.meter_no[2];
							str[idx++] =gSystemInfo.meter_no[3];str[idx++] =gSystemInfo.meter_no[4];str[idx++] =gSystemInfo.meter_no[5];
							str[idx++] = 0x68;
							str[idx++] = 0x14;
							str[idx++] = 0x10;
							str[idx++] = 0x34;
							str[idx++] = 0x34;
							str[idx++] = 0x33;
							str[idx++] = 0x37;
							str[idx++] =0x35;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
							str[idx++] =0xAB;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
							hex[6]=weekDay(BCD2byte(hex[0]),BCD2byte(hex[1]),BCD2byte(hex[2]));//week
							str[idx++] =hex[6] +0x33;str[idx++] = hex[2] +0x33;
							str[idx++] =hex[1] +0x33;str[idx++] = hex[0] +0x33;
							cs = 0;
							for(pos1 =0;pos1<idx;pos1++) cs += str[pos1];
							str[idx++] = cs;
							str[idx++] =0x16;
							app_trans_send_meter_frame(str,28, tmp,200,100);
				
							idx = 0; //�鱨�ĳ�ʼλ��
							str[idx++] =0x68;
							str[idx++] =gSystemInfo.meter_no[0];str[idx++] =gSystemInfo.meter_no[1];str[idx++] =gSystemInfo.meter_no[2];
							str[idx++] =gSystemInfo.meter_no[3];str[idx++] =gSystemInfo.meter_no[4];str[idx++] =gSystemInfo.meter_no[5];
							str[idx++] = 0x68;
							str[idx++] = 0x14;
							str[idx++] = 0x0F;
							str[idx++] = 0x35;
							str[idx++] = 0x34;
							str[idx++] = 0x33;
							str[idx++] = 0x37;
							str[idx++] =0x35;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
							str[idx++] =0xAB;str[idx++] =0x89;str[idx++] =0x67;str[idx++] =0x45;
							str[idx++] = hex[5] +0x33;
							str[idx++] = hex[4] +0x33;
							str[idx++] = hex[3] +0x33;
							cs = 0;
							for(pos1 =0;pos1<idx;pos1++) cs += str[pos1];
							str[idx++] = cs;
							str[idx++] =0x16;
							app_trans_send_meter_frame(str,27, tmp,200,100);
						}
						update_ertu_datetime(1);
						break;
					case 7: //�ɼ���ǰʵʱ����
						responseCommandToOnenet(cmdid, cmdid_len,7);
						cmdType =7;
						break;
					case 11://�ɼ��ն������
						responseCommandToOnenet(cmdid, cmdid_len,11);
						cmdType=11;
						break;
					case 12: //͸������
						cmd645 = 0;
						if((serverCommandPos = str_find(req, req_len, "\"ServerData\":{\"string\":", 22)) >= 0)
						{
							tmpPos =24+serverCommandPos;
							cmd645 = str2hexGen(req+tmpPos);
						}
						responseCommandToOnenet(cmdid, cmdid_len,12);
						cmdType =12;
						break;
#ifdef __ZHEJIANG_ONENET_EDP__						
					case 13:
						responseCommandToOnenet(cmdid, cmdid_len,13);
						cmdType=13;						
						break;
#endif						
					case 14: //��ȡ������Ϣ���ݿ�
						system_debug_info("\r\nMglget command14:");
						responseCommandToOnenet(cmdid, cmdid_len,14);
						cmdType =14;
						break;
					case 15: //ƽ̨�յ��澯�¼�����Ӧ
				
						break;
					case 32:  //��������Ӻ�ʱ�䷶Χ
						if((serverCommandPos = str_find(req, req_len, "\"ServerData\":{\"string\":", 22)) >= 0)
						{
							tmpPos =24;
							tmpPos2 =24 ;
							for(rand_loop =0;rand_loop<3;rand_loop++)
							{
								data_tmp = req[tmpPos+serverCommandPos];
								if((data_tmp>='0')&&(data_tmp<='9'))
								{
									rand_report_begin = rand_report_begin*10+ data_tmp -0x30;
									tmpPos++;
								}
								else
								{
									tmpPos++;
									break;
								}
							}
							for(rand_loop =0;rand_loop<3;rand_loop++)
							{
								data_tmp = req[tmpPos+serverCommandPos];
								if((data_tmp>='0')&&(data_tmp<='9'))
								{
									rand_report_end = rand_report_end*10+ data_tmp -0x30;
									tmpPos++;
								}
								else
								{
									tmpPos++;
									break;
								}
							}
							fwrite_ertu_params(EEADDR_EDP_RAND_BEGIN,&rand_report_begin,1);
							fwrite_ertu_params(EEADDR_EDP_RAND_END,&rand_report_end,1);	
							gSystemInfo.edp_rand_report_begin =(INT8U)rand_report_begin;	
							gSystemInfo.edp_rand_report_begin =(INT8U)rand_report_end;						
						}
						cmdType = 32;
						break;
					case 242 : //�����豸
						app_softReset();
						break;
					case 243:   //�̼��������� edp_updata_INFO

						switch(update_info_search_index)
						{
							case 0:
								mem_set(updata_compare_tmp,10,0x00);
								updata_compare_tmp[0] ='-';//updata_compare_tmp[1] ='_';
								if((updata_mess_pos1=str_find(req, req_len, updata_compare_tmp, 1)) >= 0)
								{
									edp_updata_INFO.updata_plat =*(req+updata_mess_pos1+1);
									update_info_search_index =1;
									req_pos = updata_mess_pos1+1+1;  //���ϵ���-G,���ֽ�
									req_len =req_len - updata_mess_pos1 -1-1;
								}
								//break;
							case 1:
								mem_set(updata_compare_tmp,10,0x00);
								updata_compare_tmp[0] ='-';							
								if((updata_mess_pos1=str_find(req+req_pos, req_len, updata_compare_tmp, 1)) >= 0)
								{
									edp_updata_INFO.updata_moduleType =*(req+req_pos+updata_mess_pos1+1);
									update_info_search_index =2;
									req_pos +=updata_mess_pos1+1+1;  //���ϵ���-T,���ֽ�
									req_len =req_len - updata_mess_pos1 -1-1;
								}								
								//break;
							case 2:
								mem_set(updata_compare_tmp,10,0x00);
								updata_compare_tmp[0] ='-';	updata_compare_tmp[1] ='F';updata_compare_tmp[2] ='R';
								updata_compare_tmp[3] ='O';updata_compare_tmp[4] ='M';				
								if((updata_mess_pos1=str_find(req+req_pos, req_len, updata_compare_tmp, 5)) >= 0)
								{
									mem_cpy(edp_updata_INFO.updata_from,req+req_pos+updata_mess_pos1+5,6);
									update_info_search_index =4;
									req_pos += updata_mess_pos1+5+6;  //���ϵ���_FROM,6�ֽ�
									req_len =req_len - updata_mess_pos1 -5-6;
									if(edp_updata_INFO.updata_from[0]=='F')  //����·��ı���FROM�汾��0XFFFFFF����ô����Ĭ�����еĶ�����������ֻ�����̶��İ汾��
									{
										edp_updata_INFO.updata_from_hex[0] = 0xFF;
										edp_updata_INFO.updata_from_hex[1] = 0xFF;
										edp_updata_INFO.updata_from_hex[2] = 0xFF;									
									}
									else
									{
										edp_updata_INFO.updata_from_hex[0] = ((edp_updata_INFO.updata_from[0]-'0')<<4) +(edp_updata_INFO.updata_from[1]-'0');
										edp_updata_INFO.updata_from_hex[1] = ((edp_updata_INFO.updata_from[2]-'0')<<4) +(edp_updata_INFO.updata_from[3]-'0');
										edp_updata_INFO.updata_from_hex[2] = ((edp_updata_INFO.updata_from[4]-'0')<<4) +(edp_updata_INFO.updata_from[5]-'0');	
									}
								
								}
								//break;		
							case 4:
								mem_set(updata_compare_tmp,10,0x00);
								updata_compare_tmp[0] ='-';	updata_compare_tmp[1] ='T';updata_compare_tmp[2] ='O';							
								if((updata_mess_pos1=str_find(req+req_pos, req_len, updata_compare_tmp, 3)) >= 0)
								{
									mem_cpy(edp_updata_INFO.updata_to,req+req_pos+updata_mess_pos1+3,6);
									update_info_search_index = 0;
									req_pos += updata_mess_pos1+3 + 6;  //���ϵ���_TO,6�ֽ�
									req_len =req_len - updata_mess_pos1 -3 - 6;
									edp_updata_INFO.updata_to_hex[0] = ((edp_updata_INFO.updata_to[0]-'0')<<4) +(edp_updata_INFO.updata_to[1]-'0');
									edp_updata_INFO.updata_to_hex[1] = ((edp_updata_INFO.updata_to[2]-'0')<<4) +(edp_updata_INFO.updata_to[3]-'0');
									edp_updata_INFO.updata_to_hex[2] = ((edp_updata_INFO.updata_to[4]-'0')<<4) +(edp_updata_INFO.updata_to[5]-'0');									
								}
								//break;													
						}
						responseCommandToOnenet(cmdid, cmdid_len,243);							
						if((edp_updata_INFO.updata_to_hex[0]==SOFT_VER_YEAR)&&  
									(edp_updata_INFO.updata_to_hex[1]==SOFT_VER_MONTH)&&(edp_updata_INFO.updata_to_hex[2]==SOFT_VER_DAY)) //˵��Ŀǰ�汾�������µİ汾������Ҫ������
						{
							
						}						
						else if((edp_updata_INFO.updata_from_hex[0]==SOFT_VER_YEAR)&&
									(edp_updata_INFO.updata_from_hex[1]==SOFT_VER_MONTH)&&(edp_updata_INFO.updata_from_hex[2]==SOFT_VER_DAY))
						{
								fwrite_ertu_params(EEADDR_UPDATA_INFO,&edp_updata_INFO,sizeof(struct edp_updata_info));
								cmdType = 243;
						}
						else if((edp_updata_INFO.updata_from_hex[0]==0xFF)&&
									(edp_updata_INFO.updata_from_hex[1]==0xFF)&&(edp_updata_INFO.updata_from_hex[2]==0xFF))
						{
								fwrite_ertu_params(EEADDR_UPDATA_INFO,&edp_updata_INFO,sizeof(struct edp_updata_info));
								cmdType = 243;
						}
											
						break;
					case 100:   // ����onenetƽ̨��������ַ�Ͷ˿ڣ�
						responseCommandToOnenet(cmdid, cmdid_len,100);
						if((serverCommandPos = str_find(req, req_len, "\"ServerData\":{\"string\":", 23)) >= 0)
						{
							find_ip_port(req+serverCommandPos, req_len,ip_port);
						    fwrite_ertu_params(EEADDR_IP_PORT_ONENET, ip_port,6);
						}
						break;
					case 101:   //  ����������ת�������Ͷ˿�
						responseCommandToOnenet(cmdid, cmdid_len,101);
						if((serverCommandPos = str_find(req, req_len, "\"ServerData\":{\"string\":", 23)) >= 0)
						{
//							serverData = find_first_num(req+serverCommandPos, req_len);
							find_ip_port(req+serverCommandPos, req_len,ip_port);
						    fwrite_ertu_params(EEADDR_IP_PORT_UPDATE, ip_port,6);
						}
						break;
					case 102:   //  ��ȡû���ϱ��ɹ�����������
						break;
					default:
					break;
					}
					break;
					case CMDRESP:/* ������Ӧ */
					break;
					case PINGREQ:/* �������� */
					break;
					case PINGRESP:/* ������Ӧ */
						if(compare_string(pkg._data,"\xD0\x00",2) ==0)
						{
							gSystemInfo.edp_ping_state =1;
						}
#ifdef DEBUG
						system_debug_info("********already finish heart response*******");
#endif						
						break;
					break;
					default :
					break;
			}

}
//*******************************��ʼ����δ�ɹ��ϱ����ݰ��Ĵ洢���ϵ���ط�******************************************//
uint16_t nor_flash_read_data(uint16_t sector,uint16_t offset,uint8_t *data,uint16_t len);
uint16_t nor_flash_erase_page(uint16_t sector);
uint16_t nor_flash_write_data(uint16_t sector,uint16_t offset,uint8_t *data,uint16_t len);
INT8U edpStoreDataState[80]={0xFF};       //δ�ϱ��ɹ����ݴ洢
INT8U edpStoreWaitDataState[40]={0xFF};   //��ʱ�ϱ����ݴ洢
#define  STORE_RECORD_START   4000
void first_poweron_flash_rebuilt()
{
	INT8U buftmp[8]={0},state_idx =0,page=0;
	static INT8U first_poweron =0;
	if(!first_poweron)  //�״��ϵ��ع�
	{
		for(page = 0;page<FLASH_EDP_STORT_DATA_SECTOR_CNT;page++)
		{
			nor_flash_read_data(FLASH_EDP_STORT_DATA_START+page,STORE_RECORD_START,buftmp,8);
			
			for(state_idx =0;state_idx<8;state_idx++)
			{
				if(buftmp[state_idx] == 0x0)
				{
					nor_flash_erase_page(FLASH_EDP_STORT_DATA_START+page);
					mem_set(edpStoreDataState[8*page],8,0x00);
					break;
				}
				
				if(buftmp[state_idx]==0x1)
				{
					edpStoreDataState[8*page+state_idx] =0x1;
				}
				else
				{
					edpStoreDataState[8*page+state_idx] =0x0;
				}
			}
		}
	}
	first_poweron = 1;
}
//�״��ϵ����ʱ�ϱ�flash�е����ݽ����ع����ع���Ľ���ŵ�rand_wait_report_time_index�С�
void first_poweron_flash_wait_report_rebuilt()
{
	INT8U buftmp[8]={0},state_idx =0,idx=0,page =0;
	static INT8U first_poweron =0;
	if(!first_poweron)  //�״��ϵ��ع�
	{
		for(idx = 0;idx<FLASH_EDP_RAND_REPORT_STORE_CNT*8;idx++)
		{
			nor_flash_read_data(FLASH_EDP_RAND_REPORT_STORE_START+idx/8,500*(idx%8),buftmp,8);
			
			if(mem_all_is(buftmp,8,0x00)||mem_all_is(buftmp,8,0xFF))
			{
				rand_wait_report_time_index[idx] = 0;
			}
			else
			{
				rand_wait_report_time_index[idx] =*((INT64U*)&buftmp);
			}
		}
		for(page = 0;page<FLASH_EDP_RAND_REPORT_STORE_CNT;page++)
		{
			nor_flash_read_data(FLASH_EDP_RAND_REPORT_STORE_START+page,STORE_RECORD_START,buftmp,8);
			
			for(state_idx =0;state_idx<8;state_idx++)
			{
				//if(buftmp[state_idx] == 0x0)  �������������
				//{
					//nor_flash_erase_page(FLASH_EDP_RAND_REPORT_STORE_CNT+page);
					//mem_set(edpStoreWaitDataState[8*page],8,0x00);
					//break;
				//}
				if(buftmp[state_idx]==0x1)
				{
					edpStoreWaitDataState[8*page+state_idx] =0x1;
				}
				else
				{
					edpStoreWaitDataState[8*page+state_idx] =0x0;
				}
			}
		}
	}
	first_poweron = 1;
}
/**
*@brief  handle some message that doesn't report to onenet. read some message from flash.
*/
INT8U report_buf[500] ={0xFF};
void handleUnsuccessReportDataRead()
{
	INT8U pos =0,page=0,i=0;
	INT8U buftmp[8]={0x0},find_pos_state=0,find_pos=0,has_find =0,store_flag =0;
	INT32U passMS =0;
	INT8U count =0;
	INT16U reportDataLen = 500,idx =0;
	RecvBuffer storeBuf;
	if(mem_all_is(edpStoreDataState,80,0x00))
	{
		return;
	}	
	else
	{
		//********************�ҵ����ϵ��Ǹ���Ҫ�ϴ�������***************************//
		for(i=0;i<80;)
		{
			has_find = 0;
			switch (find_pos_state)
			{
				case 0:
					if(edpStoreDataState[i++] ==0x01)
					find_pos_state =1;
					break;
				case 1:
					if(edpStoreDataState[i++] ==0x00)
					find_pos_state =2;
					if(i==80)
					{
						i=0;
					}
					break;
				case 2:
					if(edpStoreDataState[i] == 0x01)
					{
						find_pos = i;
						has_find = 1;
					}
					i++;
					if(i==80)
					{
						i=0;
					}
					break;
				default:
					break;
			}
			if(has_find)  //�ҵ��ˣ�����
			{
				break;
			}
		} // ����forѭ��	
		nor_flash_read_data(FLASH_EDP_STORT_DATA_START+find_pos/8,500*(find_pos%8),report_buf,500);
		for(idx=500-1;idx>0;idx--)  //�õ�Ҫ�ظ��ı��ĵĳ��ȡ�
		{
			if(*(report_buf+idx) == 0xff)
			{
				reportDataLen --;
			}
			else
			{
				break;
			}
		}		
		storeBuf._data =report_buf;
		storeBuf._write_pos = reportDataLen;
		storeBuf._read_pos =0;
		storeBuf._capacity =500;
		if(!IsPkgComplete(&storeBuf))  //�жϴ洢��δ�ϱ��ɹ���flash�е����ݣ��������edp��ʽ�ģ���ô���ϱ�
		{
			store_flag =0x00;   //store_flag=0 ����õ�������Ѿ��ϱ���
			nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
			return;
		}

		if(GprsObj.send_len)
		{
			return  ;
		}
		if(report_buf[0] ==0xC0)   //���flash�д�����edp ping�����Ǹ�����Ӧ��Ѱ���´��Ĵ�ģ��Ͳ��ϱ��ˣ�ֱ�ӷ��أ�flash����洢��
		{
			store_flag =0x00;   //store_flag=0 ����õ�������Ѿ��ϱ���
			nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
			edpStoreDataState[find_pos] = 0;
			return  ;
		}
		gSystemInfo.save_ack =0;
#ifdef DEBUG
		system_debug_info("\r\n===========handle report  unsuccessful data in flash=========\r\n");
#endif		
		while(!gSystemInfo.save_ack)
		{
			//GprsObj.send_len =reportDataLen;
			//GprsObj.send_ptr = report_buf;
			//gprs_send_app_data();
			remote_send_app_frame(report_buf,reportDataLen);
#ifdef DEBUG
			system_debug_info("\r\n*******send ok ******\r\n");
#endif			
			passMS =system_get_tick10ms();
			while((second_elapsed(passMS)<10)&&(!gSystemInfo.save_ack)) //С��10��
			{
			//	gprs_read_app_data();
				DelayNmSec(10);
			}
				#ifdef DEBUG
				system_debug_info("\r\n*********in report report store info **********");
				#endif			
			if(count++>3)
			{
				gSystemInfo.tcp_link = 0;
				gSystemInfo.edp_login_state=0;
				gSystemInfo.dev_config_ack = 0;
				break;
				//app_softReset();
			}
		}
		if(gSystemInfo.save_ack)
		{
			store_flag =0x00;   //store_flag=0 ����õ�������Ѿ��ϱ���
			nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
			edpStoreDataState[find_pos] = 0;
		}
	}
	return ;	
}
/**
*@brief  handle some message that doesn't report to onenet. write some message to flash.
*/
//INT8U buftmp[4096] ={0};
void handleUnsuccessReportDataWrite(INT8U *buf,INT16U len)
{
	INT8U i=0,has_find =0,find_pos_state =0,find_pos =0,store_flag=0;  //����store_flag�����ڴ洢��־�������ڲ�����ļ��㡣
	//********************�ҵ����ϵ��Ǹ���Ҫ�ϴ�������***************************//
	store_flag = 1;
	for(i=0;i<80;)
	{
		has_find = 0;
		switch (find_pos_state)
		{
			case 0:
				if(edpStoreDataState[i++] ==0x01)
				find_pos_state =2;
				break;
			case 2:
				if(edpStoreDataState[i] == 0x00)
				{
					find_pos = i;
					has_find = 1;
				}
				i++;
				if(i==80)
				{
					i=0;
				}
				break;
			default:
				break;
		}
		if(has_find)  //�ҵ��ˣ�����
		{
			break;
		}
	} // ����forѭ��	
	if(find_pos ==0)  //ÿ��Ҫ����һ������д��ʱ����Ҫ��һ�µ�һ��������ݡ�
	{
		nor_flash_erase_page(FLASH_EDP_STORT_DATA_START);
#ifdef DEBUG
	system_debug_info("\r\n**************************need to ereaser a new block 0 ****************************\r\n");
#endif		
	}
	if(find_pos%8 == 7) //˵�����δ洢��λ����ÿһ��sector�����һ�顣//����֮���н���һ�������
	{
		nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,500*(find_pos%8),buf,len);
		nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
		edpStoreDataState[find_pos] = 1;
		if(find_pos==79)
		{
			store_flag = 0;
		}
		else
		{
			store_flag = find_pos/8+1;	
		}
		nor_flash_erase_page(FLASH_EDP_STORT_DATA_START+store_flag);
		mem_set(edpStoreDataState+8*store_flag,8,0x00);
#ifdef	DEBUG
		system_debug_info("\r\n**********************unsucessful store data need to change page*******************************\r\n");
#endif		
	}
	else //����Ļ�ֱ�Ӵ洢����
	{
		nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,500*(find_pos%8),buf,len);
		nor_flash_write_data(FLASH_EDP_STORT_DATA_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
		edpStoreDataState[find_pos] = 1;
#ifdef	DEBUG
		system_debug_info("\r\n**********************unsucessful store data1*******************************\r\n");
#endif	
	}
}
/**
*@brief  rebuild the warning information from flash when first power on.
*        include overflow warning ,open meter warning ,clean meter waring.
*/
void powerOnRebuildWaningInfo()
{
	INT8U tmp[4]={0};
	INT8U str1[100],str2[100];
	INT8U len =0;
 	fread_ertu_params(EEADDR_OVERFLOW_WARNING_A,tmp,4);
	if(mem_all_is(tmp,4,0xFF))
	{
		mem_set(str1,100,0xFF);
		mem_set(str2,100,0xFF);
		len =app_read_his_item(0x19010001,str1,str2,255,0,2000);  //A������ܴ��� ����Ľ��Ӧ�÷���str1�
		if(len !=0)
		{
			gSystemInfo.edp_over_flow_A =(BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_A,(INT8U*)(&gSystemInfo.edp_over_flow_A),4);	
		}

	}
	else
	{
		gSystemInfo.edp_over_flow_A =bin2_int32u(tmp);
	}

	fread_ertu_params(EEADDR_OVERFLOW_WARNING_B,tmp,4);
	if(mem_all_is(tmp,4,0xFF))
	{
		mem_set(str1,100,0xFF);
		mem_set(str2,100,0xFF);
		len = app_read_his_item(0x19020001,str1,str2,255,0,2000);  //B������ܴ��� ����Ľ��Ӧ�÷���str1�
		if(len !=0)
		{
			gSystemInfo.edp_over_flow_B =(BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_B,(INT8U*)(&gSystemInfo.edp_over_flow_B),4);
		}
	}
	else
	{
		gSystemInfo.edp_over_flow_B =bin2_int32u(tmp);
	}
	fread_ertu_params(EEADDR_OVERFLOW_WARNING_C,tmp,4);
	if(mem_all_is(tmp,4,0xFF))
	{
		mem_set(str1,100,0xFF);
		mem_set(str2,100,0xFF);
		len =app_read_his_item(0x19030001,str1,str2,255,0,2000);  //C������ܴ��� ����Ľ��Ӧ�÷���str1�
		if(len!=0)
		{
			gSystemInfo.edp_over_flow_C =(BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			fwrite_ertu_params(EEADDR_OVERFLOW_WARNING_C,(INT8U*)(&gSystemInfo.edp_over_flow_C),4);
		}
	}
	else
	{
		gSystemInfo.edp_over_flow_C =bin2_int32u(tmp);
	}		
	fread_ertu_params(EEADDR_OPENMETER_WARNING,tmp,4);
	if(mem_all_is(tmp,4,0xFF))
	{
		mem_set(str1,100,0xFF);
		mem_set(str2,100,0xFF);
		len =app_read_his_item(0x03300D00,str1,str2,255,0,2000);  //����� ����Ľ��Ӧ�÷���str1�
		if(len !=0)
		{
			gSystemInfo.edp_open_meter =(BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			fwrite_ertu_params(EEADDR_OPENMETER_WARNING,(INT8U*)(&gSystemInfo.edp_open_meter),4);
		}
	}
	else
	{
		gSystemInfo.edp_open_meter =bin2_int32u(tmp);
	}	
	fread_ertu_params(EEADDR_CLEANNMETER_WARNING,tmp,4);
	if(mem_all_is(tmp,4,0xFF))
	{
		mem_set(str1,100,0xFF);
		mem_set(str2,100,0xFF);
		len =app_read_his_item(0x03300100,str1,str2,255,0,2000);  //���� ����Ľ��Ӧ�÷���str1�
		if(len !=0)
		{
			gSystemInfo.edp_clean_meter =(BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			fwrite_ertu_params(EEADDR_CLEANNMETER_WARNING,(INT8U*)(&gSystemInfo.edp_clean_meter),4);
		}
	}
	else
	{
		gSystemInfo.edp_clean_meter =bin2_int32u(tmp);
	}

}
/**
*@brief  ÿ��30S����edpЭ��ĸ澯�¼�
*/
void warning_report_handler(INT16U *warningMask)
{
	INT8U loop =0;
	INT8U len =0;
	INT8U str1[100] ={0},str2[100]={0};
	INT32U cnt;
	if(*warningMask ==0)  //˵������Ҫ�ϱ��澯��
	{
		return ;
	}
	for(loop=0;loop<5;loop++)
	{
		if(get_bit_value(warningMask,1,loop))
		{
			break;
		}
	}
	mem_set(str1,100,0xFF);
	mem_set(str2,100,0xFF);
	switch(loop)
	{
		case 0:
		    cnt =0;
			len =app_read_his_item(0x19010001,str1,str2,255,0,2000);  //A������ܴ��� ����Ľ��Ӧ�÷���str1�
			if(len)
			{
				cnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			}
			if(cnt != gSystemInfo.edp_over_flow_A) //����������ĸ澯����������flash�д洢�ĸ澯��������ô��Ҫ�ϱ�ƽ̨�ˡ�
			{
				//warning_report_cmd =1;
				report_warning_to_onenet(1,0);
			}
			break;
		case 1:
			cnt =0;
			len =app_read_his_item(0x19020001,str1,str2,255,0,2000);  //B������ܴ��� ����Ľ��Ӧ�÷���str1�
			if(len)
			{
				cnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			}
			if(cnt != gSystemInfo.edp_over_flow_B) //����������ĸ澯����������flash�д洢�ĸ澯��������ô��Ҫ�ϱ�ƽ̨�ˡ�
			{
				//warning_report_cmd = 1;
				report_warning_to_onenet(1,0);
			}			
			break;
		case 2:
			cnt =0;
			len =app_read_his_item(0x19030001,str1,str2,255,0,2000);  //C������ܴ��� ����Ľ��Ӧ�÷���str1�
			if(len)
			{
				cnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			}
			if(cnt !=gSystemInfo.edp_over_flow_C) //����������ĸ澯����������flash�д洢�ĸ澯��������ô��Ҫ�ϱ�ƽ̨�ˡ�
			{
				//warning_report_cmd = 1;
				report_warning_to_onenet(1,0);
			}
			break;
		case 3:
			cnt =0;
			len =app_read_his_item(0x03300D00,str1,str2,255,0,2000);  //������ܴ��� ����Ľ��Ӧ�÷���str1�
			if(len)
			{
				cnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			}
			if(cnt !=gSystemInfo.edp_open_meter) //����������ĸ澯����������flash�д洢�ĸ澯��������ô��Ҫ�ϱ�ƽ̨�ˡ�
			{
				//warning_report_cmd = 2;
				report_warning_to_onenet(2,0);
			}		
			break;
		case 4:
			cnt =0;
			len =app_read_his_item(0x03300100,str1,str2,255,0,2000);  //��������ܴ��� ����Ľ��Ӧ�÷���str1�
			if(len)
			{
				cnt = (BCD2byte(str1[2])*10000) + (BCD2byte(str1[1])*100) +(BCD2byte(str1[0]));
			}
			if(cnt !=gSystemInfo.edp_clean_meter) //����������ĸ澯����������flash�д洢�ĸ澯��������ô��Ҫ�ϱ�ƽ̨�ˡ�
			{
				//warning_report_cmd = 3;
				report_warning_to_onenet(3,0);
			}		
			break;
		default:
			break;
	}
	clr_bit_value(warningMask,1,loop);
	return ;
}

/**
 * �ϱ��澯��Ϣ��onenetƽ̨����ƽ̨�·��ĸ澯��Ӧ���д���
 */
INT8U report_warning_to_onenet(INT8U flag,INT8U value)
{
	INT8U count =0 ;
	INT32U passMS=0;
	EdpPacket* pkg;
	gSystemInfo.save_ack =0;
	if(!gSystemInfo.tcp_link) //���û�е�¼�ɹ�����ôֱ�Ӵ洢
	{
#ifdef DEBUG			 
		  	system_debug_info("\r\n***************store hold day energy****************\r\n");
#endif
#ifdef 	 __BEIJING_ONENET_EDP__
		cjsonEventWarningPack(OneNetData,flag);
#endif
#ifdef __ZHEJIANG_ONENET_EDP__
		cjsonEventWarningPack(OneNetData,flag,value);
#endif
		pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������
		return 1;
	}
	//if(GprsObj.send_len)
	//{
		//return  0 ;
	//}
	mem_set(OneNetData,1024, 0x00);  	
//	tcp_client_socket_id =0;   //9600����ǰ���Լ���õģ�һ����1
	while(!gSystemInfo.save_ack)
	{
#ifdef 	 __BEIJING_ONENET_EDP__
		cjsonEventWarningPack(OneNetData,flag);
#endif
#ifdef __ZHEJIANG_ONENET_EDP__
		cjsonEventWarningPack(OneNetData,flag,value);
#endif
		EdpPacket* pkg =PacketSavedataJson2(NULL, OneNetData, 1, 1);
		remote_send_app_frame(pkg->_data,pkg->_write_pos);
		passMS =system_get_tick10ms();
		while((second_elapsed(passMS)<30)&&(!gSystemInfo.save_ack)) //С��10��
		{
			DelayNmSec(10);
			//gprs_read_app_data();
		}
#ifdef DEBUG
		system_debug_info("\r\n*********in report warning info **********");
#endif		
		if(count++>3)
		{
			handleUnsuccessReportDataWrite(pkg->_data,pkg->_write_pos);//���û�ϴ��ɹ���ô�洢������			
			gSystemInfo.tcp_link = 0;
			gSystemInfo.edp_login_state =0;
			gSystemInfo.dev_config_ack = 0;
			break;		
		}
	}
	return 1;
}

/**
*@brief  handle some message that doesn't report to onenet. write some message to flash.
*�����ʱ�ϱ��������ϱ����ݴ洢��flash���ȴ�ʱ��㵽��Ȼ���ϱ���
*/
INT8U tmp[4096] ={0};
void handle_wait_report_data_write(INT8U *buf,INT16U len)
{
	INT8U i=0,has_find =0,find_pos_state =0,find_pos =0,store_flag=0;  //����store_flag�����ڴ洢��־�������ڲ�����ļ��㡣
	INT64U report_time = 0;
	//********************�ҵ����ϵ��Ǹ���Ҫ�ϴ�������***************************//
	store_flag = 1;
	for(i=0;i<40;)
	{
		has_find = 0;
		switch (find_pos_state)
		{
			case 0:
			if(edpStoreWaitDataState[i++] ==0x01)
			find_pos_state =2;
			break;
			case 2:
			if(edpStoreWaitDataState[i] == 0x00)
			{
				find_pos = i;
				has_find = 1;
			}
			i++;
			if(i==40)
			{
				i=0;
			}
			break;
			default:
			break;
		}
		if(has_find)  //�ҵ��ˣ�����
		{
			break;
		}
	} // ����forѭ��
	if(find_pos ==0)  //ÿ��Ҫ����һ������д��ʱ�򣬡�
	{
//		nor_flash_erase_page(FLASH_EDP_RAND_REPORT_STORE_START);
		#ifdef DEBUG
		system_debug_info("\r\n**************************need to ereaser a new block 0 ****************************\r\n");
		#endif
	}
	if(find_pos%8 == 7) //˵�����δ洢��λ����ÿһ��sector�����һ�顣//����֮���н���һ�������
	{
		nor_flash_write_data(FLASH_EDP_RAND_REPORT_STORE_START+find_pos/8,500*(find_pos%8),buf,len);	
		nor_flash_read_data(FLASH_EDP_RAND_REPORT_STORE_START+find_pos/8,500*(find_pos%8),tmp,500);	
		system_debug_info("===================================");
		system_debug_data(tmp,500);
		system_debug_info("===================================");		
		nor_flash_write_data(FLASH_EDP_RAND_REPORT_STORE_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
		mem_cpy((INT8U*)&report_time,buf,8);
		rand_wait_report_time_index[find_pos] =report_time;
		edpStoreWaitDataState[find_pos] = 1;
		if(find_pos==39)
		{
			store_flag = 0;
		}
		else
		{
			store_flag = find_pos/8+1;
		}
		nor_flash_erase_page(FLASH_EDP_RAND_REPORT_STORE_START+store_flag);
		mem_set(edpStoreWaitDataState+8*store_flag,8,0x00);
		#ifdef	DEBUG
		system_debug_info("\r\n**********************unsucessful store data need to change page*******************************\r\n");
		#endif
	}
	else //����Ļ�ֱ�Ӵ洢����
	{
		nor_flash_write_data(FLASH_EDP_RAND_REPORT_STORE_START+find_pos/8,500*(find_pos%8),buf,len);	
		nor_flash_read_data(FLASH_EDP_RAND_REPORT_STORE_START+find_pos/8,500*(find_pos%8),tmp,500);
		system_debug_info("===================================");
		system_debug_data(tmp,500);
		system_debug_info("===================================");		
		nor_flash_write_data(FLASH_EDP_RAND_REPORT_STORE_START+find_pos/8,STORE_RECORD_START + (find_pos%8),&store_flag,1);
		mem_cpy((INT8U*)&report_time,buf,8);
		rand_wait_report_time_index[find_pos] =report_time;
		edpStoreWaitDataState[find_pos] = 1;
		#ifdef	DEBUG
		system_debug_info("\r\n**********************unsucessful store data*******************************\r\n");
		#endif
	}
}
/**
*@brief  handle some message that doesn't report to onenet. read some message from flash.
*����ʱ�ϱ��洢�������ϱ���onenetƽ̨������ֻ��Ҫ����ȫ�ֱ���rand_wait_report_time_index ѡ����Ҫ�ϱ����������ɡ�
*/
//INT8U report_buf[500] ={0xFF};
void handle_wait_report_data_read()
{
	INT8U pos =0,page=0,i=0;
	INT8U buftmp[8]={0x0},find_pos_state=0,find_pos=0,has_find =0;
	INT64U store_flag =0;
	INT32U passMS =0;
	INT8U count =0;
	INT16U reportDataLen = 500,idx =0;
	INT8U index=0;
	INT64U report_time =0;
	DateTime dt;
	tpos_datetime(&dt);
	report_time = getPassedSeconds(&dt,2000);
	for(index =0;index<40;index++)
	{
		if((rand_wait_report_time_index[index]<report_time)  &&(rand_wait_report_time_index[index] !=0))
		{
			break;
		}
	}
	if(index ==40)  //��rand_wait_report_time_index ������һȦû�ҵ������ϱ��ģ���ֱ���˳���!!!!!!
	{
		return;
	}
		nor_flash_read_data(FLASH_EDP_RAND_REPORT_STORE_START+index/8,500*(index%8),report_buf,500);
		for(idx=500-1;idx>0;idx--)  //�õ�Ҫ�ظ��ı��ĵĳ��ȡ�
		{
			if(*(report_buf+idx) == 0xff)
			{
				reportDataLen --;
			}
			else
			{
				break;
			}
		}
		if(GprsObj.send_len)
		{
			return  ;
		}
	if(gSystemInfo.tcp_link ==0)  //���ģ�鲻���ߣ���ô��ֱ�Ӵ洢��δ�ϱ�����
	{
#ifdef DEBUG
		system_debug_info("==================owning outof line ,so store info ======================");
#endif
		handleUnsuccessReportDataWrite(report_buf+8, reportDataLen -8);//���û�ϴ��ɹ���ô�洢������
		store_flag =0x00;   //store_flag=0 ����õ�������Ѿ��ϱ���
		nor_flash_write_data(FLASH_EDP_RAND_REPORT_STORE_START+idx/8,500*(index%8),&store_flag,8);
		rand_wait_report_time_index[index] = 0;		
		return;
	}		
		gSystemInfo.save_ack =0;
		while(!gSystemInfo.save_ack)
		{
			//GprsObj.send_len =reportDataLen -8;  //���ǵ���ʼ8���ֽ�������ʱ��
			//GprsObj.send_ptr = report_buf+8;     //���ǵ���ʼ8���ֽ�������ʱ��
			//gprs_send_app_data();
			remote_send_app_frame(report_buf+8,reportDataLen -8);
			#ifdef DEBUG
			system_debug_info("\r\n*******send ok ******\r\n");
			#endif
			passMS =system_get_tick10ms();
			while((second_elapsed(passMS)<30)&&(!gSystemInfo.save_ack)) //С��10��
			{
				DelayNmSec(10);
				//gprs_read_app_data();
			}
			#ifdef DEBUG
			system_debug_info("\r\n*********in report wait report store info **********");
			#endif
			if(count++>3)
			{

				gSystemInfo.tcp_link = 0;
				gSystemInfo.edp_login_state=0;
				gSystemInfo.dev_config_ack = 0;
				handleUnsuccessReportDataWrite(GprsObj.send_ptr,GprsObj.send_len);//���û�ϴ��ɹ���ô�洢������				
				break;
				//app_softReset();
			}
		}
		{  //���ڰ��ն��������ʱ�ϱ������ݣ������ϱ��ɹ���񣬶���Ҫ�������ʱ����flash�г�ȥ���������ǽ�ʱ��λ�����㡣������߼�����Ǹ���ʱ��Ĵ�С�������ݵ��ϱ���
			//edpStoreWaitDataState �еı�־λֻ�����ڱ�־flash�������������д����flash�������Ѿ���
		store_flag =0x00;   //store_flag=0 ����õ�������Ѿ��ϱ���
		nor_flash_write_data(FLASH_EDP_RAND_REPORT_STORE_START+idx/8,500*(index%8),&store_flag,8);
		rand_wait_report_time_index[index] = 0;
		}
	return ;
}
//�õ������ʱ,�ñ�ź�ʱ�������õ���
INT16U getrand()
{
	INT16U randNum =0;
	INT16U systemtic=system_get_tick10ms();
	tagDatetime time;
	os_get_datetime(&time);
	
	randNum = systemtic^(time.year)^(time.month)^(time.hour)^(time.minute)^(time.second)^(time.msecond_h<<8+time.msecond_l);
	
	randNum =randNum^(gSystemInfo.meter_no[0]+(gSystemInfo.meter_no[1]<<8));
	randNum =randNum^(gSystemInfo.meter_no[2]+(gSystemInfo.meter_no[3]<<8));
	randNum =randNum^(gSystemInfo.meter_no[4]+(gSystemInfo.meter_no[5]<<8));
	return randNum;
}
//��һ���ϵ��¼onenet��Ҫ�ϱ�����Ϣ��
void first_login_onenet()
{
	INT8U flag =0;
	INT32U passMS=0;
	INT16U Gprs_recv_cnt =0;
	if(!gSystemInfo.dev_config_ack&&gSystemInfo.edp_login_state&&!gSystemInfo.edp_first_poweron)
	{
		
		passMS =system_get_tick10ms();
		Gprs_recv_cnt = GprsObj.recv_pos ;
		while((second_elapsed(passMS)<10))
		{
			//gprs_read_app_data();
			if(GprsObj.recv_pos != Gprs_recv_cnt)
			{
				passMS =system_get_tick10ms();
				Gprs_recv_cnt = GprsObj.recv_pos;
			}
			DelayNmSec(1000);
		
	#ifdef DEBUG
				system_debug_data(&Gprs_recv_cnt,2);
				system_debug_info("in 1111111");
				system_debug_data(&GprsObj.recv_pos,2);
	#endif				
		}
		#ifdef DEBUG
		system_debug_info("\r\n =================prepare to report  configure when power on============r\n");
		#endif		
		reportConfigToOnenet(); //�ϱ��豸������Ϣ��
		DelayNmSec(1000);
		#ifdef DEBUG
		system_debug_info("\r\n *********already report configration******\r\n");
		#endif
		reportHoldDayEnergyToOnenet(1); //
		DelayNmSec(1000);
		#ifdef DEBUG
		system_debug_info("\r\n *********already report hold day energy ******\r\n");
		#endif
		reportHoldDayEnergyToOnenet(0);
		DelayNmSec(1000);
		#ifdef DEBUG
		system_debug_info("\r\n *********already report real energy ******\r\n");
		#endif
		reportyEnergyBlockToOnenet();
		#ifdef DEBUG
		system_debug_info("\r\n *********already report  energy block******\r\n");
		#endif
#ifdef __ZHEJIANG_ONENET_EDP__		
		reportMonthDayEnergyToOnenet();
		#ifdef DEBUG
		system_debug_info("\r\n *********already report  month day ******\r\n");
		#endif
#endif					
		DelayNmSec(1000);
		if(gSystemInfo.update==0xDD)  //ʧ��
		{
			flag=0xFF;
			fwrite_ertu_params(EEADDR_UPDATE_FLAG,&flag,1);
			responseUpdataResToOnenet(1);
		}
		if(gSystemInfo.update==0xCC)  //�����ɹ�
		{
			responseUpdataResToOnenet(0);
			flag=0xFF;
			fwrite_ertu_params(EEADDR_UPDATE_FLAG,&flag,1);
		}
		gSystemInfo.edp_first_poweron =1;
	}
}
//��������˺���Ҫ��������tcp��ʱ��
void second_connet_onenet()
{
	if(gSystemInfo.edp_second_connet)
	{
		
		reportConfigToOnenet(); //�ϱ��豸������Ϣ��
		gSystemInfo.edp_second_connet =0;
	}
}
#ifdef __ZHEJIANG_ONENET_EDP__
//˳��ִ�� instanceΪ���ƫ��λ��
const tagResourceIdEvent  RESOURCE_ID_EVENT[] =
{
	//item_times,   check_offset1,  check_len1,     item_end,       check_offset2,  check_len2,    instance
	#if (FILEFLAG_TYPE==FILEFLAG_SGP_GMTER_G55)
	{0x03060000,    0,              3,              0x03060001,     6,              6,              2}, //ͣ�ϵ��¼���������Դ��
	{0x03110000,    0,              3,              0x03110001,     6,              6,              1}, //ͣ�ϵ��¼����е磩
	{0x1B010001,    0,              3,              0x1B011201,     0,              6,              3}, //�й����ʷ���  ,����ָ����A��
	//{0x03110000,    0,              3,              0x03110001,     0,              6,              4}, //�޹����ʷ���

	{0x19010001,    0,              3,              0x19012101,     0,              6,              5}, //A�����

	{0x1A010001,    0,              3,              0x1A012101,     0,              6,              8}, //A�����
	//{ 0x040005FF,  14,  0xFFFFFFFF,     0xFFFFFFFF,      	3200,       0,      5500,              }, //ʱ�ӵ��Ƿѹ
	#else if(FILEFLAG_TYPE==FILEFLAG_TGP_GMTER_G55)
	{0x03060000,    0,              3,              0x03060001,     6,              6,              2}, //ͣ�ϵ��¼���������Դ��
	{0x03110000,    0,              3,              0x03110001,     6,              6,              1}, //ͣ�ϵ��¼����е磩
	{0x1B010001,    0,              3,              0x1B011201,     0,              6,              3}, //�й����ʷ���  ,����ָ����A��
	//{0x03110000,    0,              3,              0x03110001,     0,              6,              4}, //�޹����ʷ���

	{0x19010001,    0,              3,              0x19012101,     0,              6,              5}, //A�����
	{0x19020001,    0,              3,              0x19022101,     0,              6,              6}, //B�����
	{0x19030001,    0,              3,              0x19032101,     0,              6,              7}, //C�����

	{0x1A010001,    0,              3,              0x1A012101,     0,              6,              8}, //A�����
	{0x1A020001,    0,              3,              0x1A022101,     0,              6,              9}, //B�����
	{0x1A030001,    0,              3,              0x1A032101,     0,              6,              10}, //C�����
	//{ 0x040005FF,  14,  0xFFFFFFFF,     0xFFFFFFFF,      	3200,       0,      5500,              }, //ʱ�ӵ��Ƿѹ
	#endif
};

void  app_check_meter_record_event(INT16U* warningMask)
{
	static INT32U last_tick=0;
	INT8U idx,offset=0,loop=0;
	INT8U request[30]={0},resp[80]={0};
	INT8U occur=0,state=0;
	tagResourceIdEvent* event;
	tagEVENT event_for,event_cur;

	if(*warningMask == 0)
	{
		return;
	}
	for(loop=0;loop<16;loop++)
	{
		if(get_bit_value(warningMask,2,loop))
		{
			break;
		}
	}
	for(idx=loop;idx<=loop;idx++)	   //��Ȼ�˴���һ��ѭ��������ÿ��ֻ��һ����
	{
		event = &RESOURCE_ID_EVENT[idx];
		mem_set(event_for.value,sizeof(event_for),0x00);
		mem_set(event_cur.value,sizeof(event_cur),0x00);
		//ʹ���б��е�(instance-1)*4��Ϊƫ�Ƶ�ַ ʱ�ӵ��Ƿѹ����Ҫ�洢
		fread_ertu_params(EEADDR_LAST_ENENT_TIMES+(event->instance-1)*4,event_for.value,4);
		
		if(app_read_his_item(event->item_times,request,resp,255, NULL,2000)==0)
		{
			continue;
		}
		mem_cpy(event_cur.times,request+event->check_offset1,event->check_len1);
		if(check_is_all_FF(event_for.times,3))
		{
			mem_cpy(event_for.times,event_cur.times,sizeof(tagEVENT));
			event_for.state=0x55;
			fwrite_ertu_params(EEADDR_LAST_ENENT_TIMES+(event->instance-1)*4,event_for.value,4);
			continue;
		}

		if(compare_string(event_for.times,event_cur.times,3)==0)
		{
			occur = 0;
			if(event_for.state!=0xAA)
			{
				continue;
			}
		}
		else
		{
			occur = 1;
		}

		if(app_read_his_item(event->item_end,request,resp,255,NULL,2000)==0)
		{
			continue;
		}
		//����ʱ����ЧΪ�Ѿ��ָ�
		if(is_valid_bcd(request+event->check_offset2,event->check_len2))
		{
			event_cur.state=0x55; 
			state = 0;
		}
		//����ʱ����ЧΪ���ڷ���
		else
		{
			event_cur.state=0xAA;
			state = 1;
		}
		
		//������� �� �����ָ��¼�
		if(occur||(event_for.state!=event_cur.state))
		{
			//report
			//���������� �Ѿ��ָ���Ҫ�ϱ������ͻָ�
			if((occur)&&(event_cur.state==0x55))
			{
				//sprintf((char*)resp,"AT+MIPLNOTIFY=0,0,3200,%d,5500,5,1,\"1\",0,0,%d\r\n",event->instance,gSystemInfo.ackid);
				//remote_send_app_frame(resp,str_cmdlen(resp));
				report_warning_to_onenet(event->instance,1);
			}
			//sprintf((char*)resp,"AT+MIPLNOTIFY=0,0,3200,%d,5500,5,1,\"%d\",0,0,%d\r\n",event->instance,state,gSystemInfo.ackid);
			//remote_send_app_frame(resp,str_cmdlen(resp));
			 report_warning_to_onenet(event->instance,state);
			mem_cpy(event_for.value,event_cur.value,sizeof(event_for));
			fwrite_ertu_params(EEADDR_LAST_ENENT_TIMES+(event->instance-1)*4,event_for.value,4);
		}
	}
	clr_bit_value(warningMask,2,loop);
	return;

}
void app_check_meter_status(void)
{
	static INT32U last_tick;
	INT16U ackid =0;
	INT16U res_len=0;
	INT8U for_status[26]={0},cur_status[26]={0},change[14]={0};
	INT8U cur_flag_chg[14]={0},request[100]={0};
	INT8U *resp,idx;
	tagDatetime datetime;
		
	fread_ertu_params(EEADDR_LAST_STATUS,for_status,sizeof(for_status));
	if(for_status[20]==0xFF)
	{
		mem_set(for_status,sizeof(for_status),0x00);
		fwrite_ertu_params(EEADDR_LAST_STATUS,for_status,sizeof(for_status));
	}
	
	resp = request+30;//Ԥ��30�������ĳ���
	if(app_read_his_item(0x040005FF,request,resp,255, NULL,2000)==0)
	{
		return;
	}
	else
	{
		memcpy(cur_status,request,20);
		if((cur_status[0]&0x04) != (for_status[0]&0x04))
		{
			OSMutexPend(&SIGNAL_SYSTEMINFO);
			gSystemInfo.ackid++;
			if(gSystemInfo.ackid>32767)
			{
				gSystemInfo.ackid =0;
			}
			OSMutexFree(&SIGNAL_SYSTEMINFO);
			
			mem_cpy(cur_status,request,sizeof(for_status));
			fwrite_ertu_params(EEADDR_LAST_STATUS,cur_status,sizeof(cur_status));
			if(cur_status[0]&0x04)
			{
				report_warning_to_onenet(0,1);
			}
			else
			{
				report_warning_to_onenet(0,0);
			}
		}
		//
		#ifdef  FILEFLAG_TGP_GMTER_G55
		if((cur_status[0]&0x20) != (for_status[0]&0x20))
		{
			OSMutexPend(&SIGNAL_SYSTEMINFO);
			gSystemInfo.ackid++;
			if(gSystemInfo.ackid>32767)
			{
				gSystemInfo.ackid =0;
			}
			OSMutexFree(&SIGNAL_SYSTEMINFO);
			
			mem_cpy(cur_status,request,sizeof(for_status));
			fwrite_ertu_params(EEADDR_LAST_STATUS,cur_status,sizeof(cur_status));
			if(cur_status[0]&0x04)
			{
				report_warning_to_onenet(4,1);
			}
			else
			{
			  report_warning_to_onenet(4,0);
			}
			//sprintf(request,"AT+MIPLNOTIFY=0,0,3200,4,5500,5,1,\"0\",0,0,%d\r\n",ackid);
			//remote_send_app_frame(request,str_cmdlen(request));
			
		}
		#endif
	}
	
	
}
#endif
void vmeter_edp_warning_report()
{

	static INT16U warningMask =0;


	DateTime dt;
	static INT8U day_old=0,minute_old =0,already_poweron_3minute =0,month_old =0;
	static INT8U report_mask =0;   //�ϱ�״̬���롣
	static INT8U report_loop =0;
	INT8U tmp=0;
	while(!day_old)
	{
		DelayNmSec(10);
		tpos_datetime(&dt);
		day_old =dt.day;
		minute_old =dt.minute;
		month_old =dt.month;
	}
	if(gSystemInfo.tcp_link)
	{
		if((gSystemInfo.update!=0xAA) &&(gSystemInfo.edp_login_state == 1))//��������״̬
		{
			if((minute_elapsed(edp_ping_start_time)>=3))
			{
				#ifdef DEBUG
				system_debug_info("\r\n ********now into edp ping:************");
				#endif
				if(edp_ping())
				{
					edp_ping_start_time =system_get_tick10ms();
				}
			}
		}
	}
	//���ڹ�����ѹ����Ļ�Ӧ��30S��һ�Ρ�
	if((gSystemInfo.update!=0xAA) &&(second_elapsed(edp_warningTime)>30))
	{
#ifdef __BEIJING_ONENET_EDP__
		warningMask = 1<<0 | 1<<1|1<<2| 1<<3|1<<4|1<<5;
#endif
#ifdef __ZHEJIANG_ONENET_EDP__
	#if (FILEFLAG_TYPE==FILEFLAG_TGP_GMTER_G55)
		warningMask = 0x1FF;   //��������볤��Ϊ9��bit����
	#endif
	#if (FILEFLAG_TYPE==FILEFLAG_SGP_GMTER_G55)
		warningMask = 0x1F;   //��������볤��5��bit����
	#endif
#endif

		 
		edp_warningTime =system_get_tick10ms();
		warning_report_cmd =0;
	}
	if((gSystemInfo.update!=0xAA)&&(gSystemInfo.tcp_link==1))
	{
#ifdef __BEIJING_ONENET_EDP__
		warning_report_handler(&warningMask);
#endif
#ifdef __ZHEJIANG_ONENET_EDP__
		app_check_meter_record_event(&warningMask);
#endif
	}
	if(gSystemInfo.update!=0xAA)  //ֻ�в���edp������״̬�Ž���
	{
		first_login_onenet();   //��һ��ע��onenetƽ̨ʱ�����ϱ������ݡ�
		second_connet_onenet();  //֮����������²�����Ҫ�ϱ������ݡ�
		tpos_datetime(&dt);
		if((abs(dt.minute -minute_old)>2)&&(!already_poweron_3minute))
		{
			already_poweron_3minute =1;  //ȷ���Ѿ�����2�����ˡ�
		}
		if(already_poweron_3minute)
		{
			if(gSystemInfo.edp_login_state)    //�����������˲����Ѿ���¼�������˿����ϱ���Щ�洢��δ�ϱ�����Ϣ�ˡ�
			{
				gSystemInfo.edp_enable_report_store =1;
			}
			if(!gSystemInfo.edp_login_state)
			{
				gSystemInfo.edp_enable_report_store = 0;
			}
			if(dt.day !=day_old)
			{
				report_mask|=0x1<<0;  //�ն���
				report_loop =0;
				day_old =dt.day;
				#ifdef DEBUG
				system_debug_info("\r\n***************need to report interval day hold *************\r\n");
				#endif
			}
			if(abs(dt.minute -minute_old) >= gSystemInfo.edp_report_interval)
			{
				report_mask|=0x1<<1;  //ʵʱ����
				report_mask|=0x1<<2;  //�������ݿ�
				minute_old = dt.minute ;
				report_loop=0;
				#ifdef DEBUG
				system_debug_info("\r\n***************need to report interval *************\r\n");
				#endif
			}
#ifdef	__ZHEJIANG_ONENET_EDP__
		 if(dt.month !=month_old)
		 {
			 report_mask|=0x1<<3;  //�¶����ϱ�
			 report_loop =0;
			 month_old =dt.month;
			 #ifdef DEBUG
			 system_debug_info("\r\n***************need to report interval month hold *************\r\n");
			 #endif
		 }
#endif		
		}
		for(report_loop ;report_loop <4;)
		{
			if(!autoReportType)   //�����Ӻ�����Ѿ���¼���Ǿͷ���376.3������,���û�е�¼��վ����ô��Ӧ��ֱ�Ӵ洢��
			{
				switch(report_mask&(0x1<<report_loop))
				{
					case  0x1:   //�����ϱ��ն���
						cmdType =0;
						wait_report_flag = 1;
						reportHoldDayEnergyToOnenet(1); //�ն���
						wait_report_flag = 0;
						autoReportType = 0;
						break;
					case 0x02:   //�����ϱ�ʵʱ����
						cmdType =0;
						wait_report_flag = 1;
						reportHoldDayEnergyToOnenet(0);
						wait_report_flag = 0;
						autoReportType = 0;
						break;
					case 0x04:    //�����ϱ�������Ϣ��
						cmdType =0;
						wait_report_flag = 1;
						reportyEnergyBlockToOnenet();
						wait_report_flag = 0;
						autoReportType = 0;
						break;
					case 0x08:
						cmdType =0;
						wait_report_flag = 1;
						reportMonthDayEnergyToOnenet();
						wait_report_flag = 0;
						autoReportType = 0;
						break;						
					default:
					break;
				}
				report_mask &=~(0x01<<report_loop);
			}
			else  //���û�е�¼�ɹ�����ôֱ�ӽ��д洢��
			{
			
			}
			report_loop ++;
			if(report_loop >=4)
			{
				report_loop =0;
			}
			break;
		}
		switch(cmdType)   //�Դ�app_protocol_onenet�������������ݽ��н���
		{
			case 7:      //��ǰ����
				reportHoldDayEnergyToOnenet(0);
				cmdType =0;
				break;
			case 11:
				reportHoldDayEnergyToOnenet(1); //�ն���
				cmdType =0;
				break;
			case 12:    //645͸��������ء�
				report645CmdResultToOnenet();
				cmdType =0;
				break;
#ifdef __ZHEJIANG_ONENET_EDP__			
			case 13:    //645͸��������ء�
				reportMonthDayEnergyToOnenet();
				cmdType =0;
				break;	
#endif							
			case 14:    //���������ݿ�
				reportyEnergyBlockToOnenet();
				cmdType =0;
				break;
			case 243:   //�յ���������������ϱ���ƽ̨�յ���������fota 2
				responseUpdataResToOnenet(2);  //�յ���������
				cmdType =0;
				//DelayNmSec(5000);
				tmp=0xAA;
				fwrite_ertu_params(EEADDR_UPDATE_FLAG,&tmp,1);
				app_softReset();
				break;
			default:
				break;
		}
		if(gSystemInfo.edp_enable_report_store)  //������Щû���ϴ��ɹ������ݡ�
		{
			if(gSystemInfo.tcp_link)
			{
				handleUnsuccessReportDataRead();  //δ�ϱ��ɹ����������ϱ�������¼��
			}

			handle_wait_report_data_read();  //�ȴ��ϱ��Ķ��У�������ȴ��ϱ���
		}
	}//!!!end if(gSystemInfo.update!=0xAA)
}