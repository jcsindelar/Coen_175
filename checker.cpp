/*
 * File:	checker.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the semantic checker for Simple C.
 *
 *		If a symbol is redeclared, the redeclaration is discarded
 *		and the original declaration is retained.
 *
 *		Extra functionality:
 *		- inserting an undeclared symbol with the error type
 */

# include <iostream>
# include <unordered_set>
# include "lexer.h"
# include "checker.h"
# include "tokens.h"
# include "Symbol.h"
# include "Scope.h"
# include "Type.h"


using namespace std;

#define E1 "break statement not within loop"
#define E2 "invalid return type"
#define E3 "invalid type for test expression"
#define E4 "lvalue required in expression"
#define E5 "invalid operands to binary %s"
#define E6 "invalid operand to unary %s"
#define E7 "invalid operand in sizeof expression"
#define E8 "invalid operand in cast expression"
#define E9 "called object is not a function"
#define E10 "invalid arguments to called function"

static unordered_set<string> defined;
static Scope *outermost, *toplevel;
static const Type error;
static Type integer(INT);
static Type real(DOUBLE);
static Type character(CHAR);

static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string undeclared = "'%s' undeclared";


/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope *openScope()
{
    toplevel = new Scope(toplevel);

    if (outermost == nullptr)
	    outermost = toplevel;

    return toplevel;
}


/*
 * Function:	closeScope
 *
 * Description:	Remove the top-level scope, and make its enclosing scope
 *		the new top-level scope.
 */

Scope *closeScope()
{
    Scope *old = toplevel;
    toplevel = toplevel->enclosing();
    return old;
}


/*
 * Function:	defineFunction
 *
 * Description:	Define a function with the specified NAME and TYPE.  A
 *		function is always defined in the outermost scope.
 */

Symbol *defineFunction(const string &name, const Type &type)
{
    if (defined.count(name) > 0) 
    {
        report(redefined, name);
        return outermost->find(name);
    }

    defined.insert(name);
    return declareFunction(name, type);
}


/*
 * Function:	declareFunction
 *
 * Description:	Declare a function with the specified NAME and TYPE.  A
 *		function is always declared in the outermost scope.  Any
 *		redeclaration is discarded.
 */

Symbol *declareFunction(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = outermost->find(name);

    if (symbol == nullptr) 
    {
        symbol = new Symbol(name, type);
        outermost->insert(symbol);
    } 
    else if (type != symbol->type()) 
    {
        report(conflicting, name);
        delete type.parameters();
    } 
    else
	    delete type.parameters();

    return symbol;
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol *declareVariable(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr) 
    {
        symbol = new Symbol(name, type);
        toplevel->insert(symbol);
    } 
    else if (outermost != toplevel)
	    report(redeclared, name);

    else if (type != symbol->type())
	    report(conflicting, name);

    return symbol;
}

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);
    if (symbol == nullptr) 
    {
        report(undeclared, name);
        symbol = new Symbol(name, error);
        toplevel->insert(symbol);
    }
    return symbol;
}

//Phase 4 functions
Type checkBreak(int& bcount)
{
    if(bcount <= 0)
    {
        report(E1);
        return error;
    }
    //bcount--; Do I need this here?
    return integer;
}

Type checkReturnType(const Type& left, Symbol& func)
{
    if (left == error)
        return error;
    
    Type returnType = Type(func.type().specifier(), func.type().indirection());

    if(left.isCompatibleWith(returnType))
    {
        return left;
    }
    report(E2);
    return error;
}

Type checkIfoWhile(const Type& left)
{
    if(left == error)
    {
        return error;
    }
    if(left.isPredicate())
    {
        return left;
    }
    report(E3);
    return error;
}

Type checkAddr(const Type& left, bool& lvalue)
{
    if(lvalue == true)
    {
        return Type(left.specifier(), left.indirection()+1);
    }
    cout << "checkAddr error" << lvalue << endl;
    report(E4);
    return error;
}

Type checkAssignment(const Type& left, const Type& right, bool& left_lvalue)
{
    if(left == error || right == error)
    {
        return error;
    }
    if(left_lvalue == false)
    {
        cout << "checkAssignment error" << left_lvalue << endl;
        report(E4);
        return error;
    }
    if(left.isCompatibleWith(right) == false)
    {
        cout << "checkAssignment = error" << left_lvalue << endl;
        report(E5, "=");
        return error;
    }
    return left;
}


Type checkIndex(const Type& left, const Type& right)
{
    if(left == error || right == error)
    {
        return error;
    }
    Type lt = left.promote();
    Type rt = right.promote();

    cout << lt.isPointer() << rt.isInteger() << rt.specifier() << "precheckind" << endl;
    if(lt.isPointer() && (rt == Type(INT)))
    {
        return Type(lt.specifier(), lt.indirection()-1);
    }
    cout << "checkIndex error" << endl;
    report(E5, "[]");
    return error;
}

Type checkIncDec(bool& lvalue)
{
    if(lvalue == true)
    {
        return lvalue;
    }
    cout << "checkIncDec" << lvalue << endl;
    report(E4);

    return error;
}

Type checkDivMul(const Type& left, const Type& right, const string& op)
{
    if(left == error || right == error)
    {
        return error;
    }
    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isNumeric() && rt.isNumeric())
    {
        if(left == Type(DOUBLE) || right == Type(DOUBLE))
        {
            return real;
        }
        else
        {
            return integer;
        }
    }
    cout << "check div mul error" << endl;
    report(E5, op);
    return error;
}

Type checkMod(const Type& left, const Type& right)
{
    if(left == error || right == error)
    {
        return error;
    }
    Type lt = left.promote();
    Type rt = right.promote();

    if(lt == Type(INT) && rt == Type(INT))
    {
        return integer;
    }
    report(E5, "%");
    return error;
}

//Check this
Type checkAdd(const Type& left, const Type& right)
{
    if(left == error || right == error)
    {
        return error;
    }

    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isNumeric() && rt.isNumeric())
    {
        if(lt.specifier() == DOUBLE || rt.specifier() == DOUBLE)
        {
            return real;
        }
        else
        {
            return integer;
        }
    }
    else if(lt.isPointer() && (rt == Type(INT)))
    { 
        return lt;
    }
    else if((lt == Type(INT)) && rt.isPointer())
    {
        return rt;
    }
    cout << "checkAdd error" << endl;
    report(E5, "+");
    return error;
}

//Check this
Type checkSub(const Type& left, const Type& right)
{
    if(left == error || right == error)
    {
        return error;
    }

    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isNumeric() and rt.isNumeric())
    {

        if(lt.specifier() == DOUBLE || rt.specifier() == DOUBLE)
        {
            return real;
        }
        else
        {
            return integer;
        }
    }
    else if(lt.isPointer() && (right == Type(INT)))
    { 
        return lt;
    }
    else if(lt.isPointer() && rt.isPointer() && lt.specifier() == rt.specifier())
    { 
        return integer;
    }
    cout << "checkSub Error" << endl;
    report(E5, "-");
    return error;
}

Type checkEQs(const Type& left, const Type& right, const string &op)
{
    if(left == error || right == error)
    {
        return error;
    }
    if(left.isCompatibleWith(right))
    {
        return integer;
    }
    cout << "checkEQs error" << endl;
    report(E5, op);
    return error;
}

Type checkLogical(const Type& left, const Type& right, const string &op)
{
    const Type &t1 = left.promote();
    const Type &t2 = right.promote();
    Type result = error;
    if(t1 != error && t2 != error)
    {
        if(t1.isPredicate() && t2.isPredicate())
        {
            result = integer;
        }
        else
        {
            cout << "Checklogical error" << endl;
            report(E5, op);
        }
    }
    return result;
}

Type checkNot(const Type& left)
{
    if(left.isPredicate())
    {
        return integer;
    }
    report(E6, "!");
    return error;
}

Type checkNEG(const Type& left)
{
    if(left.isNumeric())
    {
        return left;
    }
    report(E6, "-");
    return error;
}

Type checkDeref(const Type& left)
{
    if(left.isPointer())
    {
        return Type(left.specifier(), left.indirection()-1);
    }
    report(E6, "*");
    return error;
}

Type checkSizeOf(const Type& left)
{
    if(left.isFunction())
    {
        report(E7);
        return error;
    }
    return integer;
}

Type checkTypeCast(const Type& left, int typespec, unsigned indirection)
{
    Type result = Type(typespec, indirection);

    if(result.isNumeric() && left.isNumeric())
    {
        return result;
    }
    else if(result.isPointer() && left.isPointer())
    {
        return result;
    }
    else if(result.isPointer() && (left == Type(INT)))
    {
        return result;
    }
    else if((result == Type(INT)) && left.isPointer())
    {
        return result;
    }
    report(E8);
    return error;
}

//Still off
Type checkFuncType(const Symbol& sym, Parameters* arguments)
{
    cout << "CheckFuncType" << endl;
    if(sym.type().isFunction())
    {
        Parameters* params = sym.type().parameters();
        if(params->types.size() > arguments->types.size())
        { 
            cout << "E10" << " in first part" << endl;
            report(E10);
            return error;
        }
        else if (arguments->types.size() > params->types.size() && !params->variadic)
        {
            cout << "E10" << " in variadic part" << endl;
            report(E10);
            return error;
        }
        else
        {
            for(unsigned i = 0; i < params->types.size(); i++)
            {
                cout << "pretype" << endl;
                Type lt = ((arguments->types)[i].promote());
                Type rt = ((params->types)[i].promote());

                cout << lt << ' ' << rt << endl;
                if((rt.isCompatibleWith(lt))==false)
                {
                    cout << "E10" << endl;
                    report(E10);
                    return error;
                }
            }
            return( Type(sym.type().specifier(), sym.type().indirection() ) );
        }
    }
    report(E9);
    return error;
}

Type checkIDType(const Type& left, bool& lvalue)
{
    if(left == error)
    {
        return error;
    }
    
    if(left.isScalar())
    {
        cout << "CheckIDType" << lvalue << endl;
        lvalue = true;
        return left;
    }
    else
    {
        cout << "CheckIDType false" << lvalue << endl;
        lvalue = false;
    }
    return left;
}
