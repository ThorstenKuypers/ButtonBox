#ifndef _PDO_H_
#define _PDO_H_

EXTERN_C_START


EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL ButtonBoxHidInternalDeviceControl;
EVT_WDF_TIMER ButtonBoxEvtReadInput;

EXTERN_C_END

#endif // _PDO_H_
