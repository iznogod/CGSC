#include "cgsc_param.h"
#include "cgsc_variable.h"
#include "../scr_vm_functions.h"

void GScr_DTest(scr_entref_t entref)
{
	#ifdef _CGSC_3
	// void (*iprintlnbold)(void) = (void (*)(void))0x80c2c14;
	const char *str = "TEST";
	Scr_CallFunction(iprintlnbold, STRING(&str));
	#endif

	#ifdef _CGSC_4
	const char *str = "TEST";
	Scr_CallFunction(iprintlnbold, STRING(&str));
	#endif
}

void Scr_CallFunction(void (*function)(void), ...)
{
	function();
	__callArgNumber = 0;
}

void Scr_CallMethod(void (*function)(scr_entref_t), scr_entref_t ent, ...)
{
	function(ent);
	__callArgNumber = 0;
}

qboolean Scr_SetParamGeneric(unsigned int paramnum, void *var, int type)
{
	VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
	if (funcParam == NULL)
		return qfalse;
	else
	{
		switch (type)
		{
			case VAR_FLOAT:
				funcParam->type = VAR_FLOAT;
				funcParam->u.floatValue = *(float *)var;
				break;
			case VAR_INTEGER:
				funcParam->type = VAR_INTEGER;
				funcParam->u.intValue = *(int *)var;
				break;
			case VAR_ISTRING:
				funcParam->type = VAR_ISTRING;
				funcParam->u.stringValue = Scr_AllocString(*(const char **)var);
				break;
			case VAR_STRING:
				funcParam->type = VAR_STRING;
				funcParam->u.stringValue = Scr_AllocString(*(const char **)var);
				break;
			case VAR_VECTOR:
				#ifdef _CGSC_4
				funcParam->type = VAR_VECTOR;
				funcParam->u.vectorValue = Scr_AllocVector(*(const float **)var);
				#endif
				break;
			case VAR_POINTER:
				funcParam->type = VAR_POINTER;
				funcParam->u.pointerValue = *(int *)var;
				break;
			case VAR_FUNCTION:
				funcParam->type = VAR_FUNCTION;
				funcParam->u.codePosValue = *(const char **)var;
				break;
			case VAR_UNDEFINED:
			default:
				funcParam->type = VAR_UNDEFINED;
				funcParam->u.intValue = 0;
				break;
		}
		__callArgNumber++;
		return qtrue;
	}
}
