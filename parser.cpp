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
int bcount = 0;

static void error()
{
    if (lookahead == DONE)
	report("syntax error at end of file");
    else
	report("syntax error at '%s'", yytext);

    exit(EXIT_FAILURE);
}

static int peek()
{
    if (nexttoken == 0) {
	nexttoken = yylex();
	nextbuf = yytext;
    }

    return nexttoken;
}

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

static unsigned integer()
{
    string buf;
    buf = lexbuf;
    match(INTEGER);
    return strtoul(buf.c_str(), NULL, 0);
}

static string identifier()
{
    string buf;
    buf = lexbuf;
    match(ID);
    return buf;
}

static void closeParamScope()
{
    Scope *scope;
    scope = closeScope();
    for (auto symbol : scope->symbols())
	delete symbol;
    delete scope;
}

static bool isSpecifier(int token)
{
    return token == CHAR || token == INT || token == DOUBLE;
}

static int specifier()
{
    int typespec = ERROR;
    if (isSpecifier(lookahead)) 
	{
		typespec = lookahead;
		match(lookahead);
    } 
	else
		error();

    return typespec;
}

static unsigned pointers()
{
    unsigned count = 0;
    while (lookahead == '*') 
	{
		match('*');
		count ++;
    }
    return count;
}

static void declarator(int typespec)
{
    unsigned indirection;
    string name;
    indirection = pointers();
    name = identifier();
    if (lookahead == '[') 
	{
		match('[');
		declareVariable(name, Type(typespec, indirection, integer()));
		match(']');
    } 
	else
		declareVariable(name, Type(typespec, indirection));
}

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

static void declarations()
{
    while (isSpecifier(lookahead))
		declaration();
}

// Definitely needs to be checked
static Type primaryExpression(bool& lvalue)
{
	cout << "primaryExpression() lvalue: " << lvalue << endl;
	Type left;
	Type right; 
	Symbol *sym;

    if (lookahead == '(') 
	{
		match('(');
		left = expression(lvalue);
		match(')');
	}
	else if (lookahead == CHARACTER) 
	{
		match(CHARACTER);
		left = Type(INT);
		lvalue = false;
    } 
	else if (lookahead == STRING) 
	{
		match(STRING);
		left = Type(CHAR, 0, lexbuf.length()-2);
		lvalue = false;
    } 
	else if (lookahead == INTEGER) 
	{
		match(INTEGER);
		left = Type(INT);
		lvalue = false;
    } 
	else if (lookahead == REAL) 
	{
		match(REAL);
		left = Type(DOUBLE);
		lvalue = false;
    } 
	else if (lookahead == ID) 
	{
		string name = identifier();
		bool tempLval;
		Parameters* args = new Parameters();
		sym = checkIdentifier(name);
		lvalue = sym->type().isScalar();
		left = checkIDType(sym->type(), lvalue);
		if (lookahead == '(') 
		{
			match('(');
			if (lookahead != ')') 
			{
				Type tempT;
				tempT = expression(tempLval);
				args->types.push_back(tempT); //should I implement explist outside?
				while (lookahead == ',') 
				{
					match(',');
					tempT = expression(tempLval);
					args->types.push_back(tempT);
				}
			}
			cout << sym << " " << args << endl;
			match(')');
			left = checkFuncType(*sym, args);
			lvalue = false;
		}
    } 
	else
		error();

	return left;
}

static Type postfixExpression(bool& lvalue)
{
    Type left = primaryExpression(lvalue);
    Type right;
	cout << "postfixExpression lvalue:" << lvalue << endl;

    while (1) 
	{
		if (lookahead == '[') 
		{
			match('[');
			right = expression(lvalue);
			left = checkIndex(left, right);
			cout << "Postfix brack in parser" << lvalue << endl;
			lvalue = true;
			match(']');
		} 
		else if (lookahead == INC) 
		{
			match(INC);
			cout << "Postfix Inc in parser" << lvalue << endl;
			checkIncDec(lvalue); 
			lvalue = false;
		} 
		else if (lookahead == DEC) 
		{
			match(DEC);
			cout << "Postfix Dec in parser" << lvalue << endl;
			checkIncDec(lvalue); 
			lvalue = false;
		} 
		else
			break;
    }
	return left;
}

static Type prefixExpression(bool& lvalue)
{
	cout << "enter prefix" << lvalue << endl;
	Type left;
	if (lookahead == '-') 
	{
		match('-');
		left = prefixExpression(lvalue);
		left = checkNEG(left);
		lvalue = false;
    } 
    else if (lookahead == '!') 
	{
		match('!');
		left = prefixExpression(lvalue);
		left = checkNot(left);
		lvalue = false;
    } 
	else if (lookahead == '&') 
	{
		match('&');
		cout << "prefix & before" << lvalue << endl;
		left = prefixExpression(lvalue);
		cout << "prefix & after" << lvalue << endl;
		left = checkAddr(left, lvalue);
		lvalue = false;
    } 
	else if (lookahead == '*') 
	{
		match('*');
		left = prefixExpression(lvalue);
		left = checkDeref(left);
		cout << "Prefix * in parser" << lvalue << endl;
		lvalue = true;
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
    } 
	else if (lookahead == '(' && isSpecifier(peek())) 
	{
		match('(');
		int typespec = specifier();
		unsigned indirection = pointers();
		match(')');
		left = prefixExpression(lvalue);
		left = checkTypeCast(left, typespec, indirection);
		lvalue = false;
    } 
	else
	{
		left = postfixExpression(lvalue);
		left = left.promote();
	}
	return left;
}

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
			left = checkDivMul(left, right, "*");
			lvalue = false;
		} 
		else if (lookahead == '/') 
		{
			match('/');
			right = prefixExpression(lvalue);
			left = checkDivMul(left, right, "/");
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
		if (lookahead == LEQ) 
		{
			match(LEQ);
			right = additiveExpression(lvalue);
			left = checkEQs(left, right, "<=");
			lvalue = false;
		} 
		else if (lookahead == GEQ) 
		{
			match(GEQ);
			right = additiveExpression(lvalue);
			left = checkEQs(left, right, ">=");
			lvalue = false;
		} 
		else if (lookahead == '<') 
		{
			match('<');
			right = additiveExpression(lvalue);
			left = checkEQs(left, right, "<");
			lvalue = false;
		} 
		else if (lookahead == '>') 
		{
			match('>');
			right = additiveExpression(lvalue);
			left = checkEQs(left, right, ">");
			lvalue = false;
		} 

		else
			break;
    }
	return left;
}

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
			left = checkEQs(left, right, "==");
			lvalue = false;
		} 
		else if (lookahead == NEQ) 
		{
			match(NEQ);
			right = relationalExpression(lvalue);
			left = checkEQs(left, right, "!=");
			lvalue = false;
		} 
		else
			break;
    }
	return left;
}

static Type logicalAndExpression(bool& lvalue)
{
    Type left = equalityExpression(lvalue);

    while (lookahead == AND) 
	{
		match(AND);
		Type right = equalityExpression(lvalue);
		left = checkLogical(left, right, "&&"); 
		lvalue = false;
    }

    return left;
}

static Type expression(bool& lvalue)
{
    Type left = logicalAndExpression(lvalue);

    while (lookahead == OR) 
	{
		match(OR);
		Type right = logicalAndExpression(lvalue);
		left = checkLogical(left, right, "||");
		lvalue = false;
    }
    return left;
}

static void statements(Symbol& func)
{
    while (lookahead != '}')
		statement(func);
}

static void assignment(bool& lvalue)
{
	cout << "assignment initial:" << lvalue << endl;
	Type right, left;
    left = expression(lvalue);
	bool lv_save = lvalue;
	cout << "assignment in parser" << lvalue << endl;

    if (lookahead == '=') 
	{
		match('=');
		right = expression(lvalue);
		cout << "assignment if stmt" << lvalue << endl;
		checkAssignment(left, right, lv_save);
    }
}

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
		checkBreak(bcount);
		match(';');

    } else if (lookahead == RETURN) 
	{
		match(RETURN);
		left = expression(lvalue);
		left = checkReturnType(left, func);
		match(';');

    } 
	else if (lookahead == WHILE) 
	{
		match(WHILE);
		match('(');
		left = expression(lvalue);
		left = checkIfoWhile(left);
		match(')');
		bcount++;
		statement(func);
		bcount--;

    } 
	else if (lookahead == FOR) 
	{
		match(FOR);
		match('(');
	 	assignment(lvalue);
		match(';');
		left = expression(lvalue);
		left = checkIfoWhile(left);
		match(';');
		assignment(lvalue);
		match(')');
		bcount++;
		statement(func);
		bcount--;

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
		assignment(lvalue);
		match(';');
    }
}

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

static Parameters *parameters()
{
    Parameters *params;
    openScope();
    params = new Parameters;
    params->variadic = false;

    if (lookahead == VOID)
	match(VOID);

    else 
	{
		params->types.push_back(parameter());

		while (lookahead == ',') 
		{
			match(',');

			if (lookahead == ELLIPSIS) 
			{
				params->variadic = true;
				match(ELLIPSIS);
				break;
			}

	    params->types.push_back(parameter());
		}
    }

    return params;
}

static void globalDeclarator(int typespec)
{
    unsigned indirection;
    string name;
    indirection = pointers();
    name = identifier();

    if (lookahead == '[') 
	{
		match('[');
		declareVariable(name, Type(typespec, indirection, integer()));
		match(']');
    } 
	else if (lookahead == '(') 
	{
		match('(');
		declareFunction(name, Type(typespec, indirection, parameters()));
		closeParamScope();
		match(')');
    } 
	else
		declareVariable(name, Type(typespec, indirection));
}

static void remainingDeclarators(int typespec)
{
    while (lookahead == ',') 
	{
		match(',');
		globalDeclarator(typespec);
    }
    match(';');
}

static void topLevelDeclaration()
{
	Symbol* func;
    int typespec;
    unsigned indirection;
    Parameters *params;
    string name;
    typespec = specifier();
    indirection = pointers();
    name = identifier();
    if (lookahead == '[') 
	{
		match('[');
		declareVariable(name, Type(typespec, indirection, integer()));
		match(']');
		remainingDeclarators(typespec);
    } 
	else if (lookahead == '(') 
	{
		match('(');
		params = parameters();
		match(')');
		if (lookahead == '{')
		{
			func = defineFunction(name, Type(typespec, indirection, params));
			match('{');
			declarations();
			statements(*func);
			closeScope();
			match('}');
		} 
		else 
		{
			closeParamScope();
			declareFunction(name, Type(typespec, indirection, params));
			remainingDeclarators(typespec);
		}
    } 
	else 
	{
		declareVariable(name, Type(typespec, indirection));
		remainingDeclarators(typespec);
    }
}

int main()
{
    openScope();
    lookahead = yylex();
    while (lookahead != DONE)
		topLevelDeclaration();

    closeScope();
    exit(EXIT_SUCCESS);
}
