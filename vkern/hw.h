///////////////////////////////////////////////////////////////
//
// hw: Hardware abstration library
//
//
//   26.08.2010 00:35 - created
//

#ifndef _HW_H_INC_
#define _HW_H_INC_


EXTERN_C_BEGIN


BYTE KPI hwPortReadByte(WORD port);
void KPI hwPortWriteByte(WORD port, BYTE byte);

void KPI hwProcessorHalt(void);

void KPI hwDisableInterrupts(void);
void KPI hwEnableInterrupts(void);

void KPI hwSwitchGdtr(BYTE* pOldGdtr, BYTE* pNewGdtr);
void KPI hwSwitchIdtr(BYTE* pOldIdtr, BYTE* pNewIdtr);


EXTERN_C_END


#endif //_HW_H_INC_
