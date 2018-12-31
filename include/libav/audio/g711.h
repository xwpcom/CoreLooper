#pragma once
namespace Bear {
namespace Core {
namespace Media {

//g.711 ulaw
class AV_EXPORT G711
{
public:
	static int Encode(const LPBYTE pPcm, int cbPcm, LPBYTE pAdpcm, int cbAdpcm);
	static int Decode(const LPBYTE pAdpcm, int cbAdpcm, LPBYTE pPcm, int cbPcm);
protected:
	static unsigned char uLawFrom16BitLinear(short sample);
	static short linear16FromuLaw(unsigned char uLawByte);
};
}
}
}