#include "protocol.h"


PDU *mkPDU(uint uiMsgLen)
{
    uint uiPDULen = sizeof(PDU)+uiMsgLen; //总长度
    PDU *pdu =(PDU*)malloc(uiPDULen);
    if(NULL == pdu)
    {
        exit(EXIT_FAILURE);
    }
    memset(pdu,0,uiPDULen);//设为0
    pdu->uiPDULen = uiPDULen;//总长度
    pdu->uiMsgLen = uiMsgLen;//信息长度
    return pdu;
}
