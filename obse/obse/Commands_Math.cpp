#include "Commands_Math.h"
#include "ParamInfos.h"
#include "Utilities.h"
#include "Script.h"
#include "ScriptUtils.h"

//Some basic maths functions

#if OBLIVION

#include "GameAPI.h"

// basic math functions

bool Cmd_SquareRoot_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = sqrt(arg);

	return true;
}

bool Cmd_Log_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = log(arg);

	return true;
}
bool Cmd_Exp_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = exp(arg);

	return true;
}

bool Cmd_Log10_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = log10(arg);

	return true;
}

bool Cmd_Pow_Execute(COMMAND_ARGS)
{
	*result = 0;

	float f1 = 0;
	float f2 = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &f1, &f2)) return true;

	*result = pow(f1,f2);

	return true;
}

bool Cmd_Floor_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = floor(arg);

	return true;
}
bool Cmd_Ceil_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = ceil(arg);

	return true;
}
bool Cmd_Abs_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = abs(arg);

	return true;
}
bool Cmd_Rand_Initialized=false;
bool Cmd_Rand_Execute(COMMAND_ARGS)
{
	if(!Cmd_Rand_Initialized) {
		Cmd_Rand_Initialized=true;
		MersenneTwister::init_genrand(GetTickCount());
	}
	*result = 0;

	float rangeMin = 0;
	float rangeMax = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &rangeMin, &rangeMax)) return true;

	if(rangeMax < rangeMin)
	{
		float	temp = rangeMin;
		rangeMin = rangeMax;
		rangeMax = temp;
	}

	float	range = rangeMax - rangeMin;

	double	value = MersenneTwister::genrand_real2() * range;
	value += rangeMin;

	*result = value;

	return true;
}

// trig functions using radians

bool Cmd_Sin_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = sin(arg);

	return true;
}
bool Cmd_Cos_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = cos(arg);

	return true;
}
bool Cmd_Tan_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = tan(arg);

	return true;
}
bool Cmd_ASin_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = asin(arg);

	return true;
}
bool Cmd_ACos_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = acos(arg);

	return true;
}
bool Cmd_ATan_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = atan(arg);

	return true;
}

bool Cmd_ATan2_Execute(COMMAND_ARGS)
{
	*result = 0;

	float f1 = 0;
	float f2 = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &f1, &f2)) return true;

	*result = atan2(f1,f2);

	return true;
}
bool Cmd_Sinh_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = sinh(arg);

	return true;
}
bool Cmd_Cosh_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = cosh(arg);

	return true;
}
bool Cmd_Tanh_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = tanh(arg);

	return true;
}

// trig functions using degrees
#define DEGTORAD 0.01745329252f

bool Cmd_dSin_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = sin(arg*DEGTORAD);

	return true;
}
bool Cmd_dCos_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = cos(arg*DEGTORAD);

	return true;
}
bool Cmd_dTan_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = tan(arg*DEGTORAD);

	return true;
}
bool Cmd_dASin_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = asin(arg)/DEGTORAD;

	return true;
}
bool Cmd_dACos_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = acos(arg)/DEGTORAD;

	return true;
}
bool Cmd_dATan_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = atan(arg)/DEGTORAD;

	return true;
}
bool Cmd_dATan2_Execute(COMMAND_ARGS)
{
	*result = 0;

	float f1 = 0;
	float f2 = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &f1, &f2)) return true;

	*result = atan2(f1,f2)/DEGTORAD;

	return true;
}
bool Cmd_dSinh_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = sinh(arg*DEGTORAD);

	return true;
}
bool Cmd_dCosh_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = cosh(arg*DEGTORAD);

	return true;
}
bool Cmd_dTanh_Execute(COMMAND_ARGS)
{
	*result = 0;

	float arg = 0;
	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &arg))
		return true;

	*result = tanh(arg*DEGTORAD);

	return true;
}

bool Cmd_LeftShift_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32	lhs = 0;
	UInt32	rhs = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &lhs, &rhs))
		return true;

	if(rhs >= 32)
		*result = 0;
	else
		*result = lhs << rhs;

	return true;
}

bool Cmd_RightShift_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32	lhs = 0;
	UInt32	rhs = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &lhs, &rhs))
		return true;

	if(rhs >= 32)
		*result = 0;
	else
		*result = lhs >> rhs;

	return true;
}

bool Cmd_LogicalAnd_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32	lhs = 0;
	UInt32	rhs = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &lhs, &rhs))
		return true;

	*result = lhs & rhs;

	return true;
}

bool Cmd_LogicalOr_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32	lhs = 0;
	UInt32	rhs = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &lhs, &rhs))
		return true;

	*result = lhs | rhs;

	return true;
}

bool Cmd_LogicalXor_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32	lhs = 0;
	UInt32	rhs = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &lhs, &rhs))
		return true;

	*result = lhs ^ rhs;

	return true;
}

#pragma warning (push)
#pragma warning (disable : 4319)

bool Cmd_LogicalNot_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32	lhs = 0;

	if(!ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &lhs))
		return true;

	*result = (double)(~lhs);

	return true;
}

#pragma warning (pop)

bool Cmd_Fmod_Execute(COMMAND_ARGS)
{
	*result = 0;
	float x = 0.0;
	float n = 0.0;
	float offset = 0.0;

	ExtractArgs(paramInfo, arg1, opcodeOffsetPtr, thisObj, arg3, scriptObj, eventList, &x, &n, &offset);

	float answer = x - n * floor(x/n);
	if (offset != 0) {
		float bump = n * floor(offset/n);
		float bound = offset - bump;
		answer += bump;
		bool bBigger = (n > 0);
		if ( (bBigger && answer < bound) || (!bBigger && answer > bound) ) {
			answer += n;
		}
	}
	*result = answer;

	return true;
}

// matrix mathematics
class Matrix {
public:
	bool isMat;
	ArrayID id;
	ArrayVar* arr;
	std::vector<ArrayVar*> rows;
	bool is2d;
	double h;
	double w;
	double y;
	double x;

	Matrix() : isMat(false), arr(NULL), h(0), w(0), x(0), y(0) {
	}

	Matrix(const ArrayID arrID) : isMat(true), id(arrID), arr(g_ArrayMap.Get(arrID)), h(0), w(0), y(0), x(0) {
		if ( !arr || !(arr->IsPacked()) )
			isMat = false;
		else {
			h = arr->Size();
			if ( h <= 0 )
				isMat = false;
			else {
				ArrayElement* rele = arr->Get((double)0, false);
				ArrayID row;
				if ( !rele->GetAsArray(&row) ) {
					is2d = false;
					w = h;
					rows.push_back(NULL);
					rows[0] = arr;
				}
				else {
					is2d = true;
					rows.push_back(NULL);
					rows[0] = g_ArrayMap.Get(row);
					w = rows[0]->Size();
					if ( w <= 0 )
						isMat = false;
					else {
						ArrayID row;
						for ( UInt32 i = 1; i < h; ++i ) {
							rele = arr->Get(i, false);
							if ( rele->GetAsArray(&row) ) {
								rows.push_back(NULL);
								rows[i] = g_ArrayMap.Get(row);
								if ( rows[i]->Size() != w ) {
									isMat = false;
									break;
								}
							}
							else {
								isMat = false;
								break;
							}
						}
					}
				}
			}
		}
	}

	Matrix(const ArrayID arrID, const double length) : isMat(true), id(arrID), arr(g_ArrayMap.Get(arrID)), is2d(false), h(length), w(length), y(0), x(0) {
		rows.push_back(NULL);
		rows[y] = arr;
	}

	Matrix(const ArrayID arrID, const double width, const double height) : isMat(true), id(arrID), arr(g_ArrayMap.Get(arrID)), is2d(true), h(height), w(width), y(0), x(0) {
		for ( x = 0; x < w; ++x ) {
			ArrayID rid;
			arr->Get(y, false)->GetAsArray(&rid);
			rows.push_back(NULL);
			rows[y] = g_ArrayMap.Get(rid);
		}
	}

	Matrix(const UInt8 modID, const double length) : isMat(true), id(g_ArrayMap.CreateArray(modID)), is2d(false), h(length), w(length), y(0), x(0) {
		arr = g_ArrayMap.Get(id);
		rows.push_back(NULL);
		rows[0] = arr;
		for ( y = 0; y < h; ++y )
			arr->Get(y, true)->SetNumber(0);
	}

	Matrix(const UInt8 modID, const double height, const double width) : isMat(true), id(g_ArrayMap.CreateArray(modID)), is2d(true), h(height), w(width), y(0), x(0) {
		arr = g_ArrayMap.Get(id);
		for ( y = 0; y < h; ++y ) {
			ArrayID rid = g_ArrayMap.CreateArray(modID);
			arr->Get(x, true)->SetArray(rid, modID);
			rows[x] = g_ArrayMap.Get(rid);
			for ( x = 0; x < w; ++x )
				rows[y]->Get(x, true)->SetNumber(0);
		}
	}

	// creates an h x w Matrix in which all elements are 0.
	static Matrix genZeroMatrix(const double height, const double width, const UInt8 modID) {
		Matrix mat = Matrix();
		mat.h = height;
		mat.w = width;
		if ( mat.h == 1 || mat.w == 1 ) {
			if ( mat.h == 1 )
				mat.h = mat.w;
			else
				mat.w = mat.h;
			mat.is2d = false;
		}
		else
			mat.is2d = true;
		mat.id = g_ArrayMap.CreateArray(modID);
		mat.arr = g_ArrayMap.Get(mat.id);
		if ( !(mat.is2d) ) {
			mat.rows.push_back(mat.arr);
			for ( mat.x = 0; mat.x < mat.w; ++(mat.x) )
				mat.rows[0]->Get(mat.x, true)->SetNumber((double)0);
		}
		else {
			for ( mat.y = 0; mat.y < mat.h; ++(mat.y) ) {
				ArrayID rid = g_ArrayMap.CreateArray(modID);
				mat.rows.push_back(g_ArrayMap.Get(rid));
				mat.arr->Get(mat.y, true)->SetArray(rid, modID);
				for ( mat.x = 0; mat.x < mat.w; ++(mat.x) )
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber((double)0);
			}
		}
		mat.isMat = true;
		return mat;
	}

	// creates a d x d square Matrix in which all elements
	// along the diagonal are 1 and all other elements are 0.
	static Matrix genIdentityMatrix(const double dim, const UInt8 modID) {
		Matrix mat = Matrix();
		mat.h = mat.w = dim;
		if ( dim == 1 )
			mat.is2d = false;
		else
			mat.is2d = true;
		mat.id = g_ArrayMap.CreateArray(modID);
		mat.arr = g_ArrayMap.Get(mat.id);
		if ( !(mat.is2d) ) {
			mat.rows.push_back(mat.arr);
			mat.rows[0]->Get((double)0, true)->SetNumber((double)1);
		}
		else {
			for ( mat.y = 0; mat.y < mat.w; ++(mat.y) ) {
				ArrayID rid = g_ArrayMap.CreateArray(modID);
				mat.rows.push_back(g_ArrayMap.Get(rid));
				mat.arr->Get(mat.y, true)->SetArray(rid, modID);
				for ( mat.x = 0; mat.x < mat.w; ++(mat.x) )
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber((double)(mat.x==mat.y?1:0));
			}
		}
		mat.isMat = true;
		return mat;
	}

	// creates a 3x3 Matrix that can be left-multiplied with a 3-vector or another 3x3 Matrix
	// to cause a rotation around the given axis. Rotation given in degrees.
	static Matrix genRotationMatrix(const char axis, const double theta, const UInt8 modID) {
		const double pi = 3.1415926535;
		double costheta = cos(pi/180 * theta);
		double sintheta = sin(pi/180 * theta);

		// each axis uses the number 1 once,
		// the cosine of the angle twice,
		// and the sine and negative sine once each
		// all other elements are 0.
		double onex;	double oney;
		double cosax;	double cosay;
		double cosbx;	double cosby;
		double sinx;	double siny;
		double nsinx;	double nsiny;
		switch(axis) {
			case 'x':
			case 'X':
				onex = 0;	oney = 0;
				cosax = 1;	cosay = 1;
				nsinx = 2;	nsiny = 1;
				sinx = 1;	siny = 2;
				cosbx = 2;	cosby = 2;
				break;

			case 'y':
			case 'Y':
				cosax = 0;	cosay = 0;
				sinx = 2;	siny = 0;
				onex = 1;	oney = 1;
				nsinx = 0;	nsiny = 2;
				cosbx = 2;	cosby = 2;
				break;

			case 'z':
			case 'Z':
			default:
				cosax = 0;	cosay = 0;
				nsinx = 1;	nsiny = 0;
				sinx = 0;	siny = 1;
				cosbx = 1;	cosby = 1;
				onex = 2;	oney = 2;
				break;
		}
		Matrix mat = Matrix();
		mat.h = 3;
		mat.w = 3;
		mat.is2d = true;
		mat.id = g_ArrayMap.CreateArray(modID);
		mat.arr = g_ArrayMap.Get(mat.id);
		for ( mat.y = 0; mat.y < 3; ++(mat.y) ) {
			ArrayID rid = g_ArrayMap.CreateArray(modID);
			mat.rows.push_back(g_ArrayMap.Get(rid));
			mat.arr->Get(mat.y, true)->SetArray(rid, modID);
			for ( mat.x = 0; mat.x < 3; ++(mat.x) )
				if ( ( mat.x == cosax && mat.y == cosay ) || ( mat.x == cosbx && mat.y == cosby ) )
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber(costheta);
				else if ( mat.x == sinx && mat.y == siny )
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber(sintheta);
				else if ( mat.x == nsinx && mat.y == nsiny )
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber(-sintheta);
				else if ( mat.x == onex && mat.y == oney )
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber((double)1);
				else
					mat.rows[mat.y]->Get(mat.x, true)->SetNumber((double)0);
		}
		mat.isMat = true;
		return mat;
	}

	bool isVector() const {
		return ( !is2d || w == 1 || h == 1 );
	}

	double lenVector() const {
		return (w==1?h:w);
	}

	bool dimEqual(const Matrix& mat) const {
		if ( is2d == mat.is2d ) {
			if ( w == mat.w && h == mat.h )
				return true;
			else
				return false;
		}
		else if ( isVector() && mat.isVector() && ( lenVector() == mat.lenVector() ) )
			return true;
		return false;
	}

	double getFirstElement() {
		if ( !isMat )
			throw std::exception("Not Matrix");

		x = 0;
		y = 0;
		double v;
		ArrayElement* val = rows[x]->Get(y, false);
		if ( !val || !(val->GetAsNumber(&v)) ) {
			isMat = false;
			throw std::exception("Element not number");
		}
		return v;
	}

	double getNextElement() {
		if ( !isMat )
			throw std::exception("Not matrix");

		++x;
		if ( x >= w ) {
			++y;
			x = 0;
			if ( !is2d || y >= h ) {
				x = 0;
				y = 0;
				throw std::exception("Done");
			}
		}
		double v;
		ArrayElement* val = rows[y]->Get(x, false);
		if ( !val || !(val->GetAsNumber(&v)) ) {
			isMat = false;
			throw std::exception("Element not number");
		}
		return v;
	}

	// creates a new matrix that will be a copy of *this
	// "copy" is where the copy will be stored (should be empty at this point).
	// returns the value of the first element of the original Matrix.
	double copyEditStart(Matrix& copy) {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( copy.arr )
			throw std::exception("Copy not empty");
		copy.isMat = true;
		copy.id = g_ArrayMap.CreateArray(arr->m_owningModIndex);
		copy.arr = g_ArrayMap.Get(copy.id);
		copy.h = h;
		copy.w = w;
		copy.is2d = is2d;
		copy.rows.push_back(copy.arr);
		if ( is2d ) {
			ArrayID rid = g_ArrayMap.CreateArray(arr->m_owningModIndex);
			copy.arr->Get((double)0, true)->SetArray(rid, arr->m_owningModIndex);
			copy.rows[0] = g_ArrayMap.Get(rid);
		}
		return getFirstElement();
	}

	// accepts a value to put into the copied Matrix
	// passing the result of copyEditStart or copyEditContinue
	// will simply copy it across; passing modifications of those
	// return values will edit the copy.
	// returns the next value each time.
	double copyEditContinue(double v, Matrix& copy) {
		try {
			copy.rows[y]->Get(x, true)->SetNumber(v);
			v = getNextElement();
		}
		catch (std::exception& e) {
			if ( std::string(e.what()).compare("Done") != 0 )
				Console_Print("arr[%.0f][%.0f] error: \"%s\".", y, x, e.what());
			throw e;
		}
		if ( is2d && x == 0 ) {
			ArrayID rid = g_ArrayMap.CreateArray(arr->m_owningModIndex);
			copy.arr->Get(y, true)->SetArray(rid, arr->m_owningModIndex);
			copy.rows.push_back(g_ArrayMap.Get(rid));
		}
		return v;
	}

	Matrix copy() {
		if ( !isMat )
			throw std::exception("Not matrix");
		Matrix copy = Matrix();
		double v = copyEditStart(copy);
		while ( y < h && x < w ) {
			try {
				v = copyEditContinue(v, copy);
			}
			catch (...) {
				break;
			}
		}
		return copy;
	}

	double magnitude() {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( !isVector() )
			throw std::exception("Not vector");

		double mag = pow(getFirstElement(),2);
		while ( x < w && y < h ) {
			try {
				mag += pow(getNextElement(),2);
			}
			catch (std::exception& e) {
				if ( std::string(e.what()).compare("Done") != 0 ) {
					Console_Print("arr[%.0f][%.0f] error: \"%s\".", y, x, e.what());
					throw e;
				}
				break;
			}
		}
		return sqrt(mag);
	}

	// returns the dot (scalar) product of two vectors
	double dot(Matrix factor) {
		if ( !isMat || !(factor.isMat) )
			throw std::exception("Not matrix");
		if ( !isVector() || !(factor.isVector()) )
			throw std::exception("Dot product defined only for vectors");
		if ( !(this->dimEqual(factor)) )
			throw std::exception("Matrix dimensions incompatible");

		double v = getFirstElement()*factor.getFirstElement();
		while ( x < w && y < h ) {
			try {
				v += getNextElement()*factor.getNextElement();
			}
			catch (std::exception& e) {
				if ( std::string(e.what()).compare("Done") != 0 ) {
					Console_Print("arr[%.0f][%.0f] error: \"%s\".", y, x, e.what());
					throw e;
				}
				break;
			}
		}
		return v;
	}

	// returns the cross (vector) product of two vectors
	Matrix cross(Matrix factor) {
		if ( !isMat || !(factor.isMat) )
			throw std::exception("Not matrix");
		if ( !isVector() || !(factor.isVector()) )
			throw std::exception("Cross product defined only for vectors");
		if ( lenVector() != 3 || factor.lenVector() != 3 )
			throw std::exception("Cross product defined only in exactly 3 dimensions");
		Matrix cross = Matrix(arr->m_owningModIndex, 3);
		double a;
		double b;
		double c;
		double d;

		arr->Get((double)1, false)->GetAsNumber(&a);
		factor.arr->Get((double)2, false)->GetAsNumber(&b);
		arr->Get((double)2, false)->GetAsNumber(&c);
		factor.arr->Get((double)1, false)->GetAsNumber(&d);
		cross.arr->Get((double)0, false)->SetNumber(a*b-c*d);

		arr->Get((double)2, false)->GetAsNumber(&a);
		factor.arr->Get((double)0, false)->GetAsNumber(&b);
		arr->Get((double)0, false)->GetAsNumber(&c);
		factor.arr->Get((double)2, false)->GetAsNumber(&d);
		cross.arr->Get((double)1, false)->SetNumber(a*b-c*d);

		arr->Get((double)0, false)->GetAsNumber(&a);
		factor.arr->Get((double)1, false)->GetAsNumber(&b);
		arr->Get((double)1, false)->GetAsNumber(&c);
		factor.arr->Get((double)0, false)->GetAsNumber(&d);
		cross.arr->Get((double)2, false)->SetNumber(a*b-c*d);

		return cross;
	}

	// turns a 1d array into a 2d array with a single row.
	Matrix rowVector() {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( is2d )
			throw std::exception("Not vector");

		ArrayID vecID = g_ArrayMap.CreateArray(arr->m_owningModIndex);
		ArrayVar* vec = g_ArrayMap.Get(vecID);

		ArrayID rid = g_ArrayMap.CreateArray(arr->m_owningModIndex);
		vec->Get((double)0, true)->SetArray(rid, arr->m_owningModIndex);
		ArrayVar* row = g_ArrayMap.Get(rid);

		double v;
		for ( x = 0; x < w; ++x ) {
			arr->Get(x, false)->GetAsNumber(&v);
			row->Get(x, true)->SetNumber(v);
		}
		return Matrix(vecID, w, 1);
	}

	// turns a 1d array into a 2d array with a single column.
	Matrix colVector() {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( is2d )
			throw std::exception("Not vector");

		ArrayID vecID = g_ArrayMap.CreateArray(arr->m_owningModIndex);
		ArrayVar* vec = g_ArrayMap.Get(vecID);

		double v;
		for ( y = 0; y < h; ++y ) {
			ArrayID rid = g_ArrayMap.CreateArray(arr->m_owningModIndex);
			vec->Get(y, true)->SetArray(rid, arr->m_owningModIndex);
			ArrayVar* row = g_ArrayMap.Get(rid);

			arr->Get(y, false)->GetAsNumber(&v);
			row->Get((double)0, true)->SetNumber(v);
		}
		return Matrix(vecID, 1, h);
	}

	// returns the transpose of a matrix.
	Matrix transpose() {
		if ( !isMat )
			throw std::exception("Not matrix");
		ArrayID transID = g_ArrayMap.CreateArray(arr->m_owningModIndex);
		ArrayVar* transpose = g_ArrayMap.Get(transID);
		if ( !is2d )
			return *this;
		else {
			ArrayID rid;
			ArrayVar* row = NULL;
			double v = getFirstElement();
			while ( x < w && y < h ) {
				if ( y == 0 ) {
					rid = g_ArrayMap.CreateArray(arr->m_owningModIndex);
					transpose->Get(x, true)->SetArray(rid, arr->m_owningModIndex);
				}
				else
					transpose->Get(x, false)->GetAsArray(&rid);
				row = g_ArrayMap.Get(rid);
				row->Get(y, true)->SetNumber(v);
				try {
					v = getNextElement();
				}
				catch (std::exception& e) {
					if ( std::string(e.what()).compare("Done") != 0 ) {
						Console_Print("arr[%.0f][%.0f] error: \"%s\".", y, x, e.what());
						throw e;
					}
					break;
				}
			}
			return Matrix(transID, w, h);
		}
	}

	// returns the sum along the diagonal of a square matrix
	double trace() {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( h != w )
			throw std::exception("Not square");
		if ( !is2d ) {
			if ( h == 1 ) {
				double v;
				arr->Get((double)0, false)->GetAsNumber(&v);
				return v;
			}
			else
				throw std::exception("Not square");
		}
		else {
			double tr = 0;
			double v;
			for ( y = 0; y < h; ++y ) {
				rows[y]->Get(y, false)->GetAsNumber(&v);
				tr += v;
			}
			return tr;
		}
	}

	// returns the determinant of a square matrix
	double determinant() {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( h != w )
			throw std::exception("Not square");

		// determinant of a 1x1 matrix is just whatever the number is
		if ( h == 1 ) {
			double v;
			rows[0]->Get((double)0, false)->GetAsNumber(&v);
			return v;
		}
		else if ( !is2d )
			throw std::exception("Not square");

		// easy formula for 2x2 matrix
		else if ( h == 2 ) {
			double a;	double b;
			double c;	double d;
			rows[0]->Get((double)0, false)->GetAsNumber(&a);
			rows[0]->Get((double)1, false)->GetAsNumber(&b);
			rows[1]->Get((double)0, false)->GetAsNumber(&c);
			rows[1]->Get((double)1, false)->GetAsNumber(&d);
			return a*d - b*c;
		}

		// similar formula for 3x3 matrices; Rule of Sarrus.
		else if ( h == 3 ) {
			double a;	double b;	double c;
			double d;	double e;	double f;
			double g;	double h;	double i;
			rows[0]->Get((double)0, false)->GetAsNumber(&a);
			rows[0]->Get((double)1, false)->GetAsNumber(&b);
			rows[0]->Get((double)2, false)->GetAsNumber(&c);
			rows[1]->Get((double)0, false)->GetAsNumber(&d);
			rows[1]->Get((double)1, false)->GetAsNumber(&e);
			rows[1]->Get((double)2, false)->GetAsNumber(&f);
			rows[2]->Get((double)0, false)->GetAsNumber(&g);
			rows[2]->Get((double)1, false)->GetAsNumber(&h);
			rows[2]->Get((double)2, false)->GetAsNumber(&i);
			return a*e*i + b*f*g + c*d*h - a*f*h - b*d*i - c*e*g;
		}

		// higher dimensions more complicated; using LU decomposition.
		// very similar to Gaussian elimination (used in RREF below)
		// modified to not bother forcing each leading element to be 1,
		// or to zero any elements above leading elements.
		// effectively creates an upper triangular matrix, U.
		// det(*this) = P*det(U);
		// P is 1 if an even number of row swaps occur;
		// P is -1 if an odd number of row swaps occur.
		else {
			Matrix U = copy();
			int P = 1;
			double det = 1;
			
			// Gaussian elimination algorithm from
			// http://en.wikipedia.org/wiki/Rref#Pseudocode
			// converted to use C++ and the Matrix class
			// "r" == U.y
			// "rowCount" == U.h
			// "columnCount" == U.w
			// v, a, and b used for intermediate values with GetAsNumber()
			double lead = 0;
			double i = 0;
			double v = 0;
			double a = 0;
			double b = 0;
			for ( U.y = 0; U.y < U.h; ++(U.y) ) {
				if ( lead >= U.w )
					break;
				i = U.y;
				U.rows[i]->Get(lead, false)->GetAsNumber(&v);
				while ( v == 0 ) { // while ( M[i, lead] == 0 )
					++i;
					if ( i == U.h ) {
						return 0;
					}
					U.rows[i]->Get(lead, false)->GetAsNumber(&v);
				}
				if ( i != U.y ) {
					U.swapRows(i, U.y);
					P *= -1;
				}
				det *= v;

				for ( i = lead+1; i < U.h; ++i ) {
					if ( i != U.y ) {
						// subtract M[i, lead] multiplied by row y from row i
						for ( U.x = lead; U.x < U.w; ++(U.x) ) {
							U.rows[U.y]->Get(U.x, false)->GetAsNumber(&a);
							U.rows[i]->Get(U.x, false)->GetAsNumber(&b);
							if ( U.x == lead )
								v = b/a;
							U.rows[i]->Get(U.x, false)->SetNumber(b-a*v);
						}
					}
				}
				++lead;
			}
			return P*det;
		}
	}

	// switches the positions of two rows in a matrix.
	void swapRows(double i, double j) {
		if ( i > h || j > h )
			throw std::exception("Index out of bounds");
		if ( i != j ) {
			ArrayVar* tmp = rows[j];
			rows[j] = rows[i];
			arr->Get(j, false)->SetArray(rows[i]->ID(), arr->m_owningModIndex);
			rows[i] = tmp;
			arr->Get(i, false)->SetArray(tmp->ID(), arr->m_owningModIndex);
		}
	}

	// returns the reduced row echelon form of a matrix
	// if a non-null Matrix pointer is passed, will simultaneously
	// find the inverse of the matrix and put it in the pointed Matrix.
	// if a non-null pointer is passed along with a non-invertible matrix,
	// the pointer will be nulled.
	Matrix rref(Matrix* inv = NULL) {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( !is2d )
			throw std::exception("RREF on 1d arrays ambiguous");
		Matrix rref = copy();

		if ( rref.isVector() ) {
			if ( inv ) {
				if ( rref.h == 1 && rref.w == 1 ) {
					*inv = Matrix();
					inv->isMat = true;
					inv->is2d = rref.is2d;
					inv->id = g_ArrayMap.CreateArray(rref.arr->m_owningModIndex);
					inv->arr = g_ArrayMap.Get(inv->id);
					inv->h = 1;
					inv->w = 1;
					if ( rref.is2d ) {
						ArrayID rid = g_ArrayMap.CreateArray(rref.arr->m_owningModIndex);
						inv->arr->Get((double)0, true)->SetArray(rid, rref.arr->m_owningModIndex);
						inv->rows.push_back(g_ArrayMap.Get(rid));
					}
					else
						inv->rows.push_back(inv->arr);
					double v;
					rref.rows[0]->Get((double)0, false)->GetAsNumber(&v);
					inv->rows[0]->Get((double)0, true)->SetNumber(1/v);
				}
				else
					inv = NULL;
			}
			double a = 0;
			double b = 0;
			if ( rref.h == 1 ) {
				for ( rref.x = 0; rref.x < rref.w; ++(rref.x) ) {
					if ( a == 0 )
						rref.rows[0]->Get(rref.x, false)->GetAsNumber(&a);
					if ( a == 0 )
						continue;
					rref.rows[0]->Get(rref.x, false)->GetAsNumber(&b);
					rref.rows[0]->Get(rref.x, false)->SetNumber(b/a);
				}
			}
			else {
				for ( rref.y = 0; rref.y < rref.h; ++(rref.y) ) {
					rref.rows[rref.y]->Get((double)0, false)->GetAsNumber(&a);
					if ( a != 0 ) {
						rref.rows[0]->Get((double)0, false)->SetNumber((double)1);
						rref.rows[rref.y]->Get((double)0, false)->SetNumber((double)0);
					}
				}
			}
			return rref;
		}
		
		if ( inv ) {
			if ( rref.h != rref.w )
				inv = NULL;
			else
				*inv = Matrix::genIdentityMatrix(rref.h, arr->m_owningModIndex);
		}

		// Gaussian elimination algorithm from
		// http://en.wikipedia.org/wiki/Rref#Pseudocode
		// converted to use C++ and the Matrix class
		// "r" == rref.y
		// "rowCount" == rref.h
		// "columnCount" == rref.w
		// v, a, and b used for intermediate values with GetAsNumber()
		double lead = 0;
		double i = 0;
		double v = 0;
		double a = 0;
		double b = 0;
		for ( rref.y = 0; rref.y < rref.h; ++(rref.y) ) {
			if ( lead >= rref.w )
				break;
			i = rref.y;
			rref.rows[i]->Get(lead, false)->GetAsNumber(&v);
			while ( v == 0 ) { // while ( M[i, lead] == 0 )
				++i;
				if ( i == rref.h ) {
					i = rref.y;
					++lead;
					if ( lead == rref.w ) {

						// checks to make sure matrix was invertible
						// if not, nulls.
						if ( inv ) {
							for ( rref.y = 0; rref.y < rref.h; ++(rref.y) ) {
								rref.rows[rref.y]->Get(rref.y, false)->GetAsNumber(&v);
								if ( v == 0 ) {
									inv = NULL;
									break;
								}
							}
						}

						return rref;
					}
				}
				rref.rows[i]->Get(lead, false)->GetAsNumber(&v);
			}
			if ( i != rref.y ) {
				rref.swapRows(i, rref.y);
				if ( inv )
					inv->swapRows(i, rref.y);
			}

			// divide row y by M[y, lead]
			for ( rref.x = (inv?0:lead); rref.x < rref.w; ++(rref.x) ) {
				rref.rows[rref.y]->Get(rref.x, false)->GetAsNumber(&a);
				rref.rows[rref.y]->Get(rref.x, false)->SetNumber(a/v);
				if ( inv ) {
					inv->rows[rref.y]->Get(rref.x, false)->GetAsNumber(&b);
					inv->rows[rref.y]->Get(rref.x, false)->SetNumber(b/v);
				}
			}

			for ( i = 0; i < rref.h; ++i ) {
				if ( i != rref.y ) {
					rref.rows[i]->Get(lead, false)->GetAsNumber(&v);

					// subtract M[i, lead] multiplied by row y from row i
					for ( rref.x = (inv?0:lead); rref.x < rref.w; ++(rref.x) ) {
						rref.rows[rref.y]->Get(rref.x, false)->GetAsNumber(&a);
						rref.rows[i]->Get(rref.x, false)->GetAsNumber(&b);
						rref.rows[i]->Get(rref.x, false)->SetNumber(b-a*v);
						if ( inv ) {
							inv->rows[rref.y]->Get(rref.x, false)->GetAsNumber(&a);
							inv->rows[i]->Get(rref.x, false)->GetAsNumber(&b);
							inv->rows[i]->Get(rref.x, false)->SetNumber(b-a*v);
						}
					}

				}
			}
			++lead;
		}

		// checks to make sure matrix was invertible
		// if not, nulls.
		if ( inv ) {
			for ( rref.y = 0; rref.y < rref.h; ++(rref.y) ) {
				rref.rows[rref.y]->Get(rref.y, false)->GetAsNumber(&v);
				if ( v == 0 ) {
					inv = NULL;
					break;
				}
			}
		}

		return rref;
	}

	// short-hand for inverting a Matrix using rref().
	Matrix invert() {
		Matrix inv;
		Matrix* invPtr = &inv;
		rref(invPtr);
		if ( invPtr )
			return inv;
		else
			throw std::exception("Matrix not invertible");
	}

	// scales a matrix
	Matrix operator*(double scalar) {
		if ( !isMat )
			throw std::exception("Not matrix");
		Matrix product = Matrix();
		double v = copyEditStart(product) * scalar;
		while ( x < w && y < h ) {
			try {
				v = copyEditContinue(v, product) * scalar;
			}
			catch (...) {
				break;
			}
		}
		return product;
	}

	// short-hand for scaling a matrix by a reciprocal.
	Matrix operator/(double scalar) {
		if ( !isMat )
			throw std::exception("Not matrix");
		return *this * (1/scalar);
	}

	// adds two matrices together
	Matrix operator+(Matrix addend) {
		if ( !isMat || !(addend.isMat) )
			throw std::exception("Not matrix");
		if ( !(this->dimEqual(addend)) )
			throw std::exception("Matrix dimensions incompatible");
		Matrix sum = Matrix();
		double v = copyEditStart(sum) + addend.getFirstElement();
		while ( x < w && y < h ) {
			try {
				v = copyEditContinue(v, sum) + addend.getNextElement();
			}
			catch (...) {
				break;
			}
		}
		return sum;
	}

	// subtracts one matrix from another
	Matrix operator-(Matrix subtrahend) {
		if ( !isMat || !(subtrahend.isMat) )
			throw std::exception("Not matrix");
		if ( !(this->dimEqual(subtrahend)) )
			throw std::exception("Matrix dimensions incompatible");
		Matrix difference = Matrix();
		double v = copyEditStart(difference) - subtrahend.getFirstElement();
		while ( x < w && y < h ) {
			try {
				v = copyEditContinue(v, difference) - subtrahend.getNextElement();
			}
			catch (...) {
				break;
			}
		}
		return difference;
	}

	// returns the matrix multiplication of two matrices
	Matrix operator*(Matrix factor) {
		if ( !isMat )
			throw std::exception("Not matrix");
		if ( !is2d && !(factor.is2d) )
			throw std::exception("Matrix dimensions ambiguous");

		double height;
		double width;
		double length;
		if ( w == factor.h ) {
			if ( is2d )
				height = h;
			else
				height = 1;
			if ( factor.is2d )
				width = factor.w;
			else
				width = 1;
			length = w;
		}
		else if ( ( !is2d && 1 == factor.h ) || ( !(factor.is2d) && w == 1 ) ) {
			height = h;
			width = factor.w;
			length = 1;
		}
		else
			throw std::exception("Matrix dimensions incompatible");

		ArrayID prodID = g_ArrayMap.CreateArray(arr->m_owningModIndex);
		ArrayVar* product = g_ArrayMap.Get(prodID);
		double a;
		double b;
		double v;

		for ( double i = 0; i < height; ++i ) {
			ArrayID rid = g_ArrayMap.CreateArray(arr->m_owningModIndex);
			product->Get(i, true)->SetArray(rid, arr->m_owningModIndex);
			ArrayVar* row = g_ArrayMap.Get(rid);
			for ( double j = 0; j < width; ++j ) {
				v = 0;
				for ( double k = 0; k < length; ++k ) {
					if ( is2d )
						rows[i]->Get(k, false)->GetAsNumber(&a);
					else if ( height == 1 )
						arr->Get(k, false)->GetAsNumber(&a);
					else
						arr->Get(i, false)->GetAsNumber(&a);

					if ( factor.is2d )
						factor.rows[k]->Get(j, false)->GetAsNumber(&b);
					else if ( width == 1 )
						factor.arr->Get(k, false)->GetAsNumber(&b);
					else
						factor.arr->Get(j, false)->GetAsNumber(&b);

					v += a*b;
				}
				row->Get(j, true)->SetNumber(v);
			}
		}
		return Matrix(prodID, height, width);
	}
};

bool Cmd_GenerateZeroMatrix_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 h;
	UInt32 w;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &h, &w))
		return true;

	Matrix mat = Matrix::genZeroMatrix(h, w, scriptObj->GetModIndex());
	*result = mat.id;

	return true;
}

bool Cmd_GenerateIdentityMatrix_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 dim;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &dim))
		return true;

	Matrix mat = Matrix::genIdentityMatrix(dim, scriptObj->GetModIndex());
	*result = mat.id;

	return true;
}

bool Cmd_GenerateRotationMatrix_Execute(COMMAND_ARGS)
{
	*result = 0;

	UInt32 axis;
	float theta;
	if(!ExtractArgs(PASS_EXTRACT_ARGS, &axis, &theta))
		return true;

	Matrix mat = Matrix::genRotationMatrix((char)axis, theta, scriptObj->GetModIndex());
	*result = mat.id;

	return true;
}

bool Cmd_VectorMagnitude_Execute(COMMAND_ARGS)
{
	*result = -1;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		try {
			Matrix mat (arrID);
			*result = mat.magnitude();
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_VectorNormalize_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		Matrix mat (arrID);
		try {
			double mag = mat.magnitude();
			Matrix norm = mat / mag;
			*result = norm.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_VectorDot_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if(eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		ArrayID a = eval.Arg(0)->GetArray();
		ArrayID b = eval.Arg(1)->GetArray();
		try {
			Matrix mata (a);
			Matrix matb (b);
			Matrix product = mata.dot(matb);
			*result = product.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_VectorCross_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if(eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		ArrayID a = eval.Arg(0)->GetArray();
		ArrayID b = eval.Arg(1)->GetArray();
		try {
			Matrix mata (a);
			Matrix matb (b);
			Matrix product = mata.cross(matb);
			*result = product.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_ForceRowVector_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		Matrix vec (arrID);
		try {
			Matrix row = vec.rowVector();
			*result = row.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_ForceColumnVector_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		Matrix vec (arrID);
		try {
			Matrix col = vec.colVector();
			*result = col.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixTrace_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		try {
			Matrix mat (arrID);
			*result = mat.trace();
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixDeterminant_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		try {
			Matrix mat (arrID);
			*result = mat.determinant();
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixRREF_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		try {
			Matrix mat (arrID);
			Matrix rref = mat.rref();
			*result = rref.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixInvert_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		try {
			Matrix mat (arrID);
			Matrix inv = mat.invert();
			*result = inv.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixTranspose_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array))
	{
		ArrayID arrID = eval.Arg(0)->GetArray();
		try {
			Matrix mat (arrID);
			Matrix trans = mat.transpose();
			*result = trans.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixScale_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if (eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Number) && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		double scalar = eval.Arg(0)->GetNumber();
		ArrayID arrID = eval.Arg(1)->GetArray();
		try {
			Matrix mat (arrID);
			Matrix prod = mat * scalar;
			*result = prod.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixAdd_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if(eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		ArrayID a = eval.Arg(0)->GetArray();
		ArrayID b = eval.Arg(1)->GetArray();
		try {
			Matrix mata (a);
			Matrix matb (b);
			Matrix sum = mata + matb;
			*result = sum.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixSubtract_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if(eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		ArrayID a = eval.Arg(0)->GetArray();
		ArrayID b = eval.Arg(1)->GetArray();
		try {
			Matrix mata (a);
			Matrix matb (b);
			Matrix difference = mata - matb;
			*result = difference.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

bool Cmd_MatrixMultiply_Execute(COMMAND_ARGS)
{
	*result = 0;

	ExpressionEvaluator eval(PASS_COMMAND_ARGS);
	if(eval.ExtractArgs() && eval.NumArgs() > 0 && eval.Arg(0)->CanConvertTo(kTokenType_Array) && eval.Arg(1)->CanConvertTo(kTokenType_Array))
	{
		ArrayID a = eval.Arg(0)->GetArray();
		ArrayID b = eval.Arg(1)->GetArray();
		try {
			Matrix mata (a);
			Matrix matb (b);
			Matrix product = mata * matb;
			*result = product.id;
		}
		catch (...) {
			*result = 0;
		}
	}
	return true;
}

#endif

CommandInfo kCommandInfo_SquareRoot =
{
	"SquareRoot",
	"sqrt",
	0,
	"Calculates the root of a value",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_SquareRoot_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Pow =
{
	"Pow",
	"pow",
	0,
	"Calculates one value raised to the power of another",
	0,
	2,
	kParams_TwoFloats,
	HANDLER(Cmd_Pow_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Log =
{
	"Log",
	"log",
	0,
	"Calculates the natural log of a value",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Log_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Exp =
{
	"Exp",
	"exp",
	0,
	"Calculates the exponential of a value",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Exp_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Log10 =
{
	"Log10",
	"log10",
	0,
	"Calculates the base 10 log of a value",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Log10_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Floor =
{
	"Floor",
	"flr",
	0,
	"Returns the nearest whole number that's less than a float",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Floor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Ceil =
{
	"Ceil",
	"ceil",
	0,
	"Returns the nearest whole number that's greater than a float",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Ceil_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Abs =
{
	"Abs",
	"abs",
	0,
	"Returns the absolute value of a float",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Abs_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Rand =
{
	"Rand",
	"r",
	0,
	"Returns a float between min and max",
	0,
	2,
	kParams_TwoFloats,
	HANDLER(Cmd_Rand_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};


CommandInfo kCommandInfo_Sin =
{
	"rSin",
	"rsin",
	0,
	"Calculates the sin of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Sin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Cos =
{
	"rCos",
	"rcos",
	0,
	"Calculates the cos of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Cos_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Tan =
{
	"rTan",
	"rtan",
	0,
	"Calculates the tan of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Tan_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
CommandInfo kCommandInfo_ASin =
{
	"rASin",
	"rasin",
	0,
	"Calculates the arcsin of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_ASin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ACos =
{
	"rACos",
	"racos",
	0,
	"Calculates the arccos of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_ACos_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ATan =
{
	"rATan",
	"ratan",
	0,
	"Calculates the arctan of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_ATan_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ATan2 =
{
	"rATan2",
	"ratan2",
	0,
	"Calculates the arctan of a value in radians",
	0,
	2,
	kParams_TwoFloats,
	HANDLER(Cmd_ATan2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Sinh =
{
	"rSinh",
	"rsinh",
	0,
	"Calculates the hyperbolic sin of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Sinh_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Cosh =
{
	"rCosh",
	"rcosh",
	0,
	"Calculates the hyperbolic cos of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Cosh_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_Tanh =
{
	"rTanh",
	"rtanh",
	0,
	"Calculates the hyperbolic tan of a value in radians",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_Tanh_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
CommandInfo kCommandInfo_dSin =
{
	"Sin",
	"dsin",
	0,
	"Calculates the sin of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dSin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dCos =
{
	"Cos",
	"dcos",
	0,
	"Calculates the cos of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dCos_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dTan =
{
	"Tan",
	"dtan",
	0,
	"Calculates the tan of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dTan_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
CommandInfo kCommandInfo_dASin =
{
	"ASin",
	"dasin",
	0,
	"Calculates the arcsin of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dASin_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dACos =
{
	"ACos",
	"dacos",
	0,
	"Calculates the arccos of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dACos_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dATan =
{
	"ATan",
	"datan",
	0,
	"Calculates the arctan of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dATan_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};
CommandInfo kCommandInfo_dATan2 =
{
	"ATan2",
	"datan2",
	0,
	"Calculates the arctan of a value in degrees",
	0,
	2,
	kParams_TwoFloats,
	HANDLER(Cmd_dATan2_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dSinh =
{
	"Sinh",
	"dsinh",
	0,
	"Calculates the hyperbolic sin of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dSinh_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dCosh =
{
	"Cosh",
	"dcosh",
	0,
	"Calculates the hyperbolic cos of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dCosh_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_dTanh =
{
	"Tanh",
	"dtanh",
	0,
	"Calculates the hyperbolic tan of a value in degrees",
	0,
	1,
	kParams_OneFloat,
	HANDLER(Cmd_dTanh_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_LeftShift =
{
	"LeftShift",
	"",
	0,
	"Shifts a 32-bit integer left the specified number of bits",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_LeftShift_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_RightShift =
{
	"RightShift",
	"",
	0,
	"Shifts an unsigned 32-bit integer right the specified number of bits",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_RightShift_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_LogicalAnd =
{
	"LogicalAnd",
	"",
	0,
	"Performs a logical AND between two 32-bit integers.",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_LogicalAnd_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_LogicalOr =
{
	"LogicalOr",
	"",
	0,
	"Performs a logical OR between two 32-bit integers.",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_LogicalOr_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_LogicalXor =
{
	"LogicalXor",
	"",
	0,
	"Performs a logical XOR between two 32-bit integers.",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_LogicalXor_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_LogicalNot =
{
	"LogicalNot",
	"",
	0,
	"Performs a logical NOT on a 32-bit integer.",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_LogicalNot_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kParams_Fmod[3] =
{
	{	"x",	kParamType_Float,	0 },
	{	"n",	kParamType_Float,	0 },
	{	"offset",	kParamType_Float,	1 },
};

CommandInfo kCommandInfo_Fmod =
{
	"fmod",
	"",
	0,
	"returns the result of a floating point modulous of the passed floats",
	0,
	3,
	kParams_Fmod,
	HANDLER(Cmd_Fmod_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GenerateZeroMatrix =
{
	"GenerateZeroMatrix",
	"ZeroMat",
	0,
	"creates an m x n zero matrix",
	0,
	2,
	kParams_TwoInts,
	HANDLER(Cmd_GenerateZeroMatrix_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GenerateIdentityMatrix =
{
	"GenerateIdentityMatrix",
	"IdentityMat",
	0,
	"creates an identity matrix",
	0,
	1,
	kParams_OneInt,
	HANDLER(Cmd_GenerateIdentityMatrix_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_GenerateRotationMatrix =
{
	"GenerateRotationMatrix",
	"RotMat",
	0,
	"creates a rotation matrix about the specified axis",
	0,
	2,
	kParams_OneAxis_OneFloat,
	HANDLER(Cmd_GenerateRotationMatrix_Execute),
	Cmd_Default_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_OneMatrix[1] =
{
	{	"matrix",	kOBSEParamType_Array,	0	},
};

CommandInfo kCommandInfo_VectorMagnitude =
{
	"VectorMagnitude",
	"VecMag",
	0,
	"returns the magnitude of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_VectorMagnitude_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_VectorNormalize =
{
	"VectorNormalize",
	"VecNorm",
	0,
	"returns the vector normalized",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_VectorNormalize_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_TwoMatrices[2] =
{
	{	"array",	kOBSEParamType_Array,	0	},
	{	"array",	kOBSEParamType_Array,	0	},
};

CommandInfo kCommandInfo_VectorDot =
{
	"VectorDot",
	"dot",
	0,
	"returns the dot product of the two matrices",
	0,
	2,
	kOBSEParams_TwoMatrices,
	HANDLER(Cmd_VectorDot_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_VectorCross =
{
	"VectorCross",
	"cross",
	0,
	"returns the cross product of the two matrices",
	0,
	2,
	kOBSEParams_TwoMatrices,
	HANDLER(Cmd_VectorCross_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ForceRowVector =
{
	"ForceRowVector",
	"RowVec",
	0,
	"forces a 1d array to a 2d array with 1 row",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_ForceRowVector_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_ForceColumnVector =
{
	"ForceColumnVector",
	"ColVec",
	0,
	"forces a 1d array to a 2d array with 1 column",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_ForceColumnVector_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixTrace =
{
	"MatrixTrace",
	"tr",
	0,
	"returns the trace of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixTrace_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixDeterminant =
{
	"MatrixDeterminant",
	"det",
	0,
	"returns the determinant of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixDeterminant_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixRREF =
{
	"MatrixRREF",
	"RREF",
	0,
	"returns the reduced-row-echelon-form of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixRREF_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixInvert =
{
	"MatrixInvert",
	"MatInv",
	0,
	"returns the matrix inversion of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixInvert_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixTranspose =
{
	"MatrixTranspose",
	"Transpose",
	0,
	"returns the matrix transposed",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixTranspose_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

static ParamInfo kOBSEParams_OneScalar_OneMatrix[2] =
{
	{	"scalar",	kOBSEParamType_Number,	0	},
	{	"matrix",	kOBSEParamType_Array,	0	},
};

CommandInfo kCommandInfo_MatrixScale =
{
	"MatrixScale",
	"MatScale",
	0,
	"returns the matrix scaled by the scalar",
	0,
	2,
	kOBSEParams_OneScalar_OneMatrix,
	HANDLER(Cmd_MatrixScale_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixAdd = 
{
	"MatrixAdd",
	"MatAdd",
	0,
	"returns the sum of two matrices",
	0,
	2,
	kOBSEParams_TwoMatrices,
	HANDLER(Cmd_MatrixAdd_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixSubtract = 
{
	"MatrixSubtract",
	"MatSubtract",
	0,
	"returns the difference of two matrices",
	0,
	2,
	kOBSEParams_TwoMatrices,
	HANDLER(Cmd_MatrixSubtract_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixMultiply =
{
	"MatrixMultiply",
	"MatMult",
	0,
	"returns the matrix multiplication of the two matrices",
	0,
	2,
	kOBSEParams_TwoMatrices,
	HANDLER(Cmd_MatrixMultiply_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};
/*
CommandInfo kCommandInfo_MatrixEigenvalues =
{
	"MatrixEigenvalues",
	"eigVl",
	0,
	"returns the eigenvalues of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixEigenvalues_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};

CommandInfo kCommandInfo_MatrixEigenvectors =
{
	"MatrixEigenvectors",
	"eigVc",
	0,
	"returns the eigenvectors of the matrix",
	0,
	1,
	kOBSEParams_OneMatrix,
	HANDLER(Cmd_MatrixEigenvectors_Execute),
	Cmd_Expression_Parse,
	NULL,
	0
};*/
