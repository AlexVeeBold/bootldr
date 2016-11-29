; serial port functions (16.08.2010 21:12)
;

%define SERPORT_BASE    03F8h
%define SERPORT_BAUDL       SERPORT_BASE+0  ;DLAB=1
%define SERPORT_BAUDH       SERPORT_BASE+1  ;DLAB=1
%define SERPORT_DATA        SERPORT_BASE+0  ;DLAB=0
%define SERPORT_INTEN       SERPORT_BASE+1  ;DLAB=0
%define SERPORT_INTID       SERPORT_BASE+2
%define SERPORT_LINECTL     SERPORT_BASE+3  ;bit7:DLAB (Divisor Latch Access Bit)
%define SERPORT_MDMCTL      SERPORT_BASE+4
%define SERPORT_LINESTAT    SERPORT_BASE+5
%define SERPORT_MDMSTAT     SERPORT_BASE+6
;_BAUDH, _BAUDL: baudrate divisor: 115200/baudrate
; (300 bps: 115200/300 = 384 = 180h: _BAUDH = 01h, _BAUDL = 80h)
;_INTEN: enable interrupts:
; [7:0 0 0 0 3:ModemStatus LineStatus TransferredChar 0:ReceivedChar]
; (modem status: CTS, DSR, RI, DCD)
;_INTID: interrupt identification
; [7:0 0 0 0 0 2,1:IntReason 0:PendingInts]
; IntReason: 11: line error, 10: received char, 01: transferred char, 00: line status changed
;_LINECTL:
; [7:DLAB ForceBreak SetParityBit 4:EvenParity EnableParity StopBits 1,0:DataBits]
; (data bits per chararacter = 5 + DataBits)
;_MDMCTL:
; [7:0 0 0 LoopbackTest 3:OUT2 OUT1 RTS 0:DTR]
;_LINESTAT:
; [7:0 TRsEmpty THRegEmpty LineBreak 3:FramingError ParityError RxBufferFull 0:ReceivedChar]
; (TRsEmpty = TSR & THR are empty; THRegEmpty = THR is empty)
; (TSR = Transmit Shift Register; THR = Transmit Holding Register)
; (data -> THR -> TSR -> serial line)
;_MDMSTAT:
; [7:DCD RI DSR CTS 3:DDCD DRI DDSR 0:DCTS]
; (bits 0-3 are reset when the CPU reads this register)
; (delta bits are indicating line state change since last query)


;;;;;;;;;;;;;;;; PORT FUNCTIONS ;;;;;;;;;;;;;;;;

[BITS 16]   ; Code for real mode (16-bit code)

kSerialPortInit:            ; no parameters, uses AL, DX
    call kSerialPortTransferWait
    mov al, 10000000b   ; DLAB=on, ForceBreak=off, Parity=off, StopBit=1, CharSize=5bit
    mov dx, SERPORT_LINECTL
    out dx, al
    xor al, al          ; set baudrate divisor
    ;mov al, 03h
    mov dx, SERPORT_BAUDH
    out dx, al
    ;xor al, al          ; baudrate=150 bps (divisor = 0x0300)
    ;mov al, 0C0h        ; baudrate=600 bps (divisor = 0x00C0)
    ;mov al, 0Ch         ; baudrate=9600 bps (divisor = 0x000C)
    ;mov al, 03h         ; baudrate=38400 bps (divisor = 0x0003)
    mov al, 01h         ; baudrate=115200 bps (divisor = 0x0001)
    mov dx, SERPORT_BAUDL
    out dx, al
    mov al, 00000011b   ; DLAB=off, ForceBreak=off, Parity=off, StopBit=1, CharSize=8bit
    mov dx, SERPORT_LINECTL
    out dx, al
    xor al, al          ; disable all serial port interrupts
    mov dx, SERPORT_INTEN
    out dx, al
    ret
;endof kSerialPortInit

kSerialPortReceiveWait:    ; no parameters, uses AL, DX
    mov dx, SERPORT_LINESTAT
.wait:
    in al, dx
    test al, 00000001b  ; byte received?
    jz .wait            ; no - wait again
    ret
;endof kSerialPortReceiveWait

kSerialPortTransferWait:    ; no parameters, uses AL, DX
    mov dx, SERPORT_LINESTAT
.wait:
    in al, dx
    test al, 00100000b  ; THR is empty?
    jz .wait            ; no - wait again
    ret
;endof kSerialPortTransferWait

kSerialPortReceiveByte:    ; DL <- byte, uses AL, DX
    call kSerialPortReceiveWait
    mov dx, SERPORT_DATA
    in al, dx
    mov dl, al
    ret
;endof kSerialPortReceiveByte

kSerialPortTransferByte:    ; DL = byte, uses AL, CL, DX
    mov cl, dl      ; save byte
    call kSerialPortTransferWait
    mov al, cl      ; restore byte and send it
    mov dx, SERPORT_DATA
    out dx, al
    ret
;endof kSerialPortTransferByte

kSerialPortReceiveBuffer:    ; ES:DI = buffer, CX = # of bytes, uses AL, CX, DX, DI
.loop:
    call kSerialPortReceiveWait
    mov dx, SERPORT_DATA
    in al, dx       ; receive byte
    stosb           ; AL -> memory contents at ES:DI , DI <- DI+1
    loop .loop      ; get next byte
    ret
;endof kSerialPortReceiveBuffer

kSerialPortTransferBuffer:    ; DS:SI = buffer, CX = min.length, uses AL, CX, DX, SI
.loop:
    call kSerialPortTransferWait
    lodsb           ; AL <- memory contents at DS:SI , SI <- SI+1
    mov dx, SERPORT_DATA
    out dx, al      ; send byte
    sub cx, 1       ; if minimal length
    cmp cx, 0       ;  not exceeded then
    jg .loop        ;  send next byte
    test al, al     ; if not AL=0 then
    jnz .loop       ;  send next byte
    ret
;endof kSerialPortTransferBuffer


