; serial port functions
;

;;;;;;;;;;;;;;;; PORT FUNCTIONS ;;;;;;;;;;;;;;;;

[BITS 32]   ; Code for protected mode (32 bit)

kSerialPortTransferWait32:      ; no parameters, uses AL, DX
    mov dx, SERPORT_LINESTAT
.wait:
    in al, dx
    test al, 00100000b  ; THR is empty?
    jz .wait            ; no - wait again
    ret
;endof kSerialPortTransferWait32

kSerialPortTransferByte32:      ; DL = byte, uses AL, CL, DX
    mov cl, dl      ; save byte
    call kSerialPortTransferWait32
    mov al, cl      ; restore byte and send it
    mov dx, SERPORT_DATA
    out dx, al
    ret
;endof kSerialPortTransferByte32

kSerialPortTransferBuffer32:    ; DS:ESI = buffer, CX = min.length, uses AL, CX, DX, ESI
.loop:
    call kSerialPortTransferWait32
    lodsb           ; AL <- memory contents at DS:ESI , ESI <- ESI+1
    mov dx, SERPORT_DATA
    out dx, al      ; send byte
    sub cx, 1       ; if minimal length
    cmp cx, 0       ;  not exceeded then
    jg .loop        ;  send next byte
    test al, al     ; if not AL=0 then
    jnz .loop       ;  send next byte
    ret
;endof kSerialPortTransferBuffer32
