LOAD X
ADD Y
STORE Z
HALT
X, DEC 4
Y, DEC 8
Z, DEC 0
END


// Should assemble to 
// 0000 0x1004
// 0001 0x3005
// 0002 0x2006
// 0003 0x7000
// 0004 0x0004
// 0005 0x0008
// 0006 0x0000