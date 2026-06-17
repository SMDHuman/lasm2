//-----------------------------------------------------------------------------
// lasm2_assembler.h
// github.com/SMDHuman
//-----------------------------------------------------------------------------
#ifndef LASM2_ASSEMBLER_H
#define LASM2_ASSEMBLER_H

#include "lasm2_parser.h"

int lasm2_assemble_to_file(lines_t* lines, FILE* out_file);

#endif