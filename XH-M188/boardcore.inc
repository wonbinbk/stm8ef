; STM8S003F3P6 XH-M188 STM8S device dependent routine default code

; Note: for supporting a new board create a new board configuration
;       folder with a "globconfig.inc" and a copy of this file.

;       BOARDINIT  ( -- )
;       Init board GPIO (except COM ports)
;       IO on this board is unfettered when LED display is pulled
;       Buttons: Minus PB4, Plus PB5, On/Off PC4  -- default is input
;       PWM Voltage Control: PC6, Timer 1, Channel 1

BOARDINIT:
        ; Board I/O initialization
	; Init Timer for PWM
	; Unlock EEPROM and set AFR0 flag
        CALL    ULOCK
        MOV     FLASH_CR2, #0b10000000
        MOV     FLASH_NCR2,#0b01111111
	MOV     OPT2, #0b00000001
	MOV     NOPT2,#0b11111110
; should wait for EOP bit here...meh, seems to work
; then re-lock
        MOV     FLASH_CR2, #0b00000000
        MOV     FLASH_NCR2,#0b11111111
	CALL LOCK

	; Set voltage-control PWM timers
        MOV     TIM1_ARRH,#0x0D ; approx 4.5 kHz
        MOV     TIM1_ARRL,#0xFF 
        MOV     TIM1_CCR1H,#0x00 ; Start PWM off
        MOV     TIM1_CCR1L,#0x00 
        MOV     TIM1_CCER1,#0x01 ; output on chan 1 
        MOV     TIM1_CCMR1,#0x60 ; count-up mode
        MOV     TIM1_BKR,#0b10000000 ; output enable
	MOV     TIM1_CR1,#0b00000001 ; enable timer

        RET

;===============================================================

;      Dummy labels for PSIM interrupts declared in main.c

        .ifne   PSIM-PORTA
;       Dummy label for _EXTIA_IRQHandler
_EXTI0_IRQHandler:
        .endif

        .ifne   PSIM-PORTB
;       Dummy label for _EXTIB_IRQHandler
_EXTI1_IRQHandler:
        .endif

        .ifne   PSIM-PORTC
;       Dummy label for _EXTIC_IRQHandler
_EXTI2_IRQHandler:
        .endif

        .ifne   PSIM-PORTD
;       Dummy label for _EXTID_IRQHandler
_EXTI3_IRQHandler:
        .endif

;===============================================================

        .ifne   HAS_KEYS
;       BKEY  ( -- c )     ( TOS STM8: -- A,Z,N )
;       Read board key state as a bitfield
        .dw     LINK

        LINK =  .
        .db     (4)
        .ascii  "BKEY"
BKEY:
        ; Keys "minus" (1), "plus" (2), "on/off" (4)
        CLR     A
        BTJT    PB_IDR,#4,1$    ; test "minus"
        OR      A,#0x01
1$:     BTJT    PB_IDR,#5,2$    ; test "plus"
        OR      A,#0x02
2$:     BTJT    PC_IDR,#4,3$    ; test "on/off"
        OR      A,#0x04
3$:     JP      ASTOR

;       BKEYC  ( -- c )   ( TOS STM8: -- A,Z,N )
;       Read and translate board dependent key bitmap into char

BKEYCHAR:
        CALLR   BKEY
        JREQ    1$
        ADD     A,#'@'
        LD      (1,X),A
1$:     RET
       .endif

; ============================================================

; PWM! ( pwm -- ) 
; Set PWM duty cycle
; Max seems to be around 3300 (3583 is full on)
	.dw    LINK

	LINK =  .
        .db     (4)
        .ascii  "PWM!"
PWMSET:
        DoLitW  TIM1_CCR1H
        JP      DCSTOR

; MV! ( millivolts -- ) 
; Set PWM duty cycle
; Trim until it matches, or until 1024 PWM! just gives 5.000 volts
	.dw    LINK

	LINK =  .
        .db     (2)
        .ascii  "MV"
MVSET:
        DoLitW  1024  ; PWM @ calibration point 
        CALL    SWAPP
        DoLitW  5000  ; 5.000 volts @ cal point 
        CALL    STASL ; *\ 
	JP      PWMSET 


