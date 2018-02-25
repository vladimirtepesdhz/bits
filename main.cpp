#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

typedef	unsigned char	u8;
typedef	unsigned short	u16;
typedef	unsigned int	u32;

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

int	main(int argc,char * argv[])
{
	u8	test_bits_be[] = {0x12,0x34,0x56,0x78,0x9A,0xBC,0xDE,0xF0};
	u8	test_bits_le[] = {0xF0,0xDE,0xBC,0x9A,0x78,0x56,0x34,0x12};
	u8	test_bits_buf[8];
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
	srand(time(NULL));
	for(int iter=0;iter<100;++iter)
	{
		u32 value = 0;
		u32 wv = 0;
		u32 rv = 0;
		u32	mask = 0;
		int offset = 0;
		value = rand();
		mask = GenMask(rand()%33);
		offset = rand()%(sizeof(test_bits_buf)*8);
		wv = value & mask;
		SetBits_LE32(test_bits_buf,sizeof(test_bits_buf),offset,mask,wv);
		rv = GetBits_LE32(test_bits_buf,sizeof(test_bits_be),offset,mask);
		printf("value==0x%8.8x\toffset==%d\tmask==0x%8.8x\twv==0x%8.8x\trv==0x%8.8x\n",value,offset,mask,wv,rv);
	}
	return	0;
}
