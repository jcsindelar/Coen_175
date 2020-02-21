/*
 * File:	checker.h
 *
 * Description:	This file contains the public function declarations for the
 *		semantic checker for Simple C.
 */

# ifndef CHECKER_H
# define CHECKER_H
# include "Scope.h"

Scope *openScope();
Scope *closeScope();

Symbol *defineFunction(const std::string &name, const Type &type);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);

Type checkBreak(const Type& left);
Type checkReturnType(const Type& left, Symbol& func);
Type checkIfoWhile(const Type& left);
Type checkAddr(const Type& left, bool& lvalue);
Type checkAssignment(const Type& left, const Type& right, bool& left_lvalue);
Type checkIndex(const Type& left, const Type& right);
Type checkMul(const Type& left, const Type& right);
Type checkDiv(const Type& left, const Type& right);
Type checkMod(const Type& left, const Type& right);
Type checkAdd(const Type& left, const Type& right);
Type checkSub(const Type& left, const Type& right);
Type checkLTN(const Type& left, const Type& right);
Type checkLEQ(const Type& left, const Type& right);
Type checkGTN(const Type& left, const Type& right);
Type checkGEQ(const Type& left, const Type& right);
Type checkEQL(const Type& left, const Type& right);
Type checkNEQ(const Type& left, const Type& right);
static Type checkLogical(const Type& left, const Type& right, const string &op);
Type checkNot(const Type& left);
Type checkNEG(const Type& left);
Type checkDeref(const Type& left);
Type checkSizeOf(const Type& left); //Fix this
Type checkTypeCast(const Type& left, int typespec, unsigned indirection);
Type checkFuncType(const Symbol& sym, Parameters* arguments);

# endif /* CHECKER_H */
