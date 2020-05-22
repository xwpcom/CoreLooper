#include "stdafx.h"
#include "crypthelper.h"
#include "libcrypt/tea.h"
#include "base64ex.h"
using namespace std;
using namespace Bear::Core;

#define _USER_DATA_CRYPT_KEY	"MayGionIPCameraMaster"

string  CryptHelper::Encode(string  plainText)
{
	//DT("CryptHelper::Encode(%s)#begin",plainText.c_str());
	string  userData;

	if(!plainText.empty())
	{
		ByteBuffer inbox;
		ByteBuffer outbox;

		inbox.Write(plainText);
		
		Tea tea;
		tea.SetPassword(_USER_DATA_CRYPT_KEY);
		tea.Encode(inbox,outbox);

		char* userDataBase64 = Base64::base64Encode((char*)outbox.GetDataPointer(), outbox.GetActualDataLength());
		userData = userDataBase64;
		delete[]userDataBase64;
		userDataBase64 = NULL;
	}

	//DT("CryptHelper::Encode(%s)=(%s)",plainText.c_str(),userData.c_str());
	return userData;
}

string  CryptHelper::Decode(string  encodedText)
{
	//DT("CryptHelper::Decode(%s)#begin", encodedText.c_str());

	string  userDataPlain;

	string  userData = encodedText;
	//DT("userData=(%s),userData.IsEmpty()=%d,len=%d", userData.c_str(), userData.IsEmpty(), userData.GetLength());
	if (!userData.empty())
	{
		int outputSize = (int)userData.length() * 2;
		if (outputSize<64)
		{
			outputSize = 64;
		}

		LPBYTE output = new BYTE[outputSize];
		memset(output, 0, outputSize);
		bool ignore_errors = true;
		int ret = Base64::base64_decode_EX(userData.c_str(), (int)userData.length(), (char*)output, outputSize - 1, ignore_errors);
		if (ret>0)
		{
			ByteBuffer inbox;
			ByteBuffer outbox;

			inbox.Write(output, ret);
			Tea tea;
			tea.SetPassword(_USER_DATA_CRYPT_KEY);
			tea.Decode(inbox, outbox);
			outbox.MakeSureEndWithNull();

			userDataPlain = (const char *)outbox.GetDataPointer();
		}

		delete[]output;
		output = NULL;

	}

	//DT("CryptHelper::Decode(%s)=(%s)", encodedText.c_str(), userDataPlain.c_str());
	return userDataPlain;
}
