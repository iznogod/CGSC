#include "scr_function.h"
#include <assert.h>
#include "../qcommon.h"
#include "../scr_vm.h"
#include "../cscr_variable.h"
#include "../cscr_stringlist.h"

extern struct scrVarGlob_t gScrVarGlob;
extern VariableValue Scr_GetArrayIndexValue(unsigned int name);
extern void IncInParam();
extern unsigned int Scr_AllocString(const char *s);
extern void Scr_AddIString(const char *value);

void Scr_FreeArray(VariableValue **array, int length)
{
    for (int i = 0; i < length; i++)
        free(array[i]);
    free(array);
}

VariableValue **Scr_GetArray(unsigned int paramnum)
{
    int parentId = Scr_GetObject(paramnum);

    assert(parentId != 0);
    assert(Scr_GetObjectType(parentId) == VAR_ARRAY);

    int length = GetArraySize(parentId);
    assert(length > 0);

    VariableValueInternal *entryValue;
    VariableValue **array = (VariableValue **)malloc(length * sizeof(VariableValue *));

    for (struct { int i; int id; } loop = {0, FindLastSibling(parentId)};
        loop.id;
        loop.id = FindPrevSibling(loop.id), loop.i++)
    {
        entryValue = &gScrVarGlob.variableList[loop.id + VARIABLELIST_CHILD_BEGIN];

        assert((entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_FREE 
            && (entryValue->w.status & VAR_STAT_MASK) != VAR_STAT_EXTERNAL);
        assert(!IsObject(entryValue));

        array[loop.i] = (VariableValue *)malloc(sizeof(VariableValue));
        array[loop.i]->type = entryValue->w.type & VAR_MASK;
        array[loop.i]->u = entryValue->u.u;
    }
    return array;
}

VariableValue *Scr_SelectParam(unsigned int paramnum)
{
    VariableValue *var;

    if (paramnum >= gScrVmPub.outparamcount)
    {
        Scr_Error(va("parameter %d does not exist", paramnum + 1));
        return NULL;
    }

    var = &gScrVmPub.top[-paramnum];
    return var;
}

VariableValue *Scr_SelectParamOrDefault(unsigned int paramnum)
{
    VariableValue *var;

    if (paramnum >= gScrVmPub.outparamcount) // alloc new param
    {
        gScrVmPub.top++;
        gScrVmPub.outparamcount++;
    }

    var = &gScrVmPub.top[-paramnum];
    return var;
}

qboolean Scr_SetParamFloat(unsigned int paramnum, float value)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_FLOAT;
        funcParam->u.floatValue = value;
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamInt(unsigned int paramnum, int value)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_INTEGER;
        funcParam->u.intValue = value;
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamObject(unsigned int paramnum, int structPointer)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_OBJECT;
        funcParam->u.pointerValue = structPointer;
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamEntity(unsigned int paramnum, int entID)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_ENTITY;
        funcParam->u.entityOffset = entID;
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamString(unsigned int paramnum, const char *string)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_STRING;
        funcParam->u.stringValue = Scr_AllocString(string);
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamIString(unsigned int paramnum, const char *string)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_ISTRING;
        funcParam->u.stringValue = Scr_AllocString(string);
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamFunc(unsigned int paramnum, const char *codePos)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_FUNCTION;
        funcParam->u.codePosValue = codePos;
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamStack(unsigned int paramnum, struct VariableStackBuffer *stack)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_STACK;
        funcParam->u.stackValue = stack;
        __callArgNumber++;
        return qtrue;
    }
}

qboolean Scr_SetParamVector(unsigned int paramnum, const float *value)
{
    VariableValue *funcParam = Scr_SelectParamOrDefault(paramnum);
    if (funcParam == NULL)
        return qfalse;
    else
    {
        funcParam->type = VAR_VECTOR;
        funcParam->u.vectorValue = value;
        __callArgNumber++;
        return qtrue;
    }
}

void Scr_AddFunc(const char *codePosValue)
{
    IncInParam();
    gScrVmPub.top->type = VAR_FUNCTION;
    gScrVmPub.top->u.codePosValue = codePosValue;
}

void Scr_AddVariable(VariableValue *var)
{
    Com_Printf(0, "entity type: %s\n", var_typename[var->type]);
    // Com_Printf(0, "entity value: %d\n", var->u.stringValue);

    switch (var->type)
    {
        case VAR_POINTER:
            Scr_AddObject(var->u.pointerValue);
            break;
        case VAR_FLOAT:
            Scr_AddFloat(var->u.floatValue);
            break;
        case VAR_INTEGER:
            Scr_AddInt(var->u.intValue);
            break;
        case VAR_STRING:
            Scr_AddString(SL_ConvertToString(var->u.stringValue));
            break;
        case VAR_ISTRING:
            Scr_AddIString(SL_ConvertToString(var->u.stringValue));
            break;
        case VAR_VECTOR:
            Scr_AddVector(var->u.vectorValue);
            break;
        case VAR_ENTITY:
            Scr_AddEntity(&g_entities[/*] 157 * [*/ var->u.entityOffset]);
            break;
        case VAR_UNDEFINED:
            Scr_AddUndefined();
            break;
        case VAR_FUNCTION:
            Scr_AddFunc(var->u.codePosValue);
            break;
        default:
            Scr_AddUndefined();
    }
}

void Scr_CallFunction(void (*function)(void), ...)
{
    function();
    __callArgNumber = 0;
}
