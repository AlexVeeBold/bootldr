///////////////////////////////////////////////////////////////
//
// pic: Programmable Interrupt Controller (Intel 8259A)
//
//
//   06.09.2010 13:15 - created
//

#ifndef _PIC_H_INC_
#define _PIC_H_INC_


EXTERN_C_BEGIN


void KPI kPicInit(BYTE primaryBaseVector, BYTE secondaryBaseVector);
void KPI kPicClearBoth(void);


EXTERN_C_END


#endif //_PIC_H_INC_
