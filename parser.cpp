/*
 * File:	parser.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the recursive-descent parser for
 *		Simple C.
 */

# include <cstdlib>
# include <iostream>
# include "checker.h"
# include "tokens.h"
# include "lexer.h"

using namespace std;

static Type expression(bool& lvalue);
static void statement(Symbol& function);

static int lookahead, nexttoken;
static string lexbuf, nextbuf;

/*
 * Function:	error
 *
 * Description:	Report a syntax error to standard error.
 */

static void error()
{
    if (lookahead == DONE)
	report("syntax error at end of file");
    else
	report("syntax error at '%s'", yytext);

    exit(EXIT_FAILURE);
}


/*
 * Function:	peek
 *
 * Description:	Return the next word from the lexer, but do not fetch it.
 */

static int peek()
{
    if (nexttoken == 0) {
	nexttoken = yylex();
	nextbuf = yytext;
    }

    return nexttoken;
}


/*
 * Function:	match
 *
 * Description:	Match the next token against the specified token.  A
 *		failure indicates a syntax error and will terminate the
 *		program since our parser does not do error recovery.
 */

static void match(int t)
{
    if (lookahead != t)
	error();

    if (nexttoken != 0) {
	lookahead = nexttoken;
	lexbuf = nextbuf;
	nexttoken = 0;
    } else {
	lookahead = yylex();
	lexbuf = yytext;
    }
}


/*
 * Function:	integer
 *
 * Description:	Match the next token as a number and return its value.
 */

static unsigned integer()
{
    string buf;


    buf = lexbuf;
    match(INTEGER);
    return strtoul(buf.c_str(), NULL, 0);
}


/*
 * Function:	identifier
 *
 * Description:	Match the next token as an identifier and return its name.
 */

static string identifier()
{
    string buf;


    buf = lexbuf;
    match(ID);
    return buf;
}


/*
 * Function:	closeParamScope
 *
 * Description:	Close the current scope, which should be the parameter
 *		scope of a function declaration.  Only the types, and not
 *		the symbols, need to saved as part of the declaration, so
 *		when the scope is closed, we can safely delete the scope
 *		and its symbols.
 */

static void closeParamScope()
{
    Scope *scope;


    scope = closeScope();

    for (auto symbol : scope->symbols())
	delete symbol;

    delete scope;
}


/*
 * Function:	isSpecifier
 *
 * Description:	Return whether the given token is a type specifier.
 */

static bool isSpecifier(int token)
{
    return token == CHAR || token == INT || token == DOUBLE;
}


/*
 * Function:	specifier
 *
 * Description:	Parse a type specifier.  Simple C has only char, int, and
 *		double types.
 *
 *		specifier:
 *		  char
 *		  int
 *		  double
 */

static int specifier()
{
    int typespec = ERROR;


    if (isSpecifier(lookahead)) {
		typespec = lookahead;
		match(lookahead);
    } else
		error();

    return typespec;
}


/*
 * Function:	pointers
 *
 * Description:	Parse pointer declarators (i.e., zero or more asterisks).
 *
 *		pointers:
 *		  empty
 *		  * pointers
 */

static unsigned pointers()
{
    unsigned count = 0;


    while (lookahead == '*') {
		match('*');
		count ++;
    }

    return count;
}


/*
 * Function:	declarator
 *
 * Description:	Parse a declarator, which in Simple C is either a scalar
 *		variable or an array, with optional pointer declarators.
 *
 *		declarator:
 *		  pointers identifier
 *		  pointers identifier [ integer ]
 */

static void declarator(int typespec)
{
    unsigned indirection;
    string name;


    indirection = pointers();
    name = identifier();

    if (lookahead == '[') {
		match('[');
		declareVariable(name, Type(typespec, indirection, integer()));
		match(']');
    } else
		declareVariable(name, Type(typespec, indirection));
}


/*
 * Function:	declaration
 *
 * Description:	Parse a local variable declaration.  Global declarations
 *		are handled separately since we need to detect a function
 *		as a special case.
 *
 *		declaration:
 *		  specifier declarator-list ;
 *
 *		declarator-list:
 *		  declarator
 *		  declarator , declarator-list
 */

static void declaration()
{
    int typespec;


    typespec = specifier();
    declarator(typespec);

    while (lookahead == ',') {
		match(',');
		declarator(typespec);
    }

    match(';');
}


/*
 * Function:	declarations
 *
 * Description:	Parse a possibly empty sequence of declarations.
 *
 *		declarations:
 *		  empty
 *		  declaration declarations
 */

static void declarations()
{
    while (isSpecifier(lookahead))
		declaration();
}


/*
 * Function:	primaryExpression
 *
 * Description:	Parse a primary expression.
 *
 *		primary-expression:
 *		  ( expression )
 *		  identifier ( expression-list )
 *		  identifier ( )
 *		  identifier
 *		  character
 *		  string
 *		  integer
 *		  real
 *
 *		expression-list:
 *		  expression
 *		  expression , expression-list
 */

static Type primaryExpression(bool& lvalue)
{
	Type left;
	Type right; 
	Symbol *sym;

    if (lookahead == '(') {
		match('(');
		expression();
		match(')');

    } else if (lookahead == CHARACTER) {
		match(CHARACTER);

    } else if (lookahead == STRING) {
	match(STRING);

    } else if (lookahead == INTEGER) {
	match(INTEGER);

    } else if (lookahead == REAL) {
	match(REAL);

    } else if (lookahead == ID) {
	checkIdentifier(identifier());

	if (lookahead == '(') {
	    match('(');

	    if (lookahead != ')') {
		expression();

		while (lookahead == ',') {
		    match(',');
		    expression();
		}
	    }

	    match(')');
	}

    } else
	error();
}


/*
 * Function:	postfixExpression
 *
 * Description:	Parse a postfix expression.
 *
 *		postfix-expression:
 *		  primary-expression
 *		  postfix-expression [ expression ]
 *		  postfix-expression ++
 *		  postfix-expression --
 */

static void postfixExpression()
{
    primaryExpression();

    while (1) {
	if (lookahead == '[') {
	    match('[');
	    expression();
	    match(']');
	    cout << "index" << endl;

	} else if (lookahead == INC) {
	    match(INC);
	    cout << "inc" << endl;

	} else if (lookahead == DEC) {
	    match(DEC);
	    cout << "dec" << endl;

	} else
	    break;
    }
}


/*
 * Function:	prefixExpression
 *
 * Description:	Parse a prefix expression.
 *
 *		prefix-expression:
 *		  postfix-expression
 *		  ! prefix-expression
 *		  - prefix-expression
 *		  * prefix-expression
 *		  & prefix-expression
 *		  sizeof prefix-expression
 *		  sizeof ( specifier pointers )
 *		  ( specifier pointers ) prefix-expression
 *
 *		This grammar is still ambiguous since "sizeof(type) * n"
 *		could be interpreted as a multiplicative expression or as a
 *		cast of a dereference.  The correct interpretation is the
 *		former, as the latter makes little sense semantically.  We
 *		resolve the ambiguity here by always consuming the "(type)"
 *		as part of the sizeof expression.
 */

static Type prefixExpression(bool& lvalue)
{
	Type left;
    if (lookahead == '!') 
	{
		match('!');
		left = prefixExpression(lvalue);
		left = checkNot(left);
		lvalue = false;
    } 
	else if (lookahead == '-') 
	{
		match('-');
		left = prefixExpression(lvalue);
		left = checkNEG(left);
		lvalue = false;
    } 
	else if (lookahead == '*') 
	{
		match('*');
		left = prefixExpression(lvalue);
		left = checkDeref(left);
		lvalue = true;
    } 
	else if (lookahead == '&') 
	{
		match('&');
		left = prefixExpression(lvalue);
		left = checkAddr(left, lvalue);
		lvalue = false;
    } 
	else if (lookahead == SIZEOF) 
	{
		match(SIZEOF);

		if (lookahead == '(' && isSpecifier(peek())) 
		{
			match('(');
			specifier();
			pointers();
			match(')');
		} 
		else 
		{
			left = prefixExpression(lvalue);
		}
		left = checkSizeOf(left);
		lvalue = false;

    } else if (lookahead == '(' && isSpecifier(peek())) {
	match('(');
	specifier();
	pointers();
	match(')');
	left = prefixExpression(lvalue);
	cout << "cast" << endl;

    } else
	postfixExpression();
}


/*
 * Function:	multiplicativeExpression
 *
 * Description:	Parse a multiplicative expression.
 *
 *		multiplicative-expression:
 *		  prefix-expression
 *		  multiplicative-expression * prefix-expression
 *		  multiplicative-expression / prefix-expression
 *		  multiplicative-expression % prefix-expression
 */

static Type multiplicativeExpression(bool& lvalue)
{
    Type left = prefixExpression(lvalue);
    Type right;

    while (1) 
	{
		if (lookahead == '*') 
		{
			match('*');
			right = prefixExpression(lvalue);
			left = checkMul(left, right);
			lvalue = false;
		} 
		else if (lookahead == '/') 
		{
			match('/');
			right = prefixExpression(lvalue);
			left = checkDiv(left, right);
			lvalue = false;
		} 
		else if (lookahead == '%') 
		{
			match('%');
			right = prefixExpression(lvalue);
			left = checkMod(left, right);
			lvalue = false;
		} 
		else
			break;
    }
	return left;
}

//Proof if possible
static Type additiveExpression(bool& lvalue)
{
   	Type left = multiplicativeExpression(lvalue);
   	Type right;

    while (1) 
	{
		if (lookahead == '+') 
		{
			match('+');
			right = multiplicativeExpression(lvalue);
			left = checkAdd(left, right);
			lvalue = false;
		} 
		else if (lookahead == '-') 
		{
			match('-');
			right = multiplicativeExpression(lvalue);
			left = checkSub(left, right);
			lvalue = false;
		} 
		else
			break;
    }
	return left;
}

static Type relationalExpression(bool& lvalue)
{
    Type left = additiveExpression(lvalue);
	Type right;

    while (1) 
	{
		if (lookahead == '<') {
			match('<');
			right = additiveExpression(lvalue);
			left = checkLTN(left, right);
			lvalue = false;
		} 
		else if (lookahead == '>') 
		{
			match('>');
			right = additiveExpression(lvalue);
			left = checkGTN(left, right);
			lvalue = false;
		} 
		else if (lookahead == LEQ) 
		{
			match(LEQ);
			right = additiveExpression(lvalue);
			left = checkLEQ(left, right);
			lvalue = false;
		} 
		else if (lookahead == GEQ) 
		{
			match(GEQ);
			right = additiveExpression(lvalue);
			left = checkGEQ(left, right);
			lvalue = false;
		} 
		else
			break;
    }
	return left;
}


/*
 * Function:	equalityExpression
 *
 * Description:	Parse an equality expression.
 *
 *		equality-expression:
 *		  relational-expression
 *		  equality-expression == relational-expression
 *		  equality-expression != relational-expression
 */

//Proof maybe..?
static Type equalityExpression(bool& lvalue)
{
    Type left = relationalExpression(lvalue);
	Type right;

    while (1) 
	{
		if (lookahead == EQL) 
		{
			match(EQL);
			right = relationalExpression(lvalue);
			left = checkEQL(left, right);
			lvalue = false;
		} 
		else if (lookahead == NEQ) 
		{
			match(NEQ);
			right = relationalExpression(lvalue);
			left = checkNEQ(left, right);
			lvalue = false;
		} else
			break;
    }
	return left;
}


/*
 * Function:	logicalAndExpression
 *
 * Description:	Parse a logical-and expression.  Note that Simple C does
 *		not have bitwise-and expressions.
 *
 *		logical-and-expression:
 *		  equality-expression
 *		  logical-and-expression && equality-expression
 */

//Proof this
static Type logicalAndExpression(bool& lvalue)
{
    Type left = equalityExpression(lvalue);

    while (lookahead == AND) {
		match(AND);
		Type right = equalityExpression(lvalue);

		left = checkLogical(left, right, '&&'); //Check this here
		lvalue = false;
    }

    return left;
}

/*
 * Function:	expression
 *
 * Description:	Parse an expression, or more specifically, a logical-or
 *		expression, since Simple C does not allow comma or
 *		assignment as an expression operator.
 *
 *		expression:
 *		  logical-and-expression
 *		  expression || logical-and-expression
 */

//Proof this
static Type expression(bool& lvalue)
{
    Type left = logicalAndExpression(lvalue);

    while (lookahead == OR) {
		match(OR);
		Type right = logicalAndExpression(lvalue);

		left = checkLogical(left, right, '||'); //Check this here
		lvalue = false;
    }

    return left;
}


/*
 * Function:	statements
 *
 * Description:	Parse a possibly empty sequence of statements.  Rather than
 *		checking if the next token starts a statement, we check if
 *		the next token ends the sequence, since a sequence of
 *		statements is always terminated by a closing brace.
 *
 *		statements:
 *		  empty
 *		  statement statements
 */

static void statements(Symbol& func)
{
    while (lookahead != '}')
		statement(func);
}


/*
 * Function:	Assignment
 *
 * Description:	Parse an assignment statement.
 *
 *		assignment:
 *		  expression = expression
 *		  expression
 */

static void assignment()
{
	//Check this maybe?
	bool lvalue, lv_save;
	Type right, left;

    left = expression(lvalue);
	lv_save = lvalue;

    if (lookahead == '=') 
	{
		match('=');
		right = expression(lvalue);
		left = checkAssignment(left, right, lv_save);
    }
}


/*
 * Function:	statement
 *
 * Description:	Parse a statement.  Note that Simple C has so few
 *		statements that we handle them all in this one function.
 *
 *		statement:
 *		  { declarations statements }
 *		  break ;
 *		  return expression ;
 *		  while ( expression ) statement
 *		  for ( assignment ; expression ; assignment ) statement
 *		  if ( expression ) statement
 *		  if ( expression ) statement else statement
 *		  assignment ;
 *
 *		This grammar still suffers from the "dangling-else"
 *		ambiguity.  We resolve it here by always consuming the
 *		"else" token, thus matching an else with the closest
 *		unmatched if.
 */

static void statement(Symbol& func)
{
	bool lvalue;
	Type right, left;

    if (lookahead == '{')
	{
		match('{');
		openScope();
		declarations();
		statements(func);
		closeScope();
		match('}');

    } else if (lookahead == BREAK) 
	{
		match(BREAK);
		left = checkBreak(left);
		match(';'); //Check this here

    } else if (lookahead == RETURN) 
	{
		match(RETURN);
		left = expression(lvalue);
		left = checkReturnType(left, func);
		match(';');

    } else if (lookahead == WHILE) 
	{
		match(WHILE);
		match('(');
		left = expression(lvalue);
		left =  checkIfoWhile(left);
		match(')');
		statement(func);

    } else if (lookahead == FOR) 
	{
		//Check here maybe
		match(FOR);
		match('(');
	 	assignment();
		match(';');
		left = expression(lvalue);
		left =  checkIfoWhile(left); //May need for check func
		match(';');
		assignment();
		match(')');
		statement(func);

    } else if (lookahead == IF) 
	{
		match(IF);
		match('(');
		left = expression(lvalue);
		left = checkIfoWhile(left);
		match(')');
		statement(func);

		if (lookahead == ELSE) 
		{
			match(ELSE);
			statement(func);
		}

    } else 
	{
		assignment();
		match(';');
    }
}


/*
 * Function:	parameter
 *
 * Description:	Parse a parameter, which in Simple C is always a scalar
 *		variable with optional pointer declarators.
 *
 *		parameter:
 *		  specifier pointers identifier
 */

static Type parameter()
{
    int typespec;
    unsigned indirection;
    string name;
    Type type;


    typespec = specifier();
    indirection = pointers();
    name = identifier();

    type = Type(typespec, indirection);
    declareVariable(name, type);
    return type;
}


/*
 * Function:	parameters
 *
 * Description:	Parse the parameters of a function, but not the opening or
 *		closing parentheses.
 *
 *		parameters:
 *		  void
 *		  parameter-list
 *		  parameter-list , ...
 */

static Parameters *parameters()
{
    Parameters *params;


    openScope();
    params = new Parameters;
    params->variadic = false;

    if (lookahead == VOID)
	match(VOID);

    else {
	params->types.push_back(parameter());

	while (lookahead == ',') {
	    match(',');

	    if (lookahead == ELLIPSIS) {
		params->variadic = true;
		match(ELLIPSIS);
		break;
	    }

	    params->types.push_back(parameter());
	}
    }

    return params;
}


/*
 * Function:	globalDeclarator
 *
 * Description:	Parse a declarator, which in Simple C is either a scalar
 *		variable, an array, or a function, with optional pointer
 *		declarators.
 *
 *		global-declarator:
 *		  pointers identifier
 *		  pointers identifier [ integer ]
 *		  pointers identifier ( parameters )
 */

static void globalDeclarator(int typespec)
{
    unsigned indirection;
    string name;


    indirection = pointers();
    name = identifier();

    if (lookahead == '[') {
	match('[');
	declareVariable(name, Type(typespec, indirection, integer()));
	match(']');

    } else if (lookahead == '(') {
	match('(');
	declareFunction(name, Type(typespec, indirection, parameters()));
	closeParamScope();
	match(')');

    } else
	declareVariable(name, Type(typespec, indirection));
}


/*
 * Function:	remainingDeclarators
 *
 * Description:	Parse any remaining global declarators after the first.
 *
 * 		remaining-declarators
 * 		  ;
 * 		  , global-declarator remaining-declarators
 */

static void remainingDeclarators(int typespec)
{
    while (lookahead == ',') {
	match(',');
	globalDeclarator(typespec);
    }

    match(';');
}


/*
 * Function:	topLevelDeclaration
 *
 * Description:	Parse a global declaration or function definition.
 *
 * 		global-or-function:
 * 		  specifier pointers identifier remaining-decls
 * 		  specifier pointers identifier [ integer ] remaining-decls
 * 		  specifier pointers identifier ( parameters ) remaining-decls 
 * 		  specifier pointers identifier ( parameters ) { ... }
 */

static void topLevelDeclaration()
{
    int typespec;
    unsigned indirection;
    Parameters *params;
    string name;


    typespec = specifier();
    indirection = pointers();
    name = identifier();

    if (lookahead == '[') {
	match('[');
	declareVariable(name, Type(typespec, indirection, integer()));
	match(']');
	remainingDeclarators(typespec);

    } else if (lookahead == '(') {
	match('(');
	params = parameters();
	match(')');

	if (lookahead == '{') {
	    defineFunction(name, Type(typespec, indirection, params));
	    match('{');
	    declarations();
	    statements();
	    closeScope();
	    match('}');

	} else {
	    closeParamScope();
	    declareFunction(name, Type(typespec, indirection, params));
	    remainingDeclarators(typespec);
	}

    } else {
	declareVariable(name, Type(typespec, indirection));
	remainingDeclarators(typespec);
    }
}


/*
 * Function:	main
 *
 * Description:	Analyze the standard input stream.
 */

int main()
{
    openScope();
    lookahead = yylex();

    while (lookahead != DONE)
	topLevelDeclaration();

    closeScope();
    exit(EXIT_SUCCESS);
}
