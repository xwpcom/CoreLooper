#include "protocol/sctp/csctp.h"

//sctp内部出错时会调用本接口
void SCTP_SetError(tagSCTP* obj, const char * desc)
{
}

//sctp收到命令时会调用本接口
void SCTP_OnRecvCommandCB(tagSCTP *obj, const char *cmd, tagBundle *bundle)
{
}

void SCTP_OnCrcError(const char *body, int bytes)
{

}
