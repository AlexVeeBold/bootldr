; PS/2 port functions (18.08.2010 15:34)
;

%define PS2PORT_DATA        0060h   ; (read/write)
%define PS2PORT_OUTPUT      0060h   ; (read)
%define PS2PORT_INPUT       0064h   ; (write)
%define PS2PORT_STATUS      0064h   ; (read)
%define PS2PORT_CONTROL_B   0061h
%define PS2PORT_CONTROL_A   0092h
; some commands for _INPUT
%define PS2_CMD_KBD_DISABLE 0ADh
%define PS2_CMD_KBD_ENABLE  0AEh
%define PS2_CMD_READ_OP     0D0h    ; read output port (followed by: read from _OUTPUT)
%define PS2_CMD_WRITE_OP    0D1h    ; write output port (followed by: write to _OUTPUT)
%define PS2_CMD_A20_DISABLE 0DDh
%define PS2_CMD_A20_ENABLE  0DFh
;_STATUS (ISA/EISA)
; [7:ParityError ReceiveTimeout 5:TransmitTimeout KbdLock WrittenIsCmd 2:TestOk InputBufferFull 0:OutputBufferReady]
;_STATUS (ISA/EISA)
; [7:ParityError ExchangeTimeout 5:MouseData KbdEnabled
;  LastWrittenIsData 2:SysInitDone InputBufferFull 0:OutputBufferReady]
; (data -> inputBuffer -> 8042)
; (data <- outputBuffer <- 8042)
; do not write to _DATA while _STATUS.InputBufferFull=1
; do not read from _DATA while _STATUS.OutputBufferReady=0
;_OUTPUT (AT)
; [7:KbdDataOutput KbdClockOutput 5:InputBufferNotFull OutputBufferNotEmpty 3,2:reserved GateA20 0:SystemReset]
;_CONTROL_A (PS/2)
; [7,6:?ActivityLights? unused 4:WatchdogTimeout CmosLock 2:unused A20IsActive 0:?SystemReset?]


;;;;;;;;;;;;;;;; PS/2 PORT FUNCTIONS ;;;;;;;;;;;;;;;;

[BITS 16]   ; Code for real mode (16-bit code)

kPs2PortWriteWait:          ; no parameters, uses AL, DX
    mov dx, PS2PORT_STATUS
.wait:
    in al, dx
    test al, 00000010b  ; input buffer full?
    jnz .wait           ; yes - wait again
    ret
;endof kPs2PortWriteWait

kPs2PortReadWait:           ; no parameters, uses AL, DX
    mov dx, PS2PORT_STATUS
.wait:
    in al, dx
    test al, 00000001b  ; output buffer ready?
    jz .wait            ; no - wait again
    ret
;endof kPs2PortWriteWait

kPs2PortEnableA20_old:       ; no parameters, uses AL, CL, DX
    cli
    call kPs2PortWriteWait
    mov al, PS2_CMD_READ_OP
    mov dx, PS2PORT_INPUT
    out dx, al      ; send read-output-port command
    call kPs2PortReadWait
    mov dx, PS2PORT_OUTPUT
    in al, dx       ; receive status
    mov cl, al
    or cl, 2        ; set a20 bit
    call kPs2PortWriteWait
    mov al, PS2_CMD_WRITE_OP
    mov dx, PS2PORT_INPUT
    out dx, al      ; send write-output-port command
    call kPs2PortWriteWait
    mov al, cl
    mov dx, PS2PORT_OUTPUT
    out dx, al      ; send modified status
    sti
    ret
;endof kPs2PortEnableA20_old

kPs2PortEnableA20_fast:      ; no parameters, uses AL, DX
    mov dx, PS2PORT_CONTROL_A
    in al, dx       ; read control state
    or al, 2        ; activate a20
    out dx, al      ; write new control state
    ret
;endof kPs2PortEnableA20_fast

