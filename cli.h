#ifndef CLI_H
#define CLI_H
#include <iterator>
#include "utilities.h"
#include "databasemanager.h"
#include "dbcli_lexer.h"

using namespace std;

class Client {
    public:
        Lexer *lexer;
        DatabaseManager * dbm;

        Client();
        
        string getCommand();
        void cli_init();
        vector<string> build();

        virtual ~Client();
};

#endif