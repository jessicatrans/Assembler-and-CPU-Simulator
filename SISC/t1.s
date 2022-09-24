; a simple example
; test program that counts up to 255
;
start:  not r0 r1
        and r0 r0 r1     ; value and it's completment is zero
        not r1 r0        ; r1 contains 255
        add r1 r1 r1     ; r1 contains 254
        not r1 r1        ; not 254 = 1
loop:   and r3 r0 r0     ; r3 now has zero
        add r0 r0 r1     ; increment by 1
        bnz loop         ; repeat until r0 = 256 = 0
        and r3 r3 r3     ; and r3 with itself to set flags
                         ; on the display, it's not zero   
stop:   bnz stop         ; r3 is never zero, effectively stop

