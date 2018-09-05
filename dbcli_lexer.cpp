#include "dbcli_lexer.h"

Lexer::Lexer() {
    input = "";
    input_count = 0;
}

void Lexer::set_input(string input) {
    this->input = input;
    input_count = 0;
    length = input.size();
}

char Lexer::get_next_character() {
    if (input_count < length) {
        char c = toupper(input[input_count++]);
        return c;
    }

    return '\0';
}

bool Lexer::is_digit(char currentSymbol){
    if( (int)currentSymbol >= 48 && (int)currentSymbol <=57 ){
        return true;
    }
    return false;
}

bool Lexer::is_letter(char currentSymbol){
    if(((int)currentSymbol >= 65 && (int)currentSymbol <= 90) || ((int)currentSymbol >= 97 && (int)currentSymbol <= 122)) {
        return true;
    }
    return false;
}

vector<string> Lexer::build_vector() {
    vector<string> input_vector;
    char currentSymbol = get_next_character();

    while(currentSymbol != '\0') {
        while ((int)currentSymbol == 32 || currentSymbol == '\r' || currentSymbol == '\n' || currentSymbol == '\t') {
            currentSymbol = get_next_character();
        }

        if (is_letter(currentSymbol)) {
            string lexeme = "";
            do {
                lexeme += currentSymbol;
                currentSymbol = get_next_character();
            } while (is_letter(currentSymbol) || is_digit(currentSymbol) );
            input_vector.push_back(lexeme);
            continue;
        }

        if(is_digit(currentSymbol)){
            string lexema = "";
            do{
                lexema += currentSymbol;
                currentSymbol = get_next_character();
            }while(is_digit(currentSymbol)|| currentSymbol == '.');
            input_vector.push_back(lexema);
            continue;
        }
        
        if(currentSymbol == '"'){
            string lexema = "";
            do{
                if(currentSymbol == '\0')
                    throw std::invalid_argument("Error: expected \".");
                lexema += currentSymbol;
                currentSymbol = get_next_character();
            }while(currentSymbol != '"');
            lexema += currentSymbol;
            currentSymbol = get_next_character();
            input_vector.push_back(lexema);
            continue;
        }
        
        if(currentSymbol == '('){
            string lexema = "";
            do {
                lexema += currentSymbol;
                currentSymbol = get_next_character();

                while ((int)currentSymbol == 32 || currentSymbol == '\r' || currentSymbol == '\n' || currentSymbol == '\t') {
                    currentSymbol = get_next_character();
                }

                if (!is_digit(currentSymbol) && currentSymbol != ')') {
                    string rest = "";
                    rest += currentSymbol;
                    //cout<<"-"<<currentSymbol<<"-"<<is_digit(currentSymbol)<<endl;
                    throw std::invalid_argument("Error: expected a digit inside \"()\" and received: "+rest);
                }
            } while (currentSymbol != ')');
            
            lexema += currentSymbol;
            currentSymbol = get_next_character();
            input_vector.push_back(lexema);
            
            continue;
        }

        if(currentSymbol == '=' || currentSymbol == '*'){
            string lexema = "";
            lexema += currentSymbol;
            currentSymbol = get_next_character();
            input_vector.push_back(lexema);
            continue;
        }
        
        if(currentSymbol == ';'){
            currentSymbol = get_next_character();
            continue;
        }
        
        string rest = "";
        rest += currentSymbol;
        throw std::invalid_argument( "Symbol not supported: '"+rest+"'" );
    }
    return input_vector;
}

Lexer::~Lexer() {

}