#pragma once

#include "CommandTable.h"

// basic math functions
extern CommandInfo kCommandInfo_SquareRoot;
extern CommandInfo kCommandInfo_Log;
extern CommandInfo kCommandInfo_Exp;
extern CommandInfo kCommandInfo_Pow;
extern CommandInfo kCommandInfo_Log10;
extern CommandInfo kCommandInfo_Floor;
extern CommandInfo kCommandInfo_Ceil;
extern CommandInfo kCommandInfo_Abs;
extern CommandInfo kCommandInfo_Rand;
extern CommandInfo kCommandInfo_Fmod;

// trig functions in degrees
extern CommandInfo kCommandInfo_dSin;
extern CommandInfo kCommandInfo_dCos;
extern CommandInfo kCommandInfo_dTan;
extern CommandInfo kCommandInfo_dASin;
extern CommandInfo kCommandInfo_dACos;
extern CommandInfo kCommandInfo_dATan;
extern CommandInfo kCommandInfo_dATan2;
extern CommandInfo kCommandInfo_dSinh;
extern CommandInfo kCommandInfo_dCosh;
extern CommandInfo kCommandInfo_dTanh;


// trig functions in radians
extern CommandInfo kCommandInfo_Sin;
extern CommandInfo kCommandInfo_Cos;
extern CommandInfo kCommandInfo_Tan;
extern CommandInfo kCommandInfo_ASin;
extern CommandInfo kCommandInfo_ACos;
extern CommandInfo kCommandInfo_ATan;

extern CommandInfo kCommandInfo_ATan2;
extern CommandInfo kCommandInfo_Sinh;
extern CommandInfo kCommandInfo_Cosh;
extern CommandInfo kCommandInfo_Tanh;

// bitwise functions
extern CommandInfo kCommandInfo_LeftShift;
extern CommandInfo kCommandInfo_RightShift;
extern CommandInfo kCommandInfo_LogicalAnd;
extern CommandInfo kCommandInfo_LogicalOr;
extern CommandInfo kCommandInfo_LogicalXor;
extern CommandInfo kCommandInfo_LogicalNot;

// matrix functions
extern CommandInfo kCommandInfo_GenerateZeroMatrix;
extern CommandInfo kCommandInfo_GenerateIdentityMatrix;
extern CommandInfo kCommandInfo_GenerateRotationMatrix;
extern CommandInfo kCommandInfo_VectorMagnitude;
extern CommandInfo kCommandInfo_VectorNormalize;
extern CommandInfo kCommandInfo_VectorDot;
extern CommandInfo kCommandInfo_VectorCross;
extern CommandInfo kCommandInfo_ForceRowVector;
extern CommandInfo kCommandInfo_ForceColumnVector;
extern CommandInfo kCommandInfo_MatrixTrace;
extern CommandInfo kCommandInfo_MatrixDeterminant;
extern CommandInfo kCommandInfo_MatrixRREF;
extern CommandInfo kCommandInfo_MatrixInvert;
extern CommandInfo kCommandInfo_MatrixTranspose;
extern CommandInfo kCommandInfo_MatrixScale;
extern CommandInfo kCommandInfo_MatrixAdd;
extern CommandInfo kCommandInfo_MatrixSubtract;
extern CommandInfo kCommandInfo_MatrixMultiply;
/*extern CommandInfo kCommandInfo_MatrixEigenvalues;
extern CommandInfo kCommandInfo_MatrixEigenvectors;
*/