#define		ADSB_API_EXPORTS
#include "acars_decode.h"
#include <string.h>
#include "acars_utility.h"

#define SYN 0x16
#define SOH 0x01

/* CCITT 16 CRC */
#define POLY 0x1021

//开启统计
#define STATISTICS_FILE (0)

acars_decoder::acars_decoder()
	:_all_msg_num(0), _crc_error_num(0),_crc_correct_num(0), _acars_recv(NULL)
{
#if STATISTICS_FILE
	time_t curr_time = time(NULL);
	std::string str_time = TH_UTILITY::time2String(curr_time);

	std::string file_name = str_time + "_statistics.csv";
	_statistics_file.open(file_name);

	_statistics_file << "时间,总报文个数,crc错误个数,crc纠正个数\n";

#endif

	init_mesg();
}

acars_decoder::~acars_decoder()
{
#if STATISTICS_FILE
	_statistics_file.close();
#endif
}

void acars_decoder::set_callback(acars_recv_callback acars_rc)
{
	_acars_recv = acars_rc;
}

//acars消息初始化
void acars_decoder::init_mesg()
{
	_curr_msg_state.state = HEADL;
}

void acars_decoder::update_crc(unsigned short *crc, unsigned char ch)
{
	unsigned char v;
	unsigned int i;
	unsigned short flag;

	v = 1;
	for (i = 0; i < 8; i++) {
		flag = (*crc & 0x8000);
		*crc = *crc << 1;

		if (ch & v)
			*crc = *crc + 1;

		if (flag != 0)
			*crc = *crc ^ POLY;

		v = v << 1;
	}
}

void acars_decoder::set_recv_time(const std::string & time_str)
{
	_recv_time = time_str;
}


int acars_decoder::getmesg(unsigned char r)
{
	acars_mstat_s *st;

	st = &_curr_msg_state;

	switch (st->state) 
	{
	case HEADL:
		if (r == 0xff)
		{
			st->state = HEADF;
			return 8;
		}
		break;
	case HEADF:
		if (r != 0xff)
		{
			int i;
			unsigned char m;

			for (i = 0, m = 1; i < 7; i++, m = m << 1)
			{
				if (!(r & m))
					break;
			}
			if (i < 2)
			{
				st->state = HEADL;
				break;
			}
			st->state = BSYNC1;
			st->ind = 0;
			if (i != 2)
				return (i - 2);
			break;
		}
		return 6;
	case BSYNC1:
		if (r != 0x80 + '+')
			st->ind++;
		st->state = BSYNC2;
		return 8;
	case BSYNC2:
		if (r != '*')
			st->ind++;
		st->state = SYN1;
		return 8;
	case SYN1:
		if (r != SYN)
			st->ind++;
		st->state = SYN2;
		return 8;
	case SYN2:
		if (r != SYN)
			st->ind++;
		st->state = SOH1;
		return 8;
	case SOH1:
		if (r != SOH)
			st->ind++;
		if (st->ind > 2)
		{
			st->state = HEADL;
			break;
		}
		st->state = TXT;
		st->ind = 0;
		st->crc = 0;
		return 8;
	case TXT:
		update_crc(&st->crc, r);

		_source_txt[st->ind] = r;

		r = r & 0x7f;
		if (r == 0x03 || r == 0x17)
		{
			st->state = CRC1;
			return 8;
		}
		st->txt[st->ind] = r;
		st->ind++;
		if (st->ind > 243) {
			st->state = HEADL;
			break;
		}
		return 8;
	case CRC1:
		update_crc(&st->crc, r);
		_source_crc1 = r;
		st->state = CRC2;
		return 8;
	case CRC2:
		update_crc(&st->crc, r);
		_source_crc2 = r;
		st->state = END;
		return 8;
	case END:
		st->state = HEADL;

		++_all_msg_num;

		int ret_val = 8;

		if (st->crc == 0) 
		{
			build_mesg(st->txt, st->ind);
			ret_val = 0;
		}
		else
		{
			fprintf(stdout, "crc error\r\n");

			++_crc_error_num;

			//纠错 只纠一位 在此加1是因为加上0x83终止符，终止符参与crc计算
			correct_by_onebyte(_source_txt, st->ind + 1);

			ret_val = 8;
		}

#if STATISTICS_FILE
		time_t curr_time = time(NULL);
		std::string str_time = TH_UTILITY::time2String(curr_time);

		_statistics_file << str_time << "," << _all_msg_num << "," << _crc_error_num << "," << _crc_correct_num;
		_statistics_file << "\n";
#endif

		return ret_val;
	}

	return 8;
}

int acars_decoder::build_mesg(char *txt, int len)
{
	int i, k;
	unsigned char r;

	/*去掉特殊字符*/
	for (i = 0; i < len; i++) 
	{
		r = txt[i];
		if (r < ' ' && r != 0x0d && r != 0x0a && r!= 0x15)
			r = 0xa4;
		txt[i] = r;
	}
	txt[i] = '\0';

	/*填充MSG结构体*/
	acars_msg_t curr_msg;
	acars_msg_t* msg = &curr_msg;

	k = 0;
	msg->mode = txt[k];
	k++;

	for (i = 0; i < 7; i++, k++)
	{
		msg->addr[i] = txt[k];
	}
	msg->addr[7] = '\0';

	/* ACK/NAK */
	msg->ack = txt[k];
	k++;

	msg->label[0] = txt[k];
	k++;
	msg->label[1] = txt[k];
	k++;
	msg->label[2] = '\0';

	msg->bid = txt[k];
	k++;

	k++;

	for (i = 0; i < 4; i++, k++) 
	{
		msg->no[i] = txt[k];
	}
	msg->no[4] = '\0';

	for (i = 0; i < 6; i++, k++) 
	{
		msg->fid[i] = txt[k];
	}
	msg->fid[6] = '\0';

	strcpy(msg->txt, &(txt[k]));

	curr_msg.recv_time_str = _recv_time;

	if (_acars_recv)
	{
		_acars_recv(curr_msg);
	}

	return 1;
}

void acars_decoder::correct_by_onebyte(char *txt, int len)
{
	int error_count = 0;
	int error_bit = 0;

	for (int i = 0; i < len; ++i)
	{
		//获取校验码 一个字节的最高位
		bool check_bit = (BIT_CHECK(txt[i], 7) == 0 ? false : true);

		int bit1_count = TH_UTILITY::Bit_Count(txt[i] & 0x7f);

		//为偶数时,校验位为1;为奇数时,校验位为0
		if (!((bit1_count % 2 == 0 ? false : true) ^ check_bit))
		{
			++error_count;
			error_bit = i;              //如果只错一个字节 记录错的这个字节的位置
		}
	}

	//只纠一个字节中的一位错误
	if (error_count == 1)
	{
		char temp = txt[error_bit];

		for (int j = 0; j < 8; ++j)
		{
			//翻转
			BIT_FLIP(txt[error_bit], j);

			unsigned short crc = 0;

			for (int k = 0; k < len; ++k)
			{
				update_crc(&crc, (unsigned char)txt[k]);
			}

			update_crc(&crc, _source_crc1);
			update_crc(&crc, _source_crc2);

			if (crc == 0)
			{
				//在此减1是因为去掉0x83终止符
				build_mesg(txt, len - 1);
				++_crc_correct_num;
				break;
			}

			txt[error_bit] = temp;
		}
	}
}

void acars_set_decode_callback(acars_recv_callback cb)
{
	sln_acars_decoder::instance().set_callback(cb);
}

void acars_decode(const char* data_ptr, unsigned int data_size)
{
	for (unsigned int i = 0; i < data_size; ++i)
	{
		sln_acars_decoder::instance().getmesg(data_ptr[i]);
	}
}
