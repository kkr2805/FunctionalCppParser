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

ParserState<Expression> expression(ParserState<Expression> p);
ParserState<Expression> term(ParserState<Expression> p);
ParserState<Expression> expression(ParserState<void> p);
ParserState<Expression> term(ParserState<void> p);
ParserState<Expression> factor(ParserState<Expression> p);
ParserState<Expression> factor(ParserState<void> p);

//ParserState<Expression> factor(ParserState<Expression> p);

enum { DIGIT, ADD_OP, MUL_OP, PARENTHESS_OPEN, PARENTHESS_CLOSE, WHITE_SPACE };

int main(int argc, char** argv)
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

    //const char test[] = "100 * 200";
    //const char test[] = "(100 * 200) + 1";
    //const char test[] = "(10 * 20) + 100 / 10";
    //const char test[] = "((10 * 20) + 100) / 10";
    const char test[] = "((10 * 20) + 100) - 50 / 10";
    //const char test[] = "50 / 10";

    Parser p(test, sizeof(test), tokens);

    p.setSkipTokens(skipTokens);

    // EXPRESSION := DIGIT OP DIGIT 
    if(!p.parse(static_cast<function<bool(ParserState<void>)>>([&](ParserState<void> s)->bool{
        ParserState<Expression> result = expression(s);
        result.get(static_cast<function<void(Expression)>>([&](Expression exp){ 
                    cout << "[Parinsg Success] " << test << " = " << exp.evaluate() << endl; 
                    }));

        return !result.isError();
    })))
        cout << "Pasing Error!!" << endl;

    return 0;
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
//=================================================
ParserState<Expression> expression(ParserState<Expression> p)
{
    ParserState<Expression> result = term(p); 
    ParserState<Expression> result1 = result.on(ADD_OP, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression left)->Expression{
#ifdef _DEBUG
                cout << "OP: " << lexeme << endl;
#endif
                return Expression(left.evaluate(), lexeme.c_str()[0], 0);
                }));

    if( !result1.isError() )
    {
        ParserState<Expression> result2 = term(result1);

        return result2.bind(static_cast<function<ParserState<Expression>(Expression)>>([&](Expression right)->ParserState<Expression>{ 
                    return result1.get(static_cast<function<Expression(Expression)>>([&](Expression left)->Expression{
                                left.right(right.evaluate());
                                return left;
                                }));
                    }));
    }

    return result;
}

ParserState<Expression> term(ParserState<Expression> p)
{
    ParserState<Expression> result = factor(p);
    ParserState<Expression> result1 = result.on(MUL_OP, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression left)->Expression{
#ifdef _DEBUG
                cout << "OP: " << lexeme << endl;
#endif
                return Expression(left.evaluate(), lexeme.c_str()[0], 1);
                }));

    if( !result1.isError() )
    {
        ParserState<Expression> result2 = term(result1);

        return result2.bind(static_cast<function<ParserState<Expression>(Expression)>>([&](Expression right)->ParserState<Expression>{ 
                    return result1.get(static_cast<function<Expression(Expression)>>([&](Expression left)->Expression{
                                left.right(right.evaluate());
                                return left;
                                }));
                    }));
    }

    return result;
}

ParserState<Expression> expression(ParserState<void> p)
{
    ParserState<Expression> result = term(p); 
    ParserState<Expression> result1 = result.on(ADD_OP, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression left)->Expression{
#ifdef _DEBUG
                cout << "OP: " << lexeme << endl;
#endif
                return Expression(left.evaluate(), lexeme.c_str()[0], 0);
                }));

    if( !result1.isError() )
    {
        ParserState<Expression> result2 = term(result1);

        return result2.bind(static_cast<function<ParserState<Expression>(Expression)>>([&](Expression right)->ParserState<Expression>{ 
                    return result1.get(static_cast<function<Expression(Expression)>>([&](Expression left)->Expression{
                                left.right(right.evaluate());
                                return left;
                                }));
                    }));
    }

    return result;
}

ParserState<Expression> term(ParserState<void> p)
{
    ParserState<Expression> result = factor(p);
    ParserState<Expression> result1 = result.on(MUL_OP, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression left)->Expression{
#ifdef _DEBUG
                cout << "OP: " << lexeme << endl;
#endif
                return Expression(left.evaluate(), lexeme.c_str()[0], 1);
                }));

    if( !result1.isError() )
    {
        ParserState<Expression> result2 = term(result1);

        return result2.bind(static_cast<function<ParserState<Expression>(Expression)>>([&](Expression right)->ParserState<Expression>{ 
                    return result1.get(static_cast<function<Expression(Expression)>>([&](Expression left)->Expression{
                                left.right(right.evaluate());
                                return left;
                                }));
                    }));
    }

    return result;

}

ParserState<Expression> factor(ParserState<Expression> p)
{
    ParserState<void> result = p.on(PARENTHESS_OPEN, static_cast<function<void(string, Expression)>>([&](string lexeme, Expression){
#ifdef _DEBUG
                cout << lexeme << endl; 
#endif
                return;
                }));

    if( !result.isError() )
    {
        ParserState<Expression> result1 = expression(result);
        ParserState<Expression> result2 = result1.on(PARENTHESS_CLOSE, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression e)->Expression{return e;}));

        return result2;
    }

    return p.on(DIGIT, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression e)->Expression{
#ifdef _DEBUG
                cout << "DIGIT: " << lexeme << endl;
#endif
                return Expression(atoi(lexeme.c_str()), ' ', 0);
                }));
}

ParserState<Expression> factor(ParserState<void> p)
{
    ParserState<void> result = p.on(PARENTHESS_OPEN, static_cast<function<void(string)>>([&](string lexeme){
#ifdef _DEBUG
                cout << lexeme << endl;
#endif
                return;
                }));

    if( !result.isError() )
    {
        ParserState<Expression> result1 = expression(result);
        ParserState<Expression> result2 = result1.on(PARENTHESS_CLOSE, static_cast<function<Expression(string, Expression)>>([&](string lexeme, Expression e){return e;}));

        return result2;
    }

    return p.on(DIGIT, static_cast<function<Expression(string)>>([&](string lexeme)->Expression{
#ifdef _DEBUG
                cout << "DIGIT: " << lexeme << endl;
#endif
                return Expression(atoi(lexeme.c_str()), ' ', 0);
                }));
}
