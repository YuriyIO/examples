#include <iostream>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

enum type_of_lex
{
    LEX_NULL,

    LEX_PROGRAM,
    LEX_INT,
    LEX_BOOL,
    LEX_STRING,
    LEX_STRING_VAL,
    LEX_READ,
    LEX_WRITE,
    LEX_IF,
    LEX_ELSE,
    LEX_DO,
    LEX_WHILE,
    LEX_TRUE,
    LEX_FALSE,
    LEX_AND,
    LEX_NOT,
    LEX_OR,

    LEX_PLUS,
    LEX_MINUS,
    LEX_SLASH,
    LEX_STAR,
    LEX_GRT,
    LEX_LSS,
    LEX_GOE,
    LEX_LOE,
    LEX_EQ,
    LEX_NEQ,
    LEX_ASSIGN,
    LEX_COMMA,
    LEX_SEMICOLON,
    LEX_LPAREN,
    LEX_RPAREN,
    LEX_STBLOCK,
    LEX_FNBLOCK,
    LEX_QOTE,

    LEX_NUM,
    LEX_ID, // variable

    POLIZ_LABEL,
    POLIZ_ADDRESS,
    POLIZ_GO,
    POLIZ_FGO
};

class Lex
{
    type_of_lex type;
    int IntValue = 0;
    std::string StrValue;
    bool BoolValue;

public:
    Lex(type_of_lex type = LEX_NULL, int value = 0) : type(type), IntValue(value) {}
    Lex(type_of_lex type, bool b_v) : type(type), BoolValue(b_v) {}
    Lex(type_of_lex type, std::string str = "a") : type(type), StrValue(str) {}

    type_of_lex get_type() const { return type; }
    bool get_BoolValue() const { return BoolValue; }
    int get_IntValue() const { return IntValue; }
    std::string get_StrValue() const { return StrValue; }
    friend std::ostream &operator<<(std::ostream &s, Lex l)
    {
        s << '(' << l.get_type() << ", " << l.get_IntValue() << ");";
        return s;
    }
};

class Variable
{
    char *name = nullptr;
    type_of_lex type;
    int IntValue;
    std::string StrValue;
    bool declaration = false;
    bool assign = false;

public:
    Variable() {}
    ~Variable()
    {
        if (name != nullptr)
            delete[] name;
    }
    char *get_name() const { return name; }
    void set_name(const char *n)
    {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }

    type_of_lex get_type() const { return type; }
    void set_type(type_of_lex t) { type = t; }

    int get_IntValue() const { return IntValue; }
    void set_IntValue(int t) { IntValue = t; }

    std::string get_StrValue() const { return StrValue; }
    void set_StrValue(std::string str) { StrValue = str; }

    bool get_declaration() const { return declaration; }
    void set_declaration() { declaration = true; }

    bool get_assign() const { return assign; }
    void set_assign() { assign = true; }
};

class VTable
{
    Variable *table = nullptr;
    int top;
    int size;

public:
    void set_table(int const size)
    {
        if (table == nullptr)
        {
            table = new Variable[size];
            top = 1;
        }
    }
    VTable(int const size) : size(size)
    {
        table = new Variable[size];
        top = 1;
    }
    ~VTable()
    {
        if (table != nullptr)
            delete[] table;
    }

    Variable &operator[](const int k) { return table[k]; }
    int get_size() const { return size; }
    int put(const char *buf)
    {
        for (int i = 1; i < top; i++)
            if (!strcmp(buf, table[i].get_name()))
                return i;
        table[top].set_name(buf);
        top++;
        return top - 1;
    }
} TID(128);

class Lexer
{
    FILE *fd;
    int c;
    char buf[128];
    int buf_top;
    bool not_eof = true;

    enum state
    {
        H,
        IDENT,
        NUMB,
        COM,
        QOUT,
        COMPR,
        NOT,
        DELIM,
        FIN_OF_FILE,
        SLASH
    };
    state G;

    static const char *TW[];
    static type_of_lex wtypes[];
    static const char *TS[];
    static type_of_lex stypes[];

    void clear()
    {
        buf_top = 0;
        for (int i = 0; i < 128; i++)
            buf[i] = 0;
    }

    void add() { buf[buf_top++] = c; };

    void get_symb() { c = fgetc(fd); }

    int define_type(const char *buf, const char **table)
    {
        int i = 0;
        while (table[i])
        {
            if (!strcmp(buf, table[i]))
                return i;
            ++i;
        }
        return 0;
    }

public:
    Lexer(const char *path)
    {
        if ((fd = fopen(path, "r")) == NULL)
        {
            std::cout << "Error: file didn't open" << std::endl;
            exit(1);
        }
        clear();
        get_symb();
    }
    ~Lexer() { fclose(fd); }

    Lex get_lex();
    bool check_eof() const { return not_eof; }
};

Lex Lexer::get_lex()
{
    int digit;
    int num;
    std::string str;

    G = H;
    while (true)
    {
        switch (G)
        {
        case H:
            if (c == '\n' || c == '\r' || c == ' ' || c == '\t')
                get_symb();
            else if(c == '#')
                while(c != '\n' && c != '\r')
                    get_symb();
            else if (isalpha(c))
            {
                clear();
                add();
                get_symb();
                G = IDENT;
            }
            else if (isdigit(c))
            {
                digit = c - '0';
                get_symb();
                G = NUMB;
            }
            else if (c == '/')
            {
                clear();
                add();
                get_symb();
                if (c == '*')
                {
                    get_symb();
                    G = COM;
                }
                else
                    G = SLASH;
            }
            else if (c == '"')
            {
                get_symb();
                G = QOUT;
            }
            else if (c == '<' || c == '>' || c == '=')
            {
                clear();
                add();
                get_symb();
                G = COMPR;
            }
            else if (c == '!')
            {
                clear();
                add();
                get_symb();
                G = NOT;
            }
            else if (c != EOF)
            {
                clear();
                add();
                G = DELIM;
            }
            else
                G = FIN_OF_FILE;
            break;

        case IDENT:
            if (isalpha(c) || isdigit(c))
            {
                add();
                get_symb();
            }
            else if ((num = define_type(buf, TW)))
                return Lex(wtypes[num], num);
            else // variable
            {
                num = TID.put(buf); // new or old
                return Lex(LEX_ID, num);
            }
            break;
        case NUMB:
            if (isdigit(c))
            {
                digit = digit * 10 + c - '0';
                get_symb();
            }
            else
                return Lex(LEX_NUM, digit);
            break;

        case COM:
            if (c == '*')
            {
                get_symb();
                if (c == '/')
                {
                    get_symb();
                    G = H;
                }
            }
            else if (c != EOF)
                get_symb();
            else if (c == EOF)
                G = FIN_OF_FILE;
            break;

        case SLASH:
            num = define_type(buf, TS);
            return Lex(stypes[num], num);

        case QOUT:
            if (c == '"')
            {
                get_symb();
                return Lex(LEX_STRING_VAL, str);
            }
            else if (c != EOF)
            {
                str.push_back(c);
                get_symb();
            }
            else if (c == EOF)
                G = FIN_OF_FILE;
            break;

        case COMPR:
            if (c == '=')
            {
                add();
                get_symb();
            }
            num = define_type(buf, TS);
            return Lex(stypes[num], num);

        case NOT:
            if (c == '=')
            {
                add();
                get_symb();
            }
            num = define_type(buf, TS);
            return Lex(stypes[num], num);

        case DELIM:
            if ((num = define_type(buf, TS)))
            {
                get_symb();
                return Lex(stypes[num], num);
            }
            else
                throw c;

        case FIN_OF_FILE:
            not_eof = false;
            return Lex();
        }
    }
}

const char *Lexer::TW[] =
    {
        "", // пусто1 т.к. при 0 считаем за ошибку
        "program",

        "int", "string",

        "read", "write",

        "if", "else",

        "do", "while",

        "true", "false",
        "or", "and",

        NULL};
type_of_lex Lexer::wtypes[] =
    {
        LEX_NULL,
        LEX_PROGRAM,
        LEX_INT, LEX_STRING,
        LEX_READ, LEX_WRITE,
        LEX_IF, LEX_ELSE,
        LEX_DO, LEX_WHILE,
        LEX_TRUE, LEX_FALSE,
        LEX_OR, LEX_AND,
        LEX_NULL};

const char *Lexer::TS[] =
    {
        "",

        "+", "-", "/", "*",

        ">", "<", ">=", "<=", "==", "!=",

        "=",

        ",", ";", "(", ")", "{", "}", "\"",
        "!",
        NULL};
type_of_lex Lexer::stypes[]{
    LEX_NULL,
    LEX_PLUS, LEX_MINUS, LEX_SLASH, LEX_STAR,
    LEX_GRT, LEX_LSS, LEX_GOE, LEX_LOE, LEX_EQ, LEX_NEQ,
    LEX_ASSIGN,
    LEX_COMMA, LEX_SEMICOLON, LEX_LPAREN, LEX_RPAREN, LEX_STBLOCK, LEX_FNBLOCK, LEX_QOTE,
    LEX_NOT,
    LEX_NULL};

template <typename T, int max_size>
class Stack
{
    T s[max_size];
    int top = 0;

public:
    Stack() {}
    void push(T t)
    {
        if (!is_full())
            s[top++] = t;
        else
            throw "Stack is full";
    }
    T pop()
    {
        if (!is_empty())
            return s[--top];
        else
            throw "Stack is empty";
    }

    void reset() { top = 0; }
    bool is_empty() const { return top == 0; }
    bool is_full() const { return top == max_size - 1; }
};

class Function
{
    char *name = nullptr;
    int poliz_num;
    int num_str = 0;
    int num_int = 0;
    type_of_lex array[30];
    int index = 0;

public:
    Function() {}
    char *get_name() const { return name; }
    void set_name(const char *n)
    {
        name = new char[strlen(n) + 1];
        strcpy(name, n);
    }
    void add_int() { num_int++; }
    void add_str() { num_str++; }
    void push(type_of_lex l) { array[index++] = l; }
    type_of_lex get_el(int i) const { return array[i]; }
    void set_num_str(int i) { num_str = i; }
    int get_num_str() const { return num_str; }
    void set_num_int(int i) { num_int = i; }
    int get_num_int() const { return num_int; }
    void set_poliz_num(int i) { poliz_num = i; }
    int get_poliz_num() const { return poliz_num; }
    void add_var(type_of_lex l)
    {
        if(l == LEX_INT)
            add_int();
        else if(l == LEX_STRING)
            add_str();
        else
            throw l;
        push(l);
    }
};

class FTable
{
    Function *table = nullptr;
    int top;
    int size;

public:
    FTable(int const size) : size(size)
    {
        table = new Function[size];
        top = 1;
    }
    ~FTable()
    {
        if (table != nullptr)
            delete[] table;
    }
    int put(const char *buf)
    {
        table[top].set_name(buf);
        top++;
        return top - 1;
    }
    Function& operator[] (int i) { return table[i]; }
    bool check_fn(const char *buf) const
    {
        for (int i = 1; i < top; i++)
            if (!strcmp(buf, table[i].get_name()))
                return true;
        return false;
    }
    int get_top() const { return top; }
} FN(100);

class Poliz
{
    Lex *p = nullptr;
    int size;
    int free;

public:
    Poliz(int max_size)
    {
        p = new Lex[max_size];
        size = max_size;
        free = 0;
    }
    ~Poliz() { delete[] p; }
    void put_lex(Lex l)
    {
        p[free] = l;
        free++;
    }
    type_of_lex get_c_type() { return p[free - 1].get_type(); }
    void put_lex(Lex l, int place) { p[place] = l; }
    void blank() { free++; }
    int get_free() const { return free; }
    Lex &operator[](int ident)
    {
        if (ident > size)
            throw "Poliz: out of array";
        if (ident > free)
            throw "Poliz: indefine element of array";
        return p[ident];
    }
    void print()
    {
        for (int i = 0; i < free; i++)
            std::cout << p[i];
    }
};

class Parser
{
    Lex curr_lex;
    type_of_lex c_type;
    int c_IntVal;
    std::string c_StrVal;
    int IntVal;
    std::string StrVal;

    Stack<int, 100> st_int;
    Stack<std::string, 100> st_str;
    Stack<type_of_lex, 100> st_lex;

    type_of_lex tmp;

    Lexer scan;

    void check_id()
    {
        if (TID[c_IntVal].get_declaration())
            st_lex.push(TID[c_IntVal].get_type());
        else
            throw "not declarated";
    }

    void check_semicolon()
    {
        if (c_type != LEX_SEMICOLON)
            throw curr_lex;
        get_lex(); // were do you want to check ;?
    }
    void get_lex()
    {
        curr_lex = scan.get_lex();
        c_type = curr_lex.get_type();
        c_IntVal = curr_lex.get_IntValue();
        c_StrVal = curr_lex.get_StrValue();
    }

    void check_not()
    {
        if (st_lex.pop() != LEX_BOOL)
            throw "wrong type is in not";
        else
        {
            st_lex.push(LEX_BOOL);
            prog.put_lex(Lex(LEX_NOT, 0));
        }
    }

    void check_op()
    {
        type_of_lex t1, t2, op, r, t;
        t2 = st_lex.pop();
        op = st_lex.pop();
        t1 = st_lex.pop();
        t = t1;
        if (t1 == t2)
        {
            if (t == LEX_BOOL)
            {
                if (op == LEX_AND || op == LEX_OR)
                    r = LEX_BOOL;
                else
                    throw op;
            }
            else if (t == LEX_INT)
            {
                if (op == LEX_EQ || op == LEX_NEQ || op == LEX_GRT || op == LEX_LSS || op == LEX_GOE || op == LEX_LOE)
                    r = LEX_BOOL;
                else if (op == LEX_MINUS || op == LEX_PLUS || op == LEX_SLASH || op == LEX_STAR)
                    r = LEX_INT;
                else
                    throw op;
            }
            else if (t == LEX_STRING)
            {
                if (op == LEX_EQ || op == LEX_NEQ || op == LEX_GRT || op == LEX_LSS || op == LEX_GOE || op == LEX_LOE)
                    r = LEX_BOOL;
                else if (op == LEX_PLUS)
                    r = LEX_STRING;
                else
                    throw op;
            }
            else
                throw t;
        }
        else
            throw t2;
        st_lex.push(r);
        prog.put_lex(Lex(op, 0));
    }

    void eq_bool()
    {
        if (st_lex.pop() != LEX_BOOL)
            throw "expression is not boolean";
    }

    void eq_type()
    {
        if (st_lex.pop() != st_lex.pop())
            throw "wrong types are in assignment";
    }
    void check_Lparen()
    {
        if (c_type != LEX_LPAREN)
            throw curr_lex;
        get_lex();
    }
    void check_Rparen()
    {
        if (c_type != LEX_RPAREN)
            throw curr_lex;
        get_lex();
    }

    void check_decl()
    {
        if (!TID[c_IntVal].get_declaration())
            throw curr_lex;
    }

    void P();
    void B();
    void S();
    void DE();
    void PE();
    void E();
    void E1();
    void T();
    void F();

    void STR();
    void I();

public:
    Poliz prog;
    Parser(char *program) : scan(program), prog(1000) {}
    void analyze();
};

void Parser::analyze()
{
    get_lex();
    P();
    //prog.print();
    std::cout << "Success !!\n";
    std::cout << "Program's output:\n";
}

void Parser::P()
{
    if (c_type != LEX_PROGRAM)
        throw curr_lex;  
    get_lex();
    B();
}

void Parser::B()
{
    if (c_type == LEX_STBLOCK)
    {
        get_lex();
        while (c_type != LEX_FNBLOCK)
        {
            S();
        }
        get_lex();
    }
    else
        throw curr_lex;
}

void Parser::S()
{
    int pl0, pl1, pl2, pl3;

    if (c_type == LEX_STRING)
    {
        get_lex();
        STR();
    }
    else if (c_type == LEX_INT)
    {
        get_lex();
        I();
    }
    else if (c_type == LEX_IF)
    {
        get_lex();
        check_Lparen();
        PE();
        eq_bool();
        check_Rparen();
        pl2 = prog.get_free();
        prog.blank();
        prog.put_lex(Lex(POLIZ_FGO, 0));

        if (c_type == LEX_STBLOCK)
            B();
        else
            S();

        if (c_type == LEX_ELSE)
        {
            pl3 = prog.get_free();
            prog.blank();
            prog.put_lex(Lex(POLIZ_GO, 0));
            prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl2);
            get_lex();
            if (c_type == LEX_STBLOCK)
            {
                B();
                prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl3);
            }
            else
            {
                S();
                prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl3);
            }
        }
        else
            prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl2);
    }
    else if (c_type == LEX_WHILE)
    {
        pl0 = prog.get_free();
        get_lex();
        check_Lparen();
        PE();
        eq_bool();
        check_Rparen();
        pl1 = prog.get_free();
        prog.blank();
        prog.put_lex(Lex(POLIZ_FGO, 0));

        if (c_type == LEX_STBLOCK)
            B();
        else
            S();

        prog.put_lex(Lex(POLIZ_LABEL, pl0));
        prog.put_lex(Lex(POLIZ_GO, 0));
        prog.put_lex(Lex(POLIZ_LABEL, prog.get_free()), pl1);
    }
    else if (c_type == LEX_READ)
    {
        int i;
        i = FN.put("read");
        get_lex();
        check_Lparen();
        while(c_type == LEX_ID)
        {
            if (c_type != LEX_ID)
                throw curr_lex;
            check_decl();
            FN[i].add_var(TID[c_IntVal].get_type());
            prog.put_lex(Lex(POLIZ_ADDRESS, c_IntVal));
            get_lex();
            if(c_type == LEX_COMMA)
                get_lex();
            else break;
        }
        check_Rparen();
        check_semicolon();
        FN[i].set_poliz_num(prog.get_free());
        prog.put_lex(Lex(LEX_READ, 0));
    }
    else if (c_type == LEX_WRITE)
    {
        int i;
        i = FN.put("write");
        get_lex();
        check_Lparen();
        E();
        FN[i].add_var(tmp);
        while(c_type == LEX_COMMA)
        {
            get_lex();
            E();
            FN[i].add_var(tmp);
        }
        check_Rparen();
        check_semicolon();
        FN[i].set_poliz_num(prog.get_free());
        prog.put_lex(Lex(LEX_WRITE, 0));
    }
    else if (c_type == LEX_ID)
    {
        check_id();
        prog.put_lex(Lex(POLIZ_ADDRESS, c_IntVal));
        get_lex();
        if (c_type == LEX_ASSIGN)
        {
            get_lex();
            DE();
            eq_type();
            prog.put_lex(Lex(LEX_ASSIGN, 0));
        }
        else
            throw curr_lex;
        check_semicolon();
        prog.put_lex(Lex(LEX_SEMICOLON, 0));
    }
    else
        B();
}

void Parser::STR()
{
    if (c_type != LEX_ID)
        throw curr_lex;

    int var_num;
    while (c_type != LEX_SEMICOLON)
    {
        if (c_type != LEX_ID)
            throw curr_lex;

        var_num = c_IntVal;
        if (TID[c_IntVal].get_declaration() == true)
            throw "double declaration";

        TID[var_num].set_declaration();
        TID[var_num].set_type(LEX_STRING);

        get_lex(); // comma || assign
        if (c_type == LEX_ASSIGN)
        {
            prog.put_lex(Lex(POLIZ_ADDRESS, var_num));
            get_lex();
            if (c_type == LEX_STRING_VAL)
            {
                TID[var_num].set_assign();
                prog.put_lex(Lex(LEX_STRING_VAL, c_StrVal));
                prog.put_lex(Lex(LEX_ASSIGN, 0));
                prog.put_lex(Lex(LEX_SEMICOLON, 0));
            }
            else
                throw curr_lex;
            get_lex(); // comma || semicolon
        }
        if (c_type == LEX_COMMA)
            get_lex();
    }
    check_semicolon();
}

void Parser::I()
{
    if (c_type != LEX_ID)
        throw curr_lex;

    int var_num;
    while (c_type != LEX_SEMICOLON)
    {
        if (c_type != LEX_ID)
            throw curr_lex;

        var_num = c_IntVal;
        if (TID[c_IntVal].get_declaration() == true)
            throw "double declaration";

        TID[var_num].set_declaration();
        TID[var_num].set_type(LEX_INT);

        get_lex(); // comma || assign
        if (c_type == LEX_ASSIGN)
        {
            prog.put_lex(Lex(POLIZ_ADDRESS, var_num));
            get_lex();
            if (c_type == LEX_NUM)
            {
                TID[var_num].set_assign();
                prog.put_lex(Lex(LEX_NUM, c_IntVal));
                prog.put_lex(Lex(LEX_ASSIGN, 0));
                prog.put_lex(Lex(LEX_SEMICOLON, 0));
            }
            else
                throw curr_lex;
            get_lex(); // comma || semicolon
        }
        if (c_type == LEX_COMMA)
            get_lex();
    }
    check_semicolon();
}

void Parser::DE()
{
    int count = 0;
    E();
    while(prog.get_c_type() == POLIZ_ADDRESS)
    {
        count++;
        get_lex();
        if (c_type == LEX_ID)
        {
            check_id();
            int i = c_IntVal;
            get_lex();
            if(c_type == LEX_ASSIGN)
                prog.put_lex(Lex(POLIZ_ADDRESS, i));
            else 
                prog.put_lex(Lex(LEX_ID, i));
        }
        if (c_type == LEX_NUM)
        {
            st_lex.push(LEX_INT);
            prog.put_lex(curr_lex);
            get_lex();
        }
        else if (c_type == LEX_STRING_VAL)
        {
            st_lex.push(LEX_STRING);
            prog.put_lex(curr_lex);
            get_lex();
        }
    }
    for (int i = 0; i < count; i++)
        prog.put_lex(Lex(LEX_ASSIGN, 0));
}

void Parser::PE()
{
    E();
    if (c_type == LEX_AND || c_type == LEX_OR)
    {
        st_lex.push(c_type);
        get_lex();
        PE();
        check_op();
    }
}

void Parser::E()
{
    E1();
    if (c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GRT || c_type == LEX_LOE || c_type == LEX_GOE || c_type == LEX_NEQ)
    {
        st_lex.push(c_type);
        get_lex();
        E1();
        check_op();
    }
}

void Parser::E1()
{
    T();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS)
    {
        st_lex.push(c_type);
        get_lex();
        T();
        check_op();
    }
}

void Parser::T()
{
    F();
    while (c_type == LEX_STAR || c_type == LEX_SLASH)
    {
        st_lex.push(c_type);
        get_lex();
        F();
        check_op();
    }
}

void Parser::F()
{
    if (c_type == LEX_ID)
    {
        check_id();
        int i = c_IntVal;
        tmp = TID[i].get_type();
        get_lex();
        if(c_type == LEX_ASSIGN)
                prog.put_lex(Lex(POLIZ_ADDRESS, i));
        else 
                prog.put_lex(Lex(LEX_ID, i));
    }
    else if (c_type == LEX_NUM)
    {
        tmp = LEX_INT;
        st_lex.push(LEX_INT);
        prog.put_lex(curr_lex);
        get_lex();
    }
    else if (c_type == LEX_STRING_VAL)
    {
        tmp = LEX_STRING;
        st_lex.push(LEX_STRING);
        prog.put_lex(curr_lex);
        get_lex();
    }
    else if (c_type == LEX_TRUE)
    {
        st_lex.push(LEX_BOOL);
        prog.put_lex(Lex(LEX_TRUE, true));
        get_lex();
    }
    else if (c_type == LEX_FALSE)
    {
        st_lex.push(LEX_BOOL);
        prog.put_lex(Lex(LEX_FALSE, false));
        get_lex();
    }
    else if (c_type == LEX_NOT)
    {
        get_lex();
        F();
        check_not();
    }
    else if (c_type == LEX_LPAREN)
    {
        get_lex();
        E();
        if (c_type == LEX_RPAREN)
            get_lex();
        else
            throw curr_lex;
    }
    else
        throw curr_lex;
}

class Executer
{
    Lex pc_el;
    int flag = 0;//100-int 1000-string

public:
    int get_flag() const { return flag; }
    void rezero_flag() { flag = 0; }

    void set_flag_int() { flag = 100;}
    void set_flag_string() { flag = 1000;}

    bool flag_is_int() const { return flag == 100;}
    bool flag_is_string() const { return flag == 1000;}

    void execute(Poliz &prog);
};

void Executer::execute(Poliz &prog)
{
    Stack<int, 100> st_var;//переменные
    Stack<int, 100> st_move;//переход
    Stack<bool, 100> st_bool;//bool
    Stack<int, 100> st_num;//числа
    Stack<int, 100> st_wr_num;
    Stack<std::string, 100> st_str;//строки
    Stack<std::string, 100> st_wr_str;
    int i, j, index = 0, f_index = 100, size = prog.get_free();
    std::string str;
    bool t;
    int k1, k2, k;
    while (index < size)
    {
        pc_el = prog[index];
        switch (pc_el.get_type())
        {
        case LEX_TRUE:
        case LEX_FALSE:
            st_bool.push(pc_el.get_BoolValue());
            break;
        case LEX_NUM:
            set_flag_int();
            st_num.push(pc_el.get_IntValue());
            break;
        case LEX_STRING_VAL:
            set_flag_string();
            st_str.push(pc_el.get_StrValue());
            break;
        case POLIZ_LABEL:
            st_move.push(pc_el.get_IntValue());
            break;
        case POLIZ_ADDRESS:
            st_var.push(pc_el.get_IntValue());
            break;
        case LEX_ID:
            i = pc_el.get_IntValue();
            if (TID[i].get_assign())
            {
                if(TID[i].get_type() == LEX_STRING)
                {
                    set_flag_string();
                    st_str.push(TID[i].get_StrValue());
                }
                else
                {
                    set_flag_int();
                    st_num.push(TID[i].get_IntValue());
                }
                break;
            }
            else
                throw "POLIZ: indefinite identifier";
        case LEX_NOT:
            st_bool.push(!st_bool.pop());
            break;
        case LEX_OR:
            t = st_bool.pop();
            st_bool.push(st_bool.pop() || t);
            break;
        case LEX_AND:
            t = st_bool.pop();
            st_bool.push(st_bool.pop() && t);
            break;
        case POLIZ_GO:
            index = st_move.pop() - 1;
            break;
        case POLIZ_FGO:
            i = st_move.pop() - 1;//т. к. потом index++;
            if (!st_bool.pop())
                index = i;
            break;
        case LEX_WRITE:
            for(int i = 0; i < FN.get_top(); i++)
                if(FN[i].get_poliz_num() == index)
                    f_index = i;
            k1 = FN[f_index].get_num_int();
            k2 = FN[f_index].get_num_str();

            for(int i = 0; i < k1; i++)
                st_wr_num.push(st_num.pop());

            for(int i = 0; i < k2; i++)
                st_wr_str.push(st_str.pop());

            for(int i = 0; i < k1 + k2; i++)
            {
                if(FN[f_index].get_el(i) == LEX_INT)
                    std::cout << st_wr_num.pop() << std::endl;
                else 
                    std::cout << st_wr_str.pop() << std::endl;
            }
            f_index++;
            break;
        case LEX_READ:
            for(int i = 0; i < FN.get_top(); i++)
                if(FN[i].get_poliz_num() == index)
                    f_index = i;
            k1 = FN[f_index].get_num_int();
            k2 = FN[f_index].get_num_str();

            for(int i = 0; i < k1+k2; i++)
                st_wr_num.push(st_var.pop());

            for(int i = 0; i <k1 + k2; i++)
            {
                j = st_wr_num.pop();
                if(FN[f_index].get_el(i) == LEX_INT)
                {
                    std::cout << "Input int value for ";
                    std::cout << TID[j].get_name() << std::endl;
                    std::cin >> k;
                    TID[j].set_IntValue(k);
                }
                else
                {
                    std::cout << "Input string value for ";
                    std::cout << TID[j].get_name() << std::endl;
                    std::cin >> str;
                    TID[j].set_StrValue(str);
                }
                TID[j].set_assign();
            }
            f_index++;
            break;
        case LEX_PLUS:
            if(flag_is_string())
                st_str.push(st_str.pop() + st_str.pop());
            else
                st_num.push(st_num.pop() + st_num.pop());
            break;
        case LEX_STAR:
            st_num.push(st_num.pop() * st_num.pop());
            break;
        case LEX_MINUS:
            i = st_num.pop();
            st_num.push(st_num.pop() - i);
            break;
        case LEX_SLASH:
            i = st_num.pop();
            if (!i)
                st_num.push(st_num.pop() / i);
            else
                throw "POLIZ:divide by zero";
            break;
        case LEX_EQ:
            if(flag_is_string())
                st_bool.push(st_str.pop() == st_str.pop());
            else
                st_bool.push(st_num.pop() == st_num.pop());
            break;
        case LEX_LSS:
            if(flag_is_string())
            {
                str = st_str.pop();
                st_bool.push(st_str.pop() < str);
            }
            else
            {
                i = st_num.pop();
                st_bool.push(st_num.pop() < i);
            }
            break;
        case LEX_GRT:
            if(flag_is_string())
            {
                str = st_str.pop();
                st_bool.push(st_str.pop() > str);
            }
            else
            {
                i = st_num.pop();
                st_bool.push(st_num.pop() > i);
            }
            break;
        case LEX_LOE:
            if(flag_is_string())
            {
                str = st_str.pop();
                st_bool.push(st_str.pop() <= str);
            }
            else
            {
                i = st_num.pop();
                st_bool.push(st_num.pop() <= i);
            }
            break;
        case LEX_GOE:
            if(flag_is_string())
            {
                str = st_str.pop();
                st_bool.push(st_str.pop() >= str);
            }
            else
            {
                i = st_num.pop();
                st_bool.push(st_num.pop() >= i);
            }
            break;
        case LEX_NEQ:
            if(flag_is_string())
            {
                str = st_str.pop();
                st_bool.push(st_str.pop() != str);
            }
            else
            {
                i = st_num.pop();
                st_bool.push(st_num.pop() != i);
            }
            break;
        case LEX_ASSIGN:
            j = st_var.pop();
            if(flag_is_int())
            {
                i = st_num.pop();
                st_num.push(i);
                TID[j].set_IntValue(i);
            }
            else
            {
                str = st_str.pop();
                st_str.push(str);
                TID[j].set_StrValue(str);
            }
            TID[j].set_assign();
            break;
        case LEX_SEMICOLON:
            if(flag_is_int())
                st_num.pop();
            else
                st_str.pop();
            rezero_flag();
            break;
        default:
            throw "POLIZ: unexpected elem";
        }
        ++index;
    }
    std::cout << "Finish of executing." << std::endl;
}

class Interpretator
{
    Parser pars;
    Executer E;

public:
    Interpretator(char *program) : pars(program){};
    void interpretation();
};
void Interpretator::interpretation()
{
    pars.analyze();
    E.execute(pars.prog);
}

int main(int argc, char **argv)
{
    try
    {
        if(argc >= 2)
        {
            Interpretator I(argv[1]);
            I.interpretation();
        }
        else
            std::cout<<"There is no program\n";
        return 0;
    }
    catch (type_of_lex t)
    {
        std::cout << "Error: lex type == " << t << std::endl;
    }
    catch (Lex l)
    {
        std::cout << "Error: lex type == " << l.get_type() << std::endl;
    }
    catch (const char *str)
    {
        std::cout << "Error: " << str << std::endl;
    }
}
