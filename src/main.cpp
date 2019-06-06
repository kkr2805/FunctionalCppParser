#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <algorithm>

#include "Parser.hpp"

//#define _DEBUG

using namespace std;

struct Expression
{
    private:
        int m_left;
        int m_right;
        char m_op;

    public:
        Expression();
        Expression(int left, char op, int right);
        int left();
        int right();
        void left(int left);
        void right(int right);
        char op();
        int evaluate();
};

bool Calculator(string strExpression, int &result);

ParserState<Expression> expression(ParserState<Expression> p);
ParserState<Expression> expression(ParserState<void> p);
ParserState<Expression> expression_(ParserState<Expression> p);
ParserState<Expression> expression_(ParserState<void> p);
ParserState<Expression> term(ParserState<Expression> p);
ParserState<Expression> term(ParserState<void> p);
ParserState<Expression> term_(ParserState<Expression> p);
ParserState<Expression> factor(ParserState<Expression> p);
ParserState<Expression> factor(ParserState<void> p);

enum EXPRESSION_TOKEN{ DIGIT, ADD_OP, MUL_OP, PARENTHESS_OPEN, PARENTHESS_CLOSE, WHITE_SPACE };

int main(int argc, char** argv)
{
    const char *test[] = {
        "100 * 200",
        "(100 * 200) + 1",
        "(10 * 20) + 100 / 10",
        "((10 * 20) + 100) / 10",
        "((10 * 20) + 100) - 50 / 10",
        "((10 * 20 * 3) + 100) - 50 / 10 + 100",
        "(100+200) * 300 / 3 - 200 + 500 + 3 * 100"
    };

    for( int k = 0; k < sizeof(test) / sizeof(char*); k++ )
    {
        int result = 0;

        if(Calculator(test[k], result))
            cout << "[Parsing Success] " << test[k] << " = " << result << endl;
        else
            cout << "[Pasing Error] " << test[k] << endl;
    }

    return 0;
}

//
// Calculator Function
bool Calculator(string strExpression, int &result)
{
    TOKENS tokens;
    tokens.push_back(new DigitToken(DIGIT));
    tokens.push_back(new Token(ADD_OP, "+"));
    tokens.push_back(new Token(ADD_OP, "-"));
    tokens.push_back(new Token(MUL_OP, "*"));
    tokens.push_back(new Token(MUL_OP, "/"));
    tokens.push_back(new Token(PARENTHESS_OPEN, "("));
    tokens.push_back(new Token(PARENTHESS_CLOSE, ")"));

    TOKENS skipTokens;
    skipTokens.push_back(new WhiteSpaceToken(WHITE_SPACE));

    Parser p(strExpression.c_str(), strExpression.length(), tokens);
    p.setSkipTokens(skipTokens);

    return p.parse(FUNCTION_CAST(
                [&](ParserState<void> s)->bool{
                    ParserState<Expression> r = expression(s);
                    r.get(FUNCTION_CAST([&](Expression exp){
                        result = exp.evaluate();             
                    }, void, Expression));
                }, bool, ParserState<void>));
}

//
// Below codes implement Expresstion Class
Expression::Expression()
    : m_left(0), m_right(0), m_op('+')
{
}

Expression::Expression(int left, char op, int right)
    : m_left(left), m_op(op), m_right(right)
{
}

int Expression::left()
{
    return m_left;
}

int Expression::right()
{
    return m_right;
}

void Expression::left(int left)
{
    m_left = left;
}

void Expression::right(int right)
{
    m_right = right;
}


char Expression::op()
{
    return m_op;
}

int Expression::evaluate()
{
    switch(m_op)
    {
        case '+':
            return m_left + m_right;
        case '-':
            return m_left - m_right;
        case '*':
            return m_left * m_right;
        case '/':
            return m_left / m_right;
    }

    return m_left;
}

//=================================================
//
// Below codes implement Non-terminal Procedures.
//
// EBNF For Calculator
// EXPRESSION   := TERM { ADD_OP TERM }
// ADD_OP       := + | -
// TERM         := FACTOR { MUL_OP FACTOR }
// MUL_OP       := * | /       
// FACTOR       := ( EXPRESSION ) | DIGIT
//
//=================================================
ParserState<Expression> expression(ParserState<Expression> p)
{
    ParserState<Expression> result = term(p); 
    ParserState<Expression> result1 = expression_(result);

    return result1;
}

ParserState<Expression> expression(ParserState<void> p)
{
    ParserState<Expression> result = term(p); 
    ParserState<Expression> result1 = expression_(result);

    return result1;
}

ParserState<Expression> expression_(ParserState<Expression> p)
{
    ParserState<Expression> result1 = p.on(ADD_OP, FUNCTION_CAST([&](string lexeme, Expression left)->Expression{
#ifdef _DEBUG
                cout << "OP: " << lexeme << endl;
#endif
                return Expression(left.evaluate(), lexeme.c_str()[0], 0);
                }, Expression, string, Expression));

    if( result1.isError() )
        return p;
    
    ParserState<Expression> result2 = term(result1);

    ParserState<Expression> result = result2.bind(FUNCTION_CAST([&](Expression right)->ParserState<Expression>{ 
                    return result1.get(FUNCTION_CAST([&](Expression left)->Expression{
                                left.right(right.evaluate());
                                return left;
                            }, Expression, Expression));
                    }, ParserState<Expression>, Expression));

    return expression_(result);
}

ParserState<Expression> term(ParserState<Expression> p)
{
    ParserState<Expression> result = factor(p);
    ParserState<Expression> result1= term_(result);

    return result1;
}

ParserState<Expression> term(ParserState<void> p)
{
    ParserState<Expression> result = factor(p);
    ParserState<Expression> result1= term_(result);

    return result1;
}

ParserState<Expression> term_(ParserState<Expression> p)
{
    ParserState<Expression> result1 = p.on(MUL_OP, FUNCTION_CAST([&](string lexeme, Expression left)->Expression{
#ifdef _DEBUG
                cout << "OP: " << lexeme << endl;
#endif
                return Expression(left.evaluate(), lexeme.c_str()[0], 1);
            }, Expression, string, Expression));

    if( result1.isError() )
        return p;

    ParserState<Expression> result2 = term(result1);

    ParserState<Expression> result = result2.bind(FUNCTION_CAST([&](Expression right)->ParserState<Expression>{ 
                return result1.get(FUNCTION_CAST([&](Expression left)->Expression{
                            left.right(right.evaluate());
                            return left;
                            }, Expression, Expression));
                }, ParserState<Expression>, Expression));

    return term_(result);
}

ParserState<Expression> factor(ParserState<Expression> p)
{
    ParserState<void> result = p.on(PARENTHESS_OPEN, FUNCTION_CAST([&](string lexeme, Expression){
#ifdef _DEBUG
                cout << lexeme << endl; 
#endif
                return;
            }, void, string, Expression));

    if( !result.isError() )
    {
        ParserState<Expression> result1 = expression(result);
        ParserState<Expression> result2 = result1.on(PARENTHESS_CLOSE, FUNCTION_CAST([&](string lexeme, Expression e){
                    return e;
                }, Expression, string, Expression));

        return result2;
    }

    return p.on(DIGIT, FUNCTION_CAST([&](string lexeme, Expression e){
#ifdef _DEBUG
                cout << "DIGIT: " << lexeme << endl;
#endif
                return Expression(atoi(lexeme.c_str()), ' ', 0);
            }, Expression, string, Expression));
}

ParserState<Expression> factor(ParserState<void> p)
{
    ParserState<void> result = p.on(PARENTHESS_OPEN, FUNCTION_CAST([&](string lexeme){
#ifdef _DEBUG
                cout << lexeme << endl;
#endif
                return;
            }, void, string));

    if( !result.isError() )
    {
        ParserState<Expression> result1 = expression(result);
        ParserState<Expression> result2 = result1.on(PARENTHESS_CLOSE, FUNCTION_CAST([&](string lexeme, Expression e){
                    return e;
                }, Expression, string, Expression));

        return result2;
    }

    return p.on(DIGIT, FUNCTION_CAST([&](string lexeme){
#ifdef _DEBUG
                cout << "DIGIT: " << lexeme << endl;
#endif
                return Expression(atoi(lexeme.c_str()), ' ', 0);
            }, Expression, string));
}
