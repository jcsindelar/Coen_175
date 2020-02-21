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
    if (defined.count(name) > 0) {
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

    if (symbol == nullptr) {
	symbol = new Symbol(name, type);
	outermost->insert(symbol);

    } else if (type != symbol->type()) {
	report(conflicting, name);
	delete type.parameters();

    } else
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

    if (symbol == nullptr) {
	symbol = new Symbol(name, type);
	toplevel->insert(symbol);

    } else if (outermost != toplevel)
	report(redeclared, name);

    else if (type != symbol->type())
	report(conflicting, name);

    return symbol;
}


/*
 * Function:	checkIdentifier
 *
 * Description:	Check if NAME is declared.  If it is undeclared, then
 *		declare it as having the error type in order to eliminate
 *		future error messages.
 */

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr) {
	report(undeclared, name);
	symbol = new Symbol(name, error);
	toplevel->insert(symbol);
    }

    return symbol;
}

//Phase 4 functions
Type checkBreak(const Type& left){
    report(E1);

    //need to finish this one 
}

Type checkReturnType(const Type& left, Symbol& func)
{
    if (left == error)
        return error;
    
    Type returnType = Type(function.type().specifier(), function.type().indirection());

    if(left.isCompatible(returnType))
    {
        return left;
    }
    report(E2);
    return error;
}

Type checkIfoWhile(const Type& left){
    if(left == error){
        return error;
    }

    if(left.isPredicate()){
        return left;
    }
    report(E3);
    return error;
}

//Proof this
Type checkAddr(const Type& left, bool& lvalue){


    if(lvalue == true){
        return Type(left.specifier(), left.indirection()+1);
    }
    report(E4, "&");
    return error;
}

//Proof this
Type checkAssignment(const Type& left, const Type& right, bool& left_lvalue){

    if(left == error || right == error){
        return error;
    }
    
    if(left_lvalue != true){
        report(E4);
        return error;
    }
    if(left.isCompatible(right) == false){
        report(E5, "=");
        return error;
    }
    return left;
}


//Proof this
Type checkIndex(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }
    
    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isPointer() && rt.isInteger()){
        return Type(lt.specifier(), lt.indirection()-1);
    }
    report(E5, "[]");
    return error;
}

//Proof this
Type checkMul(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isNumeric() && right.isNumeric()){
        if(left.specifier() == DOUBLE || right.specifier() == DOUBLE){
            return real;
        }
        else{
            return integer;
        }
    }
    report(E5, "*");
    return error;
}

//Proof this
Type checkDiv(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isNumeric() && right.isNumeric()){
        if(left.specifier() == DOUBLE || right.specifier() == DOUBLE){
            return real;
        }
        else{
            return integer;
        }
    }
    report(E5, "/");
    return error;
}

//Proof this
Type checkMod(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }
    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isInteger() && rt.isInteger()){
        return integer;
    }
    report(E5, "%");
    return error;
}

//Proof this
Type checkAdd(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isNumeric() && rt.isNumeric()){
        
        if(lt.specifier() == DOUBLE || rt.specifier() == DOUBLE){
            return real;
        }
        else{
            return integer;
        }
       
    }
    else if(lt.isPointer() && rt.isInteger()){ 
        return lt;
    }
    else if(lt.isInteger() && rt.isPointer()){
        return rt;
    }

    report(E5, "+");
    return error;

}

//Proof this
Type checkSub(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    Type lt = left.promote();
    Type rt = right.promote();

    if(lt.isNumeric() and rt.isNumeric()){

        if(lt.specifier() == DOUBLE || rt.specifier() == DOUBLE){
            return real;
        }
        else{
            return integer;
        }
        
        
    }
    else if(lt.isPointer() && right.isInteger()){ 
        return lt;
    }
    else if(lt.isPointer() && rt.isPointer() && lt.specifier() == rt.specifier()){ 
        //do I have to check that pointer types are the same? last argument
        return integer;
    }

    report(E4, "-");
    return error;

}

//relationalExpression
//Proof this
Type checkLTN(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isCompatible(right)){
        return integer;
    }
    report(E5, "<");
    return error;

}

//Proof this
Type checkLEQ(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isCompatible(right)){
        return integer;
    }
    report(E5, "<=");
    return error;
}

//Proof this
Type checkGTN(const Type& left, const Type& right)
{
    if(left == error || right == error)
    {
        return error;
    }

    if(left.isCompatible(right))
    {
        return integer;
    }
    report(E5, ">");
    return error;
}

//Proof this
Type checkGEQ(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isCompatible(right)){
        return integer;
    }
    report(E5, ">=");
    return error;
}

//equalityExpression
//Proof this
Type checkEQL(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isCompatible(right)){
        return integer;
    }
    report(E5, "==");
    return error;

}
//Proof this
Type checkNEQ(const Type& left, const Type& right){
    if(left == error || right == error){
        return error;
    }

    if(left.isCompatible(right)){
        return integer;
    }
    report(E5, "!=");
    return error;
}

static Type checkLogical(const Type& left, const Type& right, const string &op)
{
    const Type &t1 = promote(left);
    const Type &t2 = promote(right);
    Type result = error;
    if(t1 != error && t2 != error)
    {
        if(t1.isPredicate() && t2.isPredicate())
        {
            result = integer;
        }
        else
        {
            report(E5, op)
        }
    }
    return result;
}

//Proof this
Type checkNot(const Type& left){
    
    
    if(left.isPredicate()){
        return integer;
    }
    report(E6, "!");
    return error;
}

//Proof this
Type checkNEG(const Type& left){

    
    if(left.isNumeric()){
        return left;
    }
    report(E6, "-");
    return error;
}

//Proof this
Type checkDeref(const Type& left){
    
    if(left.isPointer()){
        return Type(left.specifier(), left.indirection()-1);
    }
    report(E6, "*");
    return error;
}

//proof this too
Type checkSizeOf(const Type& left)
{
    if(left.isArray())
    {
        return (left.length());
    }
    report(E7);
    return error;
}

//Proof this
Type checkTypeCast(const Type& left, int typespec, unsigned indirection){

    Type result = Type(typespec, indirection);

    if(result.isNumeric() && left.isNumeric()){
        return result;
    }
    else if(result.isPointer() && left.isPointer()){
        return result;
    }
    else if(result.isPointer() && left.isInteger()){
        return result;
    }
    else if(left.isPointer() && result.isInteger()){
        return result;
    }
    report(E8);
    return error;
}

//Proof this
Type checkFuncType(const Symbol& sym, Parameters* arguments){
    if(sym.type().isFunction()){

        Parameters* params = sym.type().parameters();
        //Check if parameters exists
        if(params != nullptr){


            //if defined and parameters match, return type of function
            if(params->size() == arguments->size()){ 
                for(unsigned i = 0; i < params->size(); i++){
                    Type lt = (*arguments)[i].promote();
                    Type rt = (*params)[i].promote();

                    if( ( lt.isCompatible(rt) )==false){
                        report(E10);
                        return error;
                    }
                }
                //function returning T -> T (where T is defined by specifier and indirection)
                return( Type(sym.type().specifier(), sym.type().indirection() ) );
            }


        }
        else{
            //If function not defined, return implicit function return type
            return(Type(INT, 0, nullptr));            
        }
    }
    report(E9);
    return error;
}

