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

Type checkBreak(int& bcount);
Type checkReturnType(const Type& left, Symbol& func);
Type checkIfoWhile(const Type& left);
Type checkAddr(const Type& left, bool& lvalue);
Type checkAssignment(const Type& left, const Type& right, bool& left_lvalue);
Type checkIndex(const Type& left, const Type& right);
Type checkIncDec(bool& lvalue); 
Type checkDivMul(const Type& left, const Type& right, const std::string &op);
Type checkMod(const Type& left, const Type& right);
Type checkAdd(const Type& left, const Type& right);
Type checkSub(const Type& left, const Type& right);
Type checkEQs(const Type& left, const Type& right, const std::string &op);
Type checkLogical(const Type& left, const Type& right, const std::string &op);
Type checkNot(const Type& left);
Type checkNEG(const Type& left);
Type checkDeref(const Type& left);
Type checkSizeOf(const Type& left);
Type checkTypeCast(const Type& left, int typespec, unsigned indirection);
Type checkFuncType(const Symbol& sym, Parameters* arguments);
Type checkIDType(const Type& left, bool& lvalue);

# endif /* CHECKER_H */
