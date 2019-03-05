#include "stdafx.h"
#include "tea.h"
#include "base64ex.h"
using namespace std;
using namespace Bear::Core;

//TEA加密算法有三个magic数,为避免在程序代码中直接出现这三个数，在运行时采用两个数之和拼凑而成
#define TEA_MAGIC_NUM0			0x9E3779B9		//=TEA_MAGIC_NUM0_PART1+TEA_MAGIC_NUM0_PART2
#define TEA_MAGIC_NUM0_PART1	0x12345678
#define TEA_MAGIC_NUM0_PART2	0x8C032341

#define TEA_MAGIC_NUM1			0xC6EF3720		//=TEA_MAGIC_NUM1_PART1+TEA_MAGIC_NUM1_PART2
#define TEA_MAGIC_NUM1_PART1	0x56789abc
#define TEA_MAGIC_NUM1_PART2	0x70769C64

#define TEA_MAGIC_NUM2			0xE3779B90		//=TEA_MAGIC_NUM2_PART1+TEA_MAGIC_NUM2_PART2
#define TEA_MAGIC_NUM2_PART1	0xabcdabcd
#define TEA_MAGIC_NUM2_PART2	0x37A9EFC3

Tea::Tea(int round)
{
	memset(m_key, 0, sizeof(m_key));
	m_round=round;

	// ULONG xx=0x9E3779B9;
}

Tea::~Tea(void)
{
}

//只取pszPassword前16个字符
void Tea::SetPassword(const char *pszPassword)
{
	memset(m_key, 0, sizeof(m_key));
	strncpy((char*)m_key,pszPassword,sizeof(m_key)-1);
}

//每次加密8字节
void Tea::Encrypt(const ULONG *in, ULONG *out) 
{
	ULONG *k = (ULONG*)m_key;
	ULONG y = (in[0]);
	ULONG z = (in[1]);
	ULONG a = (k[0]);
	ULONG b = (k[1]);
	ULONG c = (k[2]);
	ULONG d = (k[3]);

	ULONG NUM0_PART1 = TEA_MAGIC_NUM0_PART1;
	ULONG NUM0_PART2 = TEA_MAGIC_NUM0_PART2;

	ULONG delta = NUM0_PART1 + NUM0_PART2;
	//ASSERT(delta == TEA_MAGIC_NUM0);
	int round = m_round;
	ULONG sum = 0;

	while (round--) 
	{
		sum += delta;
		y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
	}
	out[0] = (y);
	out[1] = (z);
}

//每次解密8字节
void Tea::Decrypt(const ULONG *in, ULONG *out)
{
	ULONG *k = (ULONG*)m_key;
	ULONG y = (in[0]);
	ULONG z = (in[1]);
	ULONG a = (k[0]);
	ULONG b = (k[1]);
	ULONG c = (k[2]);
	ULONG d = (k[3]);
	 ULONG NUM0_PART1 = TEA_MAGIC_NUM0_PART1;
	 ULONG NUM0_PART2 = TEA_MAGIC_NUM0_PART2;
	ULONG delta = NUM0_PART1 + NUM0_PART2;
	int round = m_round;
	ULONG sum = 0;

	if (round == 32)
	{
		 ULONG NUM1_PART1 = TEA_MAGIC_NUM1_PART1;
		 ULONG NUM1_PART2 = TEA_MAGIC_NUM1_PART2;
		sum = NUM1_PART1 + NUM1_PART2;
	}
	else if (round == 16)
	{
		 ULONG NUM2_PART1 = TEA_MAGIC_NUM2_PART1;
		 ULONG NUM2_PART2 = TEA_MAGIC_NUM2_PART2;
	
		sum = NUM2_PART1 + NUM2_PART2;
	}
	else
		sum = delta * round;

	while (round--) 
	{
		z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
		y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
		sum -= delta;
	}
	out[0] = (y);
	out[1] = (z);
}

//返回加密结果字节数
////cbEnc至少比cbData大8,用来存放padding
int Tea::Encode(const LPBYTE pData,int cbData,LPBYTE pEnc,int cbEnc)
{
	if(cbData<=0)
	{
		return 0;
	}

	//const int max_padding=8;	//最多填充8字节
	if(cbEnc < GetEncodeLength(cbData))
	{
		ASSERT(FALSE);
		return 0;
	}

	LPBYTE psrc=pData;
	LPBYTE pdst=pEnc;
	int left=cbData;
	int cbWrite=0;

	const int eat=8;			//每步消耗字节数
	while(left>=eat)
	{
		Encrypt((ULONG*)psrc,(ULONG*)pdst);
		psrc+=eat;
		pdst+=eat;
		cbWrite+=eat;

		left-=eat;
	}

	int padding_len=eat-left;//要填充的字节数
	BYTE padding[8];
	if(left>0)
	{
		memcpy(padding,psrc,left);
	}
	padding[7]=padding_len;//记住padding_len,解密时会根据此数据删除padding
	Encrypt((ULONG*)padding,(ULONG*)pdst);
	cbWrite+=eat;

	return cbWrite;
}

int Tea::GetEncodeLength(int cbData)
{
	if(cbData<=0)
	{
		return 0;
	}

	int cbWrite = ((cbData+8)/8)*8;
	return cbWrite;
}

//返回解密结果字节数
//cbData必须>=cbEnc
int Tea::Decode(const LPBYTE pEnc,int cbEnc,LPBYTE pData,int cbData)
{
	const int eat=8;			//每步消耗字节数

	if(cbData<cbEnc || cbEnc<=0 || ((cbEnc%eat)!=0))
	{
		//ASSERT(FALSE);
		return 0;
	}

	//const int max_padding=8;	//最多填充8字节

	LPBYTE psrc=pEnc;
	LPBYTE pdst=pData;
	int left=cbEnc;
	int cbWrite=0;

	while(left>=eat)
	{
		Decrypt((ULONG*)psrc,(ULONG*)pdst);
		psrc+=eat;
		pdst+=eat;
		cbWrite+=eat;

		left-=eat;
	}
	ASSERT(left==0);

	cbWrite-=*(pdst-1);//删除padding字节数
	return cbWrite;
}

int Tea::Encode(ByteBuffer& inbox,ByteBuffer& outbox)
{
	outbox.clear();

	LPBYTE pData=inbox.GetDataPointer();
	int cbData=inbox.GetActualDataLength();

	const int cbLen=GetEncodeLength(cbData);
	outbox.PrepareBuf(cbLen);
	int ret=Encode(pData,cbData,outbox.GetNewDataPointer(),cbLen);
	outbox.WriteDirect(cbLen);
	return 0;
}

int Tea::Decode(ByteBuffer& inbox,ByteBuffer& outbox)
{
	outbox.clear();

	LPBYTE pData=inbox.GetDataPointer();
	int cbData=inbox.GetActualDataLength();

	outbox.PrepareBuf(cbData);
	int ret=Decode(pData,cbData,outbox.GetNewDataPointer(),cbData);
	outbox.WriteDirect(ret);
	return 0;
}

string  Tea::EncodeTextWithBase64(string  plainText)
{
	ByteBuffer inbox,outbox;
	inbox.Write(plainText);
	Encode(inbox, outbox);

	return Base64::Encode(outbox.GetDataPointer(), outbox.GetActualDataLength());
}

string  Tea::DecodeTextWithBase64(string  cryptText)
{
	ByteBuffer box,outbox;
	Base64::Decode(cryptText, box);
	Decode(box, outbox);
	outbox.MakeSureEndWithNull();
	return (const char*)outbox.GetDataPointer();
}

#ifdef _DEBUG
int Tea::Test()
{
	char plain[800];//="012345678";//This is a demo for TEA!";

	srand((int)ShellTool::GetTickCount64());

	for(int times=0;times<10;times++)
	{
		{
			string  sz;
			//int len= rand()%512;
			for(int i=0;i<512;i++)
			{
				string  tmp=Core::StringTool::Format("%c",(rand()%128)+1);
				sz+=tmp;
			}

			memset(plain,0,sizeof(plain));
			strncpy(plain,sz.c_str(),sizeof(plain)-1);
		}

		char crypt_buf[1000];
		char decrypt_buf[1000];

		memset(crypt_buf,0,sizeof(crypt_buf));
		memset(decrypt_buf,0,sizeof(decrypt_buf));

		plain[sizeof(plain)-1]=0;
		int len=(int)strlen(plain);
		int ret = Encode((LPBYTE)plain,len,(LPBYTE)crypt_buf,sizeof(crypt_buf));
		ASSERT(ret == GetEncodeLength(len));
		ret = Decode((LPBYTE)crypt_buf,ret,(LPBYTE)decrypt_buf,sizeof(decrypt_buf));
		if(ret>0)
		{
			decrypt_buf[ret]=0;
			DT("[%s]",decrypt_buf);
			if(strcmp(decrypt_buf,plain))
			{
				DW("mismatch");
				ASSERT(FALSE);
			}
		}
		else
		{
			ASSERT(FALSE);
		}
	}

	return 0;
}
#endif
