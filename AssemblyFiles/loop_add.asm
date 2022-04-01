LOOP,   LOAD Num
        ADD X
        STORE Num
        LOAD Y
        SUB One
        STORE Y
        SKIPCOND 400
        JMP LOOP
        LOAD Num
        HALT
X,      DEC 2
Y,      DEC 3
One,    DEC 1
Num,    DEC 0
        END

// Program will add X  Y times

