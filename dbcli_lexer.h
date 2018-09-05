#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <vector>
#include <stdio.h>
#include <ctype.h>
#include <stdexcept>

using namespace std;

class Lexer {
    public:
        int input_count;
        int length;
        bool is_digit(char character);
        bool is_letter(char character);
        string input;
        void set_input(string input);
        char get_next_character();
        Lexer();
        vector<string> build_vector();
        virtual ~Lexer();

    protected:

    private:
};

#endif