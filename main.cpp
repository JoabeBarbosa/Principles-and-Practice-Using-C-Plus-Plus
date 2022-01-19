#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

//Symbolic constants

const char number = '8';
const char print = ';';
const char quit = 'q';
const string prompt = "> ";
const string result = "= "; //Used to indicate that what follows is a result
const char name = 'a';
const char let = 'L';
const string declkey = "let";
const char sroot = '~';
const char power = '^';
const char change = 'C';
const char constant = 'T';
const string constkey = "const";

class Token{
public:
    char kind;
    double value;
    string name;
    Token(char sign)
    :kind(sign), value(0){}
    Token(char sign, double number)
    :kind(sign), value(number){}
    Token(char sign, string n):
    kind(sign), name(n){}
};

class Token_stream{
public:
    Token_stream();
    Token get();
    void putback(Token);
    bool get_full();
    void ignore(char);
private:
    bool full;
    Token buffer;
};

class Variable{
public:
    string name;
    double value;
    bool is_constant;
    Variable(string s, double d)
    :name(s), value(d), is_constant(false){}
    Variable(string s, double d, bool b)
    :name(s), value(d), is_constant(b){}
};

class Symbol_table{
public:
    void set_value(string, double);
    double get_value(string);
    double declare();
    bool is_declared(string);
    vector<Variable>var_table;
};

Token_stream::Token_stream()
:full(false), buffer(0){};

void Token_stream::putback(Token t)
{
    if(full)throw(t);
    buffer = t;
    full = true;
}

bool Token_stream::get_full()
{
    return full;
}

void Token_stream::ignore(char c)
{
    if(full && buffer.kind==c){
        full = false;
        return;
    }
    full = false;
    char ch = 0;
    while(cin>>ch)
        if(ch==c)
        return;

}

Symbol_table st;
double expression();
Variable va {" ", 0, false};

Token Token_stream::get()
{
    if(full)
        {
            full = false;
            return buffer;
        }
    char sign;
    cin.get(sign);
    if(isspace(sign)&&sign=='\n')return Token{print};
    cin.putback(sign);
    cin >> sign;
    switch(sign){
    case '^': // power
    case '~': // sroot
    case ';': // print
    case '=': // result
    case 'q': // quit
    case '(':
    case ')':
    case '+':
    case '*':
    case '/':
    case '-':
    case '{':
    case '}':
    case '!':
    case '%':
        return Token{sign};
    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    {
        cin.putback(sign);
        double val;
        cin >> val;
        return Token{number, val};
    }
default:
    if(isalpha(sign)){
        string s;
        s += sign;
        while(cin.get(sign)&&(isalpha(sign)||isdigit(sign)||sign=='_'))s+=sign;
        if(isspace(sign)&&sign=='\n')cin.putback(print);
        else cin.putback(sign);
        if(s==declkey){va.is_constant = false; return Token{let};}
        if(s==constkey){va.is_constant = true; return Token{constant};}
        cin >> sign;
        if(sign=='='&&st.is_declared(s))return Token{change, s};
        cin.putback(sign);
        for(const Variable& v:st.var_table)if(v.name==s)return Token(number, v.value);
        if(va.is_constant)return Token{constant, s};
        return Token{name, s};
    }
    throw(sign);
 }
}

Token_stream ts;

double redeclaration();

double expression();

double term();

double powr();

double factorial();

double primary();

double Symbol_table::get_value(string s)
{
    for(const Variable& v:st.var_table)
        if(v.name==s)return v.value;
    throw(s);
}

void Symbol_table::set_value(string s, double d)
{
    for(Variable& v:st.var_table)
        if(v.name==s&&!v.is_constant){
            v.value = d;
            return;
        }
    throw(s);
}

double statement();
void clean_up_mess();

void calculate()
{
    double val = 0;
    while(cin)
    try{
        if(!ts.get_full())
            cout << prompt;
        Token t = ts.get();
        if(t.kind==quit)return;
        if(t.kind==print)
            cout << result << val << '\n';
        else
            ts.putback(t);
        if(!ts.get_full())
            cout << prompt;
        val = statement();
    }
    catch(char myChar)
    {
        cout << "Exception! Bad input: " << myChar << endl;
        cout << "Try again!\n" << endl;
        clean_up_mess();

    }
    catch(string myString){
        cout << "Error! String in Calculation() " << myString << endl;
    }
    catch(double myDouble){
        cout << "Error! Double in Calculation() " << myDouble << endl;
    }
    catch(Token myToken){
        cout << "Error! Token calculation()! t.kind = " << myToken.kind << endl;
    }
}

void clean_up_mess()
{
    ts.ignore(print);
}

double statement()
{
    Token t = ts.get();
    switch(t.kind){
    case let: case constant:
        return st.declare(); //Declaration return new variable's value, which simplifies the code.
    case change:
        ts.putback(t);
        return redeclaration();
    default:
        ts.putback(t);
        return expression();
    }
}

bool Symbol_table::is_declared(string var)
{
    for(const Variable& v:var_table)
        if(v.name==var)return true;
    return false;
}

double define_name(string var, double val, bool b)
{
    if(st.is_declared(var))throw(var);
    st.var_table.push_back(Variable{var, val, b});
    return val;
}

double Symbol_table::declare()
{
    Token t = ts.get();
    if(t.kind!=name&&t.kind!=constant)throw(t.kind);
    string var_name = t.name;

    Token t2 = ts.get();
    if(t2.kind!='=')throw(t2.kind);

    double d = expression();
    define_name(var_name, d, va.is_constant);
    return d;
}

double redeclaration()
{
    Token t = ts.get();
    double d = expression();
    st.set_value(t.name, d);
    return d;
}

double expression()
{
    double left = term();
    Token t = ts.get();
    while(true){
        switch(t.kind){
        case '+':
            left += term();
            t = ts.get();
            break;
        case '-':
            left -= term();
            t = ts.get();
            break;
        default:
            ts.putback(t);
            return left;
        }
    }
}

double term()
{
    double left = powr();
    Token t = ts.get();
    while(true){
        switch(t.kind){
        case '%':
            {
                double d = factorial();
                if(d == 0) throw(d);
                left = fmod(left,d);
                t = ts.get();
                break;
            }
        case '*':
            left *= factorial();
            t = ts.get();
            break;
        case '/':
            {
                double d = factorial();
                if(d==0) throw(d); //throw a double, zero. Don't divide by zero.
                left /= d;
                t = ts.get();
                break;
            }
        default:
            ts.putback(t);
            return left;
        }
    }
}

double powr()
{
    double left = factorial();
    Token t = ts.get();
    while(true){
        switch(t.kind){
        case power:
            {
                int i = static_cast<int>(factorial());
                left = pow(left, i);
                t = ts.get();
                break;
            }

        default:
            ts.putback(t);
            return left;

        }
    }
}

double factorial()
{
    double left = primary();
    Token t = ts.get();
    while(true){
        switch(t.kind){
    case '!':
        {
            int i = static_cast<int>(left);
            if(i==0||i==1){
                i = 1;
            left = i;
            t = ts.get();
            break;
            }
            for(int j=(i-1); j>0; --j)
                i*=j;
            left = i;
            t = ts.get();
            break;
        }

    default:
        ts.putback(t);
        return left;
     }
    }
}

double primary()
{
    Token t = ts.get();
    switch(t.kind){
    case sroot: //square root
        {
            double d = primary();
            if(d<0)throw(d);
            return sqrt(d);
        }
    case quit:
        ts.putback(t);
        return 0;
    case '+':
        return +primary();
    case '-':
        return -primary();
    case '{':
        {
            double c = expression();
            t = ts.get();
            if(t.kind!='}')throw(t);
            return c;
        }
    case '(':
        {
            double d = expression();
            t = ts.get();
            if(t.kind!=')') throw(t); // Throw t!=')'. Is not a expression.
            return d;
        }
    case number:
        return t.value;
    default:
        throw(t.kind); // Error: primary expected.
    }
}

int main()
try
{
    cout << "Welcome to our simple calculator.\n"
            "Please enter expression using floating-point numbers.\n"
            "Operators available: +, -, !, * and /.\n"
            "To print the evaluation on the screen enter ;.\n"
            "To quit enter the letter q.\n";

    define_name("pi", 3.1415926535, true);
    define_name("e", 2.7182818284, true);
    define_name("k", 1000, true);

    calculate();
}

catch(char myChar){
    if(myChar==quit)
        cout << "By By!" << endl;
    else
        cout << "Error. Bad Token: " << myChar <<  '\n';

}
catch(string myString){
    cout << "Error! String exception! " << myString << endl;
}

catch(...)
{
    cout << "Error! Exception!" << endl;
}
