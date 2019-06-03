#ifndef _PARSER_H_
#define _PARSER_H_

#include <vector>
#include <string>

#include "Token.h"

using namespace std;

enum { NORMAL, ERROR } m_state; 

template<typename T>
class ParserState
{
    private:
        int m_state;
        char* m_buffer;
        int m_len;
        TOKENS m_skipTokens;
        TOKENS& m_tokens;
        const T m_data;
        
    public:
        ParserState(int state, char*buffer, int len, TOKENS& tokens, TOKENS skipTokens, T data);

        template<typename R> ParserState<R> on(int token, function<R (string, T)> fn);
        ParserState<void> on(int token, function<void (string, T)> fn);
        template<typename R> ParserState<R> get(function<R(T)> f);
        ParserState<void> get(function<void(T)> f);
        template<typename R> ParserState<R> bind(function<ParserState<R>(T)> f);
        ParserState<void> bind(function<ParserState<void>(T)> f);
        template<typename R> ParserState<T> result(ParserState<R> s);
        bool isError(){ return m_state == ERROR; }
        T getdata() { return m_data; }
};

template<>
class ParserState<void>
{
    private:
        int m_state;
        char* m_buffer;
        int m_len;
        TOKENS m_skipTokens;
        TOKENS& m_tokens;
        
    public:
        ParserState(int state, char*buffer, int len, TOKENS& tokens, TOKENS skipTokens);

        template<typename R> ParserState<R> on(int token, function<R (string)> fn);
        ParserState<void> on(int token, function<void (string)> fn);
        template<typename R> ParserState<R> get(function<R(void)> f);
        ParserState<void> get(function<void ()> f);
        template<typename R> ParserState<R> bind(function<ParserState<R>(void)> f);
        ParserState<void> bind(function<ParserState<void>(void)> f);
        template<typename R> ParserState<void> result(ParserState<R> s);
        bool isError(){ return m_state == ERROR; }
};

// =======================================
//
// Below codes implements ParserState<T>
//
// =======================================
template<typename T>
ParserState<T>::ParserState(int state, char*buffer, int len, TOKENS& tokens, TOKENS skipTokens, T data)
    : m_state(state), m_buffer(buffer), m_len(len), m_tokens(tokens), m_skipTokens(skipTokens), m_data(data)
{
}

template<typename T>
template<typename R>
ParserState<R> ParserState<T>::on(int token, function<R (string, T)> fn)
{
    if( m_state == ERROR ) 
        return ParserState<R>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens, R());
    
    Token* t = NULL;
    bool found = false;
    int advanced = 0;
    int skip = 0;
    int matched = 0;

    // skip chain of responsibility
    do
    {
        found = false;

        for(int i = 0; i < m_skipTokens.size(); i++)
        {
            if( (skip = m_skipTokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
            {
                found = true;
                advanced += skip;
            }
        }
    } while(found);

    // matching chain of reponsibility
    for(int i = 0; i < m_tokens.size(); i++)
    {
        if( (matched = m_tokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
        {
            found = true;
            advanced += matched;
            t = m_tokens[i]; 
            break;
        }
    }

    if( found == false || (t && t->getToken() != token))
        return ParserState<R>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens, R());

    R retValue = fn(t->lexeme(), m_data);

    return ParserState<R>(NORMAL, m_buffer + advanced, m_len - advanced, m_tokens, m_skipTokens, retValue);
}

template<typename T>
ParserState<void> ParserState<T>::on(int token, function<void (string, T)> fn)
{
    if( m_state == ERROR ) 
        return ParserState<void>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens);

    Token* t = NULL;
    bool found = false;
    int advanced = 0;
    int skip = 0;
    int matched = 0;

    // skip chain of responsibility
    do
    {
        found = false;

        for(int i = 0; i < m_skipTokens.size(); i++)
        {
            if( (skip = m_skipTokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
            {
                found = true;
                advanced += skip;
            }
        }
    } while(found);

    // matching chain of reponsibility
    for(int i = 0; i < m_tokens.size(); i++)
    {
        if( (matched = m_tokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
        {
            found = true;
            advanced += matched;
            t = m_tokens[i]; 
            break;
        }
    }

    if( found == false || (t && t->getToken() != token))
        return ParserState<void>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens); 

    fn(t->lexeme(), m_data);

    return ParserState<void>(NORMAL, m_buffer + advanced, m_len - advanced, m_tokens, m_skipTokens);
}

template<typename T>
template<typename R> 
ParserState<R> ParserState<T>::get(function<R(T)> f)
{
    return ParserState<R>(m_state, m_buffer, m_len, m_tokens, m_skipTokens, f(m_data));
}

template<typename T>
ParserState<void> ParserState<T>::get(function<void (T)> f)
{
    f(m_data);
    return ParserState<void>(m_state, m_buffer, m_len, m_tokens, m_skipTokens);
}

template<typename T>
template<typename R> 
ParserState<R> ParserState<T>::bind(function<ParserState<R>(T)> f)
{
    ParserState<R> ret = f(m_data);
    return ParserState<R>(m_state, m_buffer, m_len, m_tokens, m_skipTokens, ret.m_data);
}

template<typename T>
ParserState<void> ParserState<T>::bind(function<ParserState<void>(T)> f)
{
    f(m_data);
    return ParserState<void>(m_state, m_buffer, m_len, m_tokens, m_skipTokens);
}

template<typename T>
template<typename R> 
ParserState<T> ParserState<T>::result(ParserState<R> s)
{
    if( s.m_state == ERROR )
        return *this;

    return ParserState<T>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens);
}

// =======================================
//
// Below codes implements ParserState<void>
//
// =======================================
ParserState<void>::ParserState(int state, char*buffer, int len, TOKENS& tokens, TOKENS skipTokens)
    : m_state(state), m_buffer(buffer), m_len(len), m_tokens(tokens), m_skipTokens(skipTokens)
{
}

template<typename R>
ParserState<R> ParserState<void>::on(int token, function<R (string)> fn)
{
    if( m_state == ERROR ) 
        return ParserState<R>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens, R());
    
    Token* t = NULL;
    bool found = false;
    int advanced = 0;
    int skip = 0;
    int matched = 0;

    // skip chain of responsibility
    do
    {
        found = false;

        for(int i = 0; i < m_skipTokens.size(); i++)
        {
            if( (skip = m_skipTokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
            {
                found = true;
                advanced += skip;
            }
        }
    } while(found);

    // matching chain of reponsibility
    for(int i = 0; i < m_tokens.size(); i++)
    {
        if( (matched = m_tokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
        {
            found = true;
            advanced += matched;
            t = m_tokens[i]; 
            break;
        }
    }

    if( found == false || (t && t->getToken() != token))
        return ParserState<R>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens, R());

    R retValue = fn(t->lexeme());

    return ParserState<R>(NORMAL, m_buffer + advanced, m_len - advanced, m_tokens, m_skipTokens, retValue);
}

ParserState<void> ParserState<void>::on(int token, function<void (string)> fn)
{
    if( m_state == ERROR ) 
        return ParserState<void>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens);
    
    Token* t = NULL;
    bool found = false;
    int advanced = 0;
    int skip = 0;
    int matched = 0;

    // skip chain of responsibility
    do
    {
        found = false;

        for(int i = 0; i < m_skipTokens.size(); i++)
        {
            if( (skip = m_skipTokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
            {
                found = true;
                advanced += skip;
            }
        }
    } while(found);

    // matching chain of reponsibility
    for(int i = 0; i < m_tokens.size(); i++)
    {
        if( (matched = m_tokens[i]->match(m_buffer + advanced, m_len - advanced)) > 0 )
        {
            found = true;
            advanced += matched;
            t = m_tokens[i]; 
            break;
        }
    }

    if( found == false || (t && t->getToken() != token))
        return ParserState<void>(ERROR, m_buffer, m_len, m_tokens, m_skipTokens);

    fn(t->lexeme());

    return ParserState<void>(NORMAL, m_buffer + advanced, m_len - advanced, m_tokens, m_skipTokens);
}

template<typename R> 
ParserState<R> ParserState<void>::get(function<R(void)> f)
{
    return ParserState<R>(m_state, m_buffer, m_len, m_tokens, m_skipTokens, f());
}

ParserState<void> ParserState<void>::get(function<void ()> f)
{
    f();
    return ParserState<void>(m_state, m_buffer, m_len, m_tokens, m_skipTokens);
}

template<typename R> 
ParserState<R> ParserState<void>::bind(function<ParserState<R>(void)> f)
{
    ParserState<R> ret = f();
    return ParserState<R>(m_state, m_buffer, m_len, m_tokens, m_skipTokens, ret.m_data);
}

ParserState<void> ParserState<void>::bind(function<ParserState<void>(void)> f)
{
    f();
    return ParserState<void>(m_state, m_buffer, m_len, m_tokens, m_skipTokens);
}

class Parser
{
    private:
        vector<char> m_buffer;
        TOKENS m_skipTokens;
        TOKENS m_tokens;

    public:
        Parser(const char* str, int len, TOKENS& tokens);

        void setSkipTokens(TOKENS& tokens);
        bool parse(function<bool (ParserState<void>)>);

};

Parser::Parser(const char* str, int len, TOKENS& tokens)
    : m_tokens(tokens)
{
    m_buffer.resize(len);
    copy(str, str + len, m_buffer.begin());
}

void Parser::setSkipTokens(TOKENS& tokens)
{
    m_skipTokens = tokens;
}

bool Parser::parse(function<bool (ParserState<void>)> f)
{
   return f(ParserState<void>(NORMAL, &m_buffer[0], m_buffer.size(), m_tokens, m_skipTokens)); 
}

#endif
