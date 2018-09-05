#include "cli.h"

Client::Client() {
    dbm = new DatabaseManager();
    lexer = new Lexer();
    cli_init();
}

void Client::cli_init() {
    bool exit = false;

    while (!exit) {
        string input = getCommand();
        vector<string> entrance;
        lexer->set_input(input);

        try {
            entrance = lexer->build_vector();
        }catch (const std::invalid_argument& ia) {
            std::cerr << "Lexer error, invalid expression: " << ia.what() << '\n';
            continue;
        }

        if (entrance[0] == "HELP") {
            const string dbcli_commands = "\n\n\t\t-CREATE DATABASE [DATABASE_NAME] [DATABASE_SIZE];\n"
                                        "\t\t-DROP DATABASE [DATABASE_NAME];\n"
                                        "\t\t-PRINT DATABASE [DATABASE_NAME];\n"
                                        "\t\t-CREATE TABLE [TABLE_NAME] [FIELD_NAME FIELD_TYPE(FIELD_SIZE | ' ')]*;\n"
                                        "\t\t-DROP TABLE [TABLE_NAME];\n"
                                        "\t\t-PRINT TABLE [TABLE_NAME];\n"
                                        "\t\t-INSERT INTO [TABLE_NAME] { [TABLE_COLUMN] = [COLUMN_VALUE] }*;\n"
                                        "\t\t-EXIT;\n\n";
            cout << dbcli_commands << endl;

        } else if(entrance[0] == "CREATE"){
            if(validateEntranceLen(entrance, 2) && entrance[1] == "TABLE"){
                dbm->create_table(entrance);

            }else if(validateEntranceLen(entrance, 2) && entrance[1] == "DATABASE"){
                if(!validateEntranceLen(entrance, 4)){
                    continue;
                }

                unsigned int size = 0;
                from_String_to_uint(entrance[3],&size);
                dbm->create_database(entrance[2], size);

            }else{
                cout << "Command not supported, type HELP; for usage." << endl;

            }
        }else if(entrance[0] == "CONNECT"){
            if(entrance.size() < 2){
                cout << "Not enough arguments, type HELP; for usage." << endl;
                continue;

            }
            dbm->use_database(entrance[1]);

        } else if (entrance[0] == "DROP") {
            if(validateEntranceLen(entrance, 2) && entrance[1] == "TABLE"){
                try{
                    if(entrance.size() < 3){
                        cout << "Not enough arguments, type HELP; for usage." << endl;
                        continue;
                    }
                    dbm->drop_table(entrance[2]);

                } catch (const std::invalid_argument& ia) {
                    std::cerr << ia.what() << '\n';
                    continue;
                }
            }else if(validateEntranceLen(entrance, 2) && entrance[1] == "DATABASE") {
                if(entrance.size() < 3){
                    cout << "Not enough arguments, type HELP; for usage." << endl;
                    continue;
                }
                dbm->drop_database(entrance[2]);
            }else {
                cout << "Command does't exist, type HELP; for usage." << endl;
            }
        }
        else if(entrance[0] == "INSERT"){
            if(!validateEntranceLen(entrance,3)){
                continue;
            }

            try{
                dbm->insert(entrance);

            }catch (const std::invalid_argument& ia) {
                std::cerr << ia.what() << '\n';
                continue;

            }
        }else if(entrance[0] == "PRINT"){
            if(!validateEntranceLen(entrance,2))
                continue;

            if(entrance[1]== "DATABASE"){
                dbm->print_database_info();

            }else if(entrance[1] == "TABLE"){
                if(!validateEntranceLen(entrance,3)) {
                    continue;
                }

                dbm->print_table_info(entrance[2]);

            }else{
                cout << "Command doesn't exist, type HELP; for usage" << endl;
                continue;

            }
        }
        else if(entrance[0] == "EXIT"){
            exit = true;

        }else{
            cout << "Command doesn't exist, type HELP; for usage." << endl;

        }
    }
}

vector<string> Client::build() {
        vector<string> entrance;

        try {
            entrance = lexer->build_vector();
        }catch (const std::invalid_argument& ia) {
            std::cerr << "Invalid format: " << ia.what() << '\n';
            return entrance;
        }

        return entrance;
}

string Client::getCommand() {
    string command, myCommand = "";
    bool exit = false;

    cout << "DBCLI:" << dbm->use << ">" ;

    while(!exit){
        getline(cin, command);
        if (command.find(";") != std::string::npos) {
            exit = true;
        } else {
            command.append(" ");
        }
        myCommand.append(command);
    }

    return myCommand;
}

Client::~Client()
{

}