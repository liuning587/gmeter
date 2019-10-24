#ifndef __TOPS_FUN_TOOLS_H__
#define __TOPS_FUN_TOOLS_H__

#include "dev_file.h"
//��������壬Ŀǰ����376.1�ļ�������ļ���ʶ����
#define FILEFLAG_CLEAR            0x00    //��������ļ�
#define FILEFLAG_DEV_PROG         0x01    //�ն������ļ�
#define FILEFLAG_LOG			  0x02    //log�ļ���
#define FILEFLAG_CONFIG			  0x03    //�ű������ļ���
#define FILEFLAG_OUT_FLASH		  0x04    //�Զ����ɵ�flash�ļ���


#define FILEFLAG_SGP_GMTER_G55       0xFA    //����G��G55����
#define FILEFLAG_TGP_GMTER_G55       0xFB    //����G��G55����

#define FILEFLAG_TYPE   FILEFLAG_TGP_GMTER_G55  //���豸������G��G55����

#if    FILEFLAG_TYPE == FILEFLAG_SGP_GMTER_G55
	#define   __METER_SIMPLE__
#else
	//#define 
#endif	

//MXоƬ��boot����ʱ�������¹滮
#define FLADDR_PROG           0
//�����������ʼ��ַ
#define FLADDR_PROG_START          (FLADDR_PROG+512)    //524*1024=536576
//����������˵��(256�ֽڣ�ǰ128�ֽ����������Ƿ���д����128�ֽڱ�ʾ�Ƿ���д�ɹ���)
#define FLADDR_PROG_FLAG           (FLADDR_PROG + (FLASH_SECTOR_UPDATE_END-1)*4096)      //128�ֽ�
//���������������,��дǰΪ0xFF,��д�󣬱�Ϊ0x669955AA
//������������� PROG_FLAG Ϊ669955AA   PROG_FINISH��0xFF
#define FLADDR_PORG_FINISH         (FLADDR_PROG_FLAG  + 128)  //4�ֽ�
//#define FLADDR_PORG_HEADER         (FLADDR_PROG_FLAG  + 256)  //4�ֽ�
#define FLADDR_PORG_HEADER FLADDR_PROG

BOOLEAN read_datfile_header(DATHEADER *header, INT8U *buffer);
BOOLEAN check_update_header(DATHEADER *header,INT8U fileflag);
void update_download_record(INT16U sec_count,INT16U cur_sec,INT16U sec_len);
BOOLEAN check_download_record(INT8U *buffer);
BOOLEAN datfile_update(DATHEADER *header, INT8U *resetFlag,INT8U *buffer);
BOOLEAN check_datfile(DATHEADER *header, INT8U *buffer);
void decode_update(INT8U *buffer,INT16U len,INT32U addr);
BOOLEAN usb_file_process_log(void);
//INT8U log_data_buffer[4096];
//INT16U log_data_buffer_len;
struct output_flash
{
	INT8U  flash_out_flag;  //���ϱ�־����ʾ��Ҫ��flash ��
	INT16U begin_sector;
	INT16U read_len;
	INT16U  sector_pos;	
}output_flash_sector;

#endif
 