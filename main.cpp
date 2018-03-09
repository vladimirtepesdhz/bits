#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

typedef	unsigned char	u8;
typedef	unsigned short	u16;
typedef	unsigned int	u32;
typedef	enum _bool_t
{
	t_false
	,t_true
}bool_t;

//#define	GEN_MASK(n)	((((u32)1)<<(n))-1)
u32	GenMask(int n)
{
	if(n <= 0)
		return	0;
	else if(n >= 32)
		return	0xFFFFFFFF;
	else
		return	((((u32)1)<<n)-1);
}

u32	GetBits_LE32(unsigned char const * pdata,int data_size,int offset,u32 bits_mask)
{
	u32 mask32 = 0xFFFFFFFF;
	int byte_offset = 0;
	int bit_rem = 0;
	int iter = 0;
	u32 result_data = 0;
	
	byte_offset = (offset >> 3);
	bit_rem = (offset & 0x07);
	if(byte_offset >= data_size)
		return	0;
	for(iter=0;(iter<4)&&(byte_offset + iter < data_size);++iter)
	{
		result_data += (((u32)pdata[byte_offset+iter])<<(iter*8));
	}
	if(bit_rem)
	{
		result_data &= (mask32 << bit_rem);
		result_data >>= bit_rem;

		if(byte_offset + 4 < data_size)
		{
			u32 i_mask = 0;
			u32 temp = 0;
			i_mask = GenMask(bit_rem);
			temp = (((u32)pdata[byte_offset+4])&i_mask)<<(32-bit_rem);
			result_data += temp;
		}
	}
	return	(result_data & bits_mask);
}

u32	GetBits_BE32(unsigned char const * pdata,int data_size,int offset,u32 bits_mask)
{
	u32 mask32 = 0xFFFFFFFF;
	int byte_offset = 0;
	int bit_rem = 0;
	int iter = 0;
	u32 result_data = 0;
	int invert_offset = data_size - 1;

	byte_offset = (offset >> 3);
	bit_rem = (offset & 0x07);
	if(byte_offset >= data_size)
		return	0;
	for(iter=0;(iter<4)&&(byte_offset + iter < data_size);++iter)
	{
		result_data += (((u32)pdata[invert_offset - (byte_offset+iter)])<<(iter*8));
	}
	if(bit_rem)
	{
		result_data &= (mask32 << bit_rem);
		result_data >>= bit_rem;

		if(byte_offset + 4 < data_size)
		{
			u32 i_mask = 0;
			u32 temp = 0;
			i_mask = GenMask(bit_rem);
			temp = (((u32)pdata[invert_offset-(byte_offset+4)])&i_mask)<<(32-bit_rem);
			result_data += temp;
		}
	}
	return	(result_data & bits_mask);
}

void	SetBits_LE32(u8 * pdata,int data_size,int offset,u32 bits_mask,u32 value)
{
	int byte_offset = 0;
	int bit_rem = 0;
	u32 ovf = 0;
	u32 ovm = 0;
	int iter = 0;

	byte_offset = (offset >> 3);
	bit_rem = (offset & 0x07);
	value &= bits_mask;
	if(bit_rem)
	{
		u32 i_mask = (GenMask(bit_rem)<<(32 - bit_rem));
		
		ovf = (value & i_mask)>>(32 - bit_rem);
		ovm = (bits_mask & i_mask)>>(32 - bit_rem);
		value <<= bit_rem;
		bits_mask <<= bit_rem;
	}
	for(iter=0;(iter<4)&&(byte_offset + iter < data_size);++iter)
	{
		u8 tv = 0;
		u8 tm = 0;
		u8 * p_dst =0;
		tv = value & 0xFF;
		tm = bits_mask & 0xFF;
		if(tm)
		{
			p_dst = &pdata[byte_offset + iter];
			*p_dst &= ~tm;
			*p_dst |= tv;
		}
		value >>= 8;
		bits_mask >>= 8;
	}
	if(ovm && byte_offset + 4 < data_size)
	{
		u8 * p_dst = &pdata[byte_offset + iter];
		*p_dst &= ~ovm;
		*p_dst |= ovf;
	}
}


void	SetBits_BE32(u8 * pdata,int data_size,int offset,u32 bits_mask,u32 value)
{
	int byte_offset = 0;
	int bit_rem = 0;
	u32 ovf = 0;
	u32 ovm = 0;
	int iter = 0;
	int invert_offset = data_size - 1;

	byte_offset = (offset >> 3);
	bit_rem = (offset & 0x07);
	value &= bits_mask;
	if(bit_rem)
	{
		u32 i_mask = (GenMask(bit_rem)<<(32 - bit_rem));
		
		ovf = (value & i_mask)>>(32 - bit_rem);
		ovm = (bits_mask & i_mask)>>(32 - bit_rem);
		value <<= bit_rem;
		bits_mask <<= bit_rem;
	}
	for(iter=0;(iter<4)&&(byte_offset + iter < data_size);++iter)
	{
		u8 tv = 0;
		u8 tm = 0;
		u8 * p_dst =0;
		tv = value & 0xFF;
		tm = bits_mask & 0xFF;
		if(tm)
		{
			p_dst = &pdata[invert_offset - (byte_offset + iter)];
			*p_dst &= ~tm;
			*p_dst |= tv;
		}
		value >>= 8;
		bits_mask >>= 8;
	}
	if(ovm && byte_offset + 4 < data_size)
	{
		u8 * p_dst = &pdata[invert_offset - (byte_offset + 4)];
		*p_dst &= ~ovm;
		*p_dst |= ovf;
	}
}

void	ShiftLeft_LE(u8 * pdata,int data_size,int shift,bool_t rotate)
{
	int byte_cnt = 0;
	int bit_rem = 0;
	int iter = 0;
	u8 * p_buf = 0;
	u8 mask = 0;
	u8 ovf = 0;
	u8 so = 0;

	if(!pdata)
		return;
	if(data_size <= 0)
		return;
	if(shift <= 0)
		return;
	byte_cnt = shift / 8;
	if(t_false == rotate && byte_cnt >= data_size)
	{
		memset(pdata,0x00,data_size);
		return;
	}
	byte_cnt %= data_size;
	bit_rem = shift & 0x07;

	if(byte_cnt)	//字节位移
	{
		if(rotate)
		{
			p_buf = (u8 *)malloc(byte_cnt);
			memcpy(p_buf,&pdata[data_size - byte_cnt],byte_cnt);
		}
		for(iter=data_size - 1; iter >= byte_cnt; --iter)
		{
			pdata[iter] = pdata[iter - byte_cnt];
		}
		if(rotate)
		{
			memcpy(&pdata[0],p_buf,byte_cnt);
			free(p_buf);
		}
		else
		{
			memset(&pdata[0],0x00,byte_cnt);
		}
	}
	if(bit_rem)	//比特位移
	{
		mask = (GenMask(bit_rem) << (8-bit_rem));
		if(rotate)
		{
			so = (pdata[data_size - 1] & mask) >> (8-bit_rem);
		}
		for(iter=data_size-1;iter>0;--iter)
		{
			pdata[iter] <<= bit_rem;
			ovf = ((pdata[iter-1] & mask) >> (8-bit_rem));
			pdata[iter] |= ovf;
		}
		pdata[0] <<= bit_rem;
		if(rotate)
		{
			pdata[0] |= so;
		}
	}
}

void	ShiftRight_LE(u8 *pdata,int data_size,int shift,bool_t rotate)
{
	int byte_cnt = 0;
	int bit_rem = 0;
	int iter = 0;
	u8 * p_buf = 0;
	u8 mask = 0;
	u8 ovf = 0;
	u8 so = 0;

	if(!pdata)
		return;
	if(data_size <= 0)
		return;
	if(shift <= 0)
		return;
	byte_cnt = shift / 8;
	if(t_false == rotate && byte_cnt >= data_size)
	{
		memset(pdata,0x00,data_size);
		return;
	}
	byte_cnt %= data_size;
	bit_rem = shift & 0x07;

	if(byte_cnt)	//字节位移
	{
		if(rotate)
		{
			p_buf = (u8 *)malloc(byte_cnt);
			memcpy(p_buf,&pdata[0],byte_cnt);
		}
		for(iter=0;iter< data_size - byte_cnt;++iter)
		{
			pdata[iter] = pdata[iter + byte_cnt];
		}
		if(rotate)
		{
			memcpy(&pdata[data_size - byte_cnt],p_buf,byte_cnt);
			free(p_buf);
		}
		else
		{
			memset(&pdata[data_size - byte_cnt],0x00,byte_cnt);
		}
	}
	if(bit_rem)	//比特位移
	{
		mask = GenMask(bit_rem);
		if(rotate)
		{
			so = (pdata[0] & mask) << (8 - bit_rem);
		}
		for(iter=0;iter<data_size-1;++iter)
		{
			pdata[iter] >>= bit_rem;
			ovf = ((pdata[iter+1] & mask) << (8-bit_rem));
			pdata[iter] |= ovf;
		}
		pdata[data_size - 1] >>= bit_rem;
		if(rotate)
		{
			pdata[data_size - 1] |= so;
		}
	}
}

void	ShiftLeft_BE(u8 * pdata,int data_size,int shift,bool_t rotate)
{
	int byte_cnt = 0;
	int bit_rem = 0;
	int iter = 0;
	u8 * p_buf = 0;
	u8 mask = 0;
	u8 ovf = 0;
	u8 so = 0;

	if(!pdata)
		return;
	if(data_size <= 0)
		return;
	if(shift <= 0)
		return;
	byte_cnt = shift / 8;
	if(t_false == rotate && byte_cnt >= data_size)
	{
		memset(pdata,0x00,data_size);
		return;
	}
	byte_cnt %= data_size;
	bit_rem = shift & 0x07;

	if(byte_cnt)	//字节位移
	{
		if(rotate)
		{
			p_buf = (u8 *)malloc(byte_cnt);
			memcpy(p_buf,&pdata[0],byte_cnt);
		}
		for(iter=0;iter<data_size-byte_cnt;++iter)
		{
			pdata[iter] = pdata[iter + byte_cnt];
		}
		if(rotate)
		{
			memcpy(&pdata[data_size - byte_cnt],p_buf,byte_cnt);
			free(p_buf);
		}
		else
		{
			memset(&pdata[data_size - byte_cnt],0x00,byte_cnt);
		}
	}
	if(bit_rem)	//比特位移
	{
		mask = (GenMask(bit_rem) << (8-bit_rem));
		if(rotate)
		{
			so = (pdata[0] & mask) >> (8-bit_rem);
		}
		for(iter=0;iter<data_size-1;++iter)
		{
			pdata[iter] <<= bit_rem;
			ovf = ((pdata[iter+1] & mask) >> (8-bit_rem));
			pdata[iter] |= ovf;
		}
		pdata[data_size-1] <<= bit_rem;
		if(rotate)
		{
			pdata[data_size-1] |= so;
		}
	}
}

void	ShiftRight_BE(u8 * pdata,int data_size,int shift,bool_t rotate)
{
	int byte_cnt = 0;
	int bit_rem = 0;
	int iter = 0;
	u8 * p_buf = 0;
	u8 mask = 0;
	u8 ovf = 0;
	u8 so = 0;

	if(!pdata)
		return;
	if(data_size <= 0)
		return;
	if(shift <= 0)
		return;

	byte_cnt = shift / 8;
	if(t_false == rotate && byte_cnt >= data_size)
	{
		memset(pdata,0x00,data_size);
		return;
	}

	byte_cnt %= data_size;
	bit_rem = shift & 0x07;

	if(byte_cnt)	//字节位移
	{
		if(rotate)
		{
			p_buf = (u8 *)malloc(byte_cnt);
			memcpy(p_buf,&pdata[data_size-byte_cnt],byte_cnt);
		}
		for(iter=data_size-1;iter>=byte_cnt;--iter)
		{
			pdata[iter] = pdata[iter - byte_cnt];
		}
		if(rotate)
		{
			memcpy(&pdata[0],p_buf,byte_cnt);
			free(p_buf);
		}
		else
		{
			memset(&pdata[0],0x00,byte_cnt);
		}
	}
	if(bit_rem)	//比特位移
	{
		mask = GenMask(bit_rem);
		if(rotate)
		{
			so = (pdata[data_size-1] & mask) << (8-bit_rem);
		}
		for(iter=data_size-1;iter>0;--iter)
		{
			pdata[iter] >>= bit_rem;
			ovf = ((pdata[iter-1] & mask) << (8-bit_rem));
			pdata[iter] |= ovf;
		}
		pdata[0] >>= bit_rem;
		if(rotate)
		{
			pdata[0] |= so;
		}
	}
}

void OutputBinaryByte(u8 v)
{
	u8 mask = 0x80;
	while(mask)
	{
		if(mask & v)
			printf("1");
		else
			printf("0");
		mask >>= 1;
	}
}

void OutputBinaryData(u8 * pdata,int data_size,bool_t le)
{
	int iter = 0;
	if(le)
	{
		for(iter=data_size-1;iter>=0;--iter)
		{
			OutputBinaryByte(pdata[iter]);
			printf(" ");
		}
	}
	else
	{
		for(iter=0;iter<data_size;++iter)
		{
			OutputBinaryByte(pdata[iter]);
			printf(" ");
		}
	}
}

u8	test_bits_be[] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
u8	test_bits_le[] = {0xF0,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12};
u8	test_bits_buf[8];
int	main(int argc,char * argv[])
{
#if 0
	for(int iter=0;iter<sizeof(test_bits_le)*8;++iter)
	{
		printf("LE 0x%8.8x\t",GetBits_LE32(test_bits_le,sizeof(test_bits_le),iter,0xFFFF));
		printf("BE 0x%8.8x\n",GetBits_BE32(test_bits_be,sizeof(test_bits_be),iter,0xFFFF));
	}
	for(int offset=0;offset<sizeof(test_bits_buf)*8;++offset)
	{
		memset(test_bits_buf,0,sizeof(test_bits_buf));
		value = GetBits_LE32(test_bits_le,sizeof(test_bits_le),offset,0xFFFF); 
		SetBits_LE32(test_bits_buf,sizeof(test_bits_buf),offset,0xFFFF,value);
		printf("test_bits_buf[]={");
		for(int iter=0;iter<sizeof(test_bits_buf);++iter)
		{
			printf("0x%2.2x,",test_bits_buf[iter]);
		}
		printf("}\n");
	}
#endif

#if 0
	srand(time(NULL));
	for(int iter=0;iter<100;++iter)
	{
		u32 value = 0;
		u32 wv = 0;
		u32 rv = 0;
		u32	mask = 0;
		int offset = 0;
		int cnt = 0;

		value = rand();
		offset = rand()%(sizeof(test_bits_buf)*8);
		cnt = sizeof(test_bits_buf)*8 - offset;
		if(cnt > 33)
			cnt = 33;
		mask = GenMask(rand()%cnt);
		wv = value & mask;
		SetBits_LE32(test_bits_le,sizeof(test_bits_le),offset,mask,wv);
		rv = GetBits_LE32(test_bits_le,sizeof(test_bits_le),offset,mask);
		printf("le: value==0x%8.8x\toffset==%d\tmask==0x%8.8x\twv==0x%8.8x\trv==0x%8.8x\n",value,offset,mask,wv,rv);
		if(rv != wv)
			printf("ERROR!\n");
		SetBits_BE32(test_bits_be,sizeof(test_bits_be),offset,mask,wv);
		rv = GetBits_BE32(test_bits_be,sizeof(test_bits_be),offset,mask);
		printf("be: value==0x%8.8x\toffset==%d\tmask==0x%8.8x\twv==0x%8.8x\trv==0x%8.8x\n",value,offset,mask,wv,rv);
		if(rv != wv)
			printf("ERROR!\n");
	}
#endif

	for(int rotate = (int)t_false;rotate <= (int)t_true ; ++rotate)
	{
		for(int offset=0;offset<= sizeof(test_bits_buf)*8;++offset)
		{
			memcpy(test_bits_buf,test_bits_le,sizeof(test_bits_buf));
			ShiftLeft_LE(test_bits_buf,sizeof(test_bits_buf),offset,(bool_t)rotate);
			printf("le %s left %d:\ttest_bits_buf[]={",rotate?"rotate":"shift",offset);
			//for(int iter=0;iter<sizeof(test_bits_buf);++iter)
			for(int iter=sizeof(test_bits_buf)-1;iter>=0;--iter)
			{
				printf("0x%2.2x,",test_bits_buf[iter]);
			}
			//printf("}\n");
			printf("\t");
			OutputBinaryData(test_bits_buf,sizeof(test_bits_buf),t_true);
			printf("\n");
		}
		printf("\n");
		for(int offset=0;offset<= sizeof(test_bits_buf)*8;++offset)
		{
			memcpy(test_bits_buf,test_bits_le,sizeof(test_bits_buf));
			ShiftRight_LE(test_bits_buf,sizeof(test_bits_buf),offset,(bool_t)rotate);
			printf("le %s right %d:\ttest_bits_buf[]={",rotate?"rotate":"shift",offset);
			//for(int iter=0;iter<sizeof(test_bits_buf);++iter)
			for(int iter=sizeof(test_bits_buf)-1;iter>=0;--iter)
			{
				printf("0x%2.2x,",test_bits_buf[iter]);
			}
			//printf("}\n");
			printf("\t");
			OutputBinaryData(test_bits_buf,sizeof(test_bits_buf),t_true);
			printf("\n");
		}
		printf("\n");
	
		for(int offset=0;offset<= sizeof(test_bits_buf)*8;++offset)
		{
			memcpy(test_bits_buf,test_bits_be,sizeof(test_bits_buf));
			ShiftLeft_BE(test_bits_buf,sizeof(test_bits_buf),offset,(bool_t)rotate);
			printf("be %s left %d:\ttest_bits_buf[]={",rotate?"rotate":"shift",offset);
			for(int iter=0;iter<sizeof(test_bits_buf);++iter)
			{
				printf("0x%2.2x,",test_bits_buf[iter]);
			}
			//printf("}\n");
			printf("\t");
			OutputBinaryData(test_bits_buf,sizeof(test_bits_buf),t_false);
			printf("\n");
		}
		printf("\n");
		for(int offset=0;offset<= sizeof(test_bits_buf)*8;++offset)
		{
			memcpy(test_bits_buf,test_bits_be,sizeof(test_bits_buf));
			ShiftRight_BE(test_bits_buf,sizeof(test_bits_buf),offset,(bool_t)rotate);
			printf("be %s right %d:\ttest_bits_buf[]={",rotate?"rotate":"shift",offset);
			for(int iter=0;iter<sizeof(test_bits_buf);++iter)
			{
				printf("0x%2.2x,",test_bits_buf[iter]);
			}
			//printf("}\n");
			printf("\t");
			OutputBinaryData(test_bits_buf,sizeof(test_bits_buf),t_false);
			printf("\n");
		}
		printf("\n");
	}
	return	0;
}
