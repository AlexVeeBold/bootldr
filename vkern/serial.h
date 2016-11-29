///////////////////////////////////////////////////////////////
//
// serial: Serial port
//
//
//   26.08.2010 00:44 - created
//

#ifndef _SERIAL_H_INC_
#define _SERIAL_H_INC_


EXTERN_C_BEGIN


void KPI kSerialPortInit(DWORD baudrate);
void KPI kSerialPortTransferByte(BYTE byData);
void KPI kSerialPortTransferBuffer(BYTE* byBuffer, DWORD dwBufferLength);


EXTERN_C_END


#endif //_SERIAL_H_INC_
