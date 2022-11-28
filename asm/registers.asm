copyRegsManual:
SW r0, 0x0000 (gp)
SW at, 0x0004 (gp)
SW v0, 0x0008 (gp)
SW v1, 0x000C (gp)
SW a0, 0x0010 (gp)
SW a1, 0x0014 (gp)
SW a2, 0x0018 (gp)
SW a3, 0x001C (gp)
SW t0, 0x0020 (gp)
SW t1, 0x0024 (gp)
SW t2, 0x0028 (gp)
SW t3, 0x002C (gp)
SW t4, 0x0030 (gp)
SW t5, 0x0034 (gp)
SW t6, 0x0038 (gp)
SW t7, 0x003C (gp)
SW s0, 0x0040 (gp)
SW s1, 0x0044 (gp)
SW s2, 0x0048 (gp)
SW s3, 0x004C (gp)
SW s4, 0x0050 (gp)
SW s5, 0x0054 (gp)
SW s6, 0x0058 (gp)
SW s7, 0x005C (gp)
SW t8, 0x0060 (gp)
SW t9, 0x0064 (gp)
SW k0, 0x0068 (gp)
SW k1, 0x006C (gp)
SW gp, 0x0070 (gp)
SW sp, 0x0074 (gp)
SW fp, 0x0078 (gp)
SW ra, 0x007C (gp)
//FPRs
SDC1 f0, 0x0080 (gp)
SDC1 f2, 0x0088 (gp)
SDC1 f4, 0x0090 (gp)
SDC1 f6, 0x0098 (gp)
SDC1 f8, 0x00A0 (gp)
SDC1 f10, 0x00A8 (gp)
SDC1 f12, 0x00B0 (gp)
SDC1 f14, 0x00B8 (gp)
SDC1 f16, 0x00C0 (gp)
SDC1 f18, 0x00C8 (gp)
SDC1 f20, 0x00D0 (gp)
SDC1 f22, 0x00D8 (gp)
SDC1 f24, 0x00E0 (gp)
SDC1 f26, 0x00E8 (gp)
SDC1 f28, 0x00F0 (gp)
SDC1 f30, 0x00F8 (gp)
//COP0 regs
MFC0 t0, $0
SW t0, 0x0100 (gp)
NOP
MFC0 t0, $1
SW t0, 0x0104 (gp)
NOP
MFC0 t0, $2
SW t0, 0x0108 (gp)
NOP
MFC0 t0, $3
SW t0, 0x010C (gp)
NOP
MFC0 t0, $4
SW t0, 0x0110 (gp)
MFC0 t0, $0
SW t0, 0x0114 (gp)
NOP
MFC0 t0, $5
SW t0, 0x0118 (gp)
NOP
MFC0 t0, $6
SW t0, 0x011C (gp)
NOP
MFC0 t0, $7
SW t0, 0x0120 (gp)
NOP
MFC0 t0, $8
SW t0, 0x0124 (gp)
NOP
MFC0 t0, $9
SW t0, 0x0128 (gp)
NOP
MFC0 t0, $10
SW t0, 0x012C (gp)
NOP

MFC0 t0, $11
SW t0, 0x0130 (gp)
NOP

MFC0 t0, $12
SW t0, 0x0134 (gp)
NOP

MFC0 t0, $13
SW t0, 0x0138 (gp)
NOP

MFC0 t0, $14
SW t0, 0x013C (gp)
NOP

MFC0 t0, $15
SW t0, 0x0140 (gp)
NOP

MFC0 t0, $16
SW t0, 0x0144 (gp)
NOP

MFC0 t0, $17
SW t0, 0x0148 (gp)
NOP

MFC0 t0, $18
SW t0, 0x014C (gp)
NOP

MFC0 t0, $19
SW t0, 0x0150 (gp)
NOP

MFC0 t0, $20
SW t0, 0x0154 (gp)
NOP

MFC0 t0, $21
SW t0, 0x0158 (gp)
NOP

MFC0 t0, $22
SW t0, 0x015C (gp)
NOP

MFC0 t0, $23
SW t0, 0x0160 (gp)
NOP

MFC0 t0, $24
SW t0, 0x0164 (gp)
NOP

MFC0 t0, $25
SW t0, 0x0168 (gp)
NOP

MFC0 t0, $26
SW t0, 0x016C (gp)
NOP

MFC0 t0, $27
SW t0, 0x0170 (gp)
NOP

MFC0 t0, $28
SW t0, 0x0174 (gp)
NOP

MFC0 t0, $29
SW t0, 0x0178 (gp)
NOP

MFC0 t0, $30
SW t0, 0x017C (gp)
NOP

MFC0 t0, $31
SW t0, 0x0180 (gp)
NOP

JR RA
NOP

loadRegsManual:
LW r0, 0x0000 (gp)
LW at, 0x0004 (gp)
LW v0, 0x0008 (gp)
LW v1, 0x000C (gp)
LW a0, 0x0010 (gp)
LW a1, 0x0014 (gp)
LW a2, 0x0018 (gp)
LW a3, 0x001C (gp)
LW t0, 0x0020 (gp)
LW t1, 0x0024 (gp)
LW t2, 0x0028 (gp)
LW t3, 0x002C (gp)
LW t4, 0x0030 (gp)
LW t5, 0x0034 (gp)
LW t6, 0x0038 (gp)
LW t7, 0x003C (gp)
LW s0, 0x0040 (gp)
LW s1, 0x0044 (gp)
LW s2, 0x0048 (gp)
LW s3, 0x004C (gp)
LW s4, 0x0050 (gp)
LW s5, 0x0054 (gp)
LW s6, 0x0058 (gp)
LW s7, 0x005C (gp)
LW t8, 0x0060 (gp)
LW t9, 0x0064 (gp)
LW k0, 0x0068 (gp)
LW k1, 0x006C (gp)
LW gp, 0x0070 (gp)
LW sp, 0x0074 (gp)
LW fp, 0x0078 (gp)
//LW ra, 0x007C (gp)
//FPRs
LDC1 f0, 0x0080 (gp)
LDC1 f2, 0x0088 (gp)
LDC1 f4, 0x0090 (gp)
LDC1 f6, 0x0098 (gp)
LDC1 f8, 0x00A0 (gp)
LDC1 f10, 0x00A8 (gp)
LDC1 f12, 0x00B0 (gp)
LDC1 f14, 0x00B8 (gp)
LDC1 f16, 0x00C0 (gp)
LDC1 f18, 0x00C8 (gp)
LDC1 f20, 0x00D0 (gp)
LDC1 f22, 0x00D8 (gp)
LDC1 f24, 0x00E0 (gp)
LDC1 f26, 0x00E8 (gp)
LDC1 f28, 0x00F0 (gp)
LDC1 f30, 0x00F8 (gp)
//COP0 regs
LW t0, 0x0100 (gp)
MTC0 t0, $0 //index
NOP
//LW t0, 0x0104 (gp)
//MTC0 t0, $1 //random
//NOP
LW t0, 0x0108 (gp)
MTC0 t0, $2
NOP
LW t0, 0x010C (gp)
MTC0 t0, $3
NOP
LW t0, 0x0110 (gp)
MTC0 t0, $4
NOP
LW t0, 0x0114 (gp)
MTC0 t0, $0
NOP
LW t0, 0x0118 (gp)
MTC0 t0, $5
NOP
LW t0, 0x011C (gp)
MTC0 t0, $6
NOP
LW t0, 0x0120 (gp)
MTC0 t0, $7
NOP
LW t0, 0x0124 (gp)
MTC0 t0, $8
NOP
LW t0, 0x0128 (gp)
MTC0 t0, $9
NOP
LW t0, 0x012C (gp)
MTC0 t0, $10
NOP

LW t0, 0x0130 (gp)
MTC0 t0, $11
NOP

LW t0, 0x0134 (gp)
MTC0 t0, $12
NOP

LW t0, 0x0138 (gp)
MTC0 t0, $13
NOP

LW t0, 0x013C (gp)
MTC0 t0, $14
NOP

LW t0, 0x0140 (gp)
MTC0 t0, $15
NOP

LW t0, 0x0144 (gp)
MTC0 t0, $16
NOP

LW t0, 0x0148 (gp)
MTC0 t0, $17
NOP

LW t0, 0x014C (gp)
MTC0 t0, $18
NOP

LW t0, 0x0150 (gp)
MTC0 t0, $19
NOP

LW t0, 0x0154 (gp)
MTC0 t0, $20
NOP

LW t0, 0x0158 (gp)
MTC0 t0, $21
NOP

LW t0, 0x015C (gp)
MTC0 t0, $22
NOP

LW t0, 0x0160 (gp)
MTC0 t0, $23
NOP

LW t0, 0x0164 (gp)
MTC0 t0, $24
NOP

LW t0, 0x0168 (gp)
MTC0 t0, $25
NOP

LW t0, 0x016C (gp)
MTC0 t0, $26
NOP

LW t0, 0x0170 (gp)
MTC0 t0, $27
NOP

LW t0, 0x0174 (gp)
MTC0 t0, $28
NOP

LW t0, 0x0178 (gp)
MTC0 t0, $29
NOP

LW t0, 0x017C (gp)
MTC0 t0, $30
NOP

LW t0, 0x0180 (gp)
MTC0 t0, $31
NOP

JR RA
NOP