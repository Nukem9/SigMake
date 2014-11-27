#pragma once

enum SIGNATURE_TYPE
{
	SIG_CODE,
	SIG_IDA,
	SIG_CRC,
};

SIG_DESCRIPTOR *GenerateSigFromCode(duint Start, duint End);

bool MatchOperands(_DInst *Instruction, _Operand *Operands, int PrefixSize);
int MatchInstruction(_DInst *Instruction, PBYTE Data);