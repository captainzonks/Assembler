JNS FirstJump
OUTPUT
HALT
FirstJump, DEC 0    // DEC 0 is where the return address is stored
LOAD X
SUB Y
STOREI StoragePtr
OUTPUT
JNS SecondJump
JMPI FirstJump
SecondJump, DEC 0
LOADI PTR
ADD W
STORE Z
JMPI SecondJump
PTR, DEC 20 // pointer to Q
StoragePtr, DEC 19
W, DEC 13   // value variables
X, DEC 4
Y, DEC 2
Z, DEC 0    // our storage variable
Q, DEC 5
END
