start:  not r0 r1
        and r0 r0 r1    ; whatever
        not r1 r0       ; and 
        add r1 r1 r1
        not r1 r1
loop:   and r3 r0 r0
        add r0 r0 r1
        bnz loop
        and r3 r3 r3
stop:   bnz stop        ; whatever
