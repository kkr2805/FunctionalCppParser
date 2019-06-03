#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <string>

using namespace std;

class Token
{
    protected:
        int m_token;
        string m_lexeme;

    public:
        Token(int token, string lexeme);
        int getToken() { return m_token; };
        virtual int match(const char* str, int length);
        virtual string lexeme();
        virtual int token();

};

Token::Token(int token, string lexeme)
    : m_token(token), m_lexeme(lexeme)
{

}

int Token::match(const char* str, int length)
{
    int left = 0;
    int right = 0;

    while(left < m_lexeme.length() && right < length)
    {
        if( m_lexeme[left] != str[right] )
            return 0;

        left++;
        right++;
    }

    if( left != m_lexeme.length() )
        return 0;

    return left;
}

string Token::lexeme()
{
    return m_lexeme;
}

int Token::token()
{
    return m_token;
}

class WhiteSpaceToken : public Token
{
    private:


    public:
        WhiteSpaceToken(int token);
        virtual int match(const char* str, int length);
};

WhiteSpaceToken::WhiteSpaceToken(int token)
    : Token(token, "")
{
    
}

int WhiteSpaceToken::match(const char* str, int length)
{
    m_lexeme.clear();
    
    bool found = ( isspace(str[0]) ) ? true : false;
    int pos = 0;

    if(!found)
        return 0;
    else
        m_lexeme += str[pos++];
    

    while(pos < length)
    {
        if( !isspace(str[pos]) )
            break;

        m_lexeme += str[pos++];
    }
    
    return pos;

}

class DigitToken : public Token
{
    private:

    public:
        DigitToken(int token);
        int match(const char* str, int length);

};

DigitToken::DigitToken(int token)
    : Token(token, "")
{

}

int DigitToken::match(const char* str, int length)
{
    m_lexeme.clear();
    
    bool found = ( isdigit(str[0]) ) ? true : false;
    int pos = 0;

    if(!found)
        return 0;
    else
        m_lexeme += str[pos++];
    

    while(pos < length)
    {
        if( !isdigit(str[pos]) )
            break;

        m_lexeme += str[pos++];
    }
    
    return pos;

}
typedef vector<Token*> TOKENS;


#endif
