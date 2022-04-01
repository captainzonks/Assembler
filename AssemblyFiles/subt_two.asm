LOAD X
SUB Y
STORE Z
HALT
X, DEC 10
Y, DEC 5
Z, DEC 0
END


// Should assemble to 
// 0000 0x1004
// 0001 0x4005
// 0002 0x2006
// 0003 0x7000
// 0004 0x000a
// 0005 0x0005
// 0006 0x0000