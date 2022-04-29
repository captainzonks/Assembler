CLEAR
CALL AddTwoProc
CALL AddAgainProc
LOAD Z
OUTPUT
HALT
AddTwoProc, PROC
LOAD X
ADD Y
STORE Z
RET
AddTwoProc, ENDP
AddAgainProc, PROC
LOAD Z
ADD X
STORE Z
RET
AddAgainProc, ENDP

X, DEC 4
Y, DEC 8
Z, DEC 0

END
