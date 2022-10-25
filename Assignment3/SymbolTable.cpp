#include "SymbolTable.h"

void SymbolTable::run(string filename)
{

    ifstream f(filename);
    int curScope = 0;
    if (!f.eof())
    {
        string cmd;
        getline(f, cmd);
        if (regex_match(cmd, linear_com))
        {
            hf = hashLinear;
            int position = 0, lth = cmd.length();
            for (int i = 7; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position = i;
                    break;
                }
            }
            m = stoi(cmd.substr(7, position - 7));
            data = new Data[m];
            c1 = stoi(cmd.substr(position + 1, lth - position - 1));
        }
        else if (regex_match(cmd, quad_com))
        {
            hf = hashQuadratic;
            int position[2] = {0, 0}, counter = 0, lth = cmd.length();
            for (int i = 10; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position[counter++] = i;
                }
            }
            m = stoi(cmd.substr(10, position[0] - 10));
            data = new Data[m];
            c1 = stoi(cmd.substr(position[0] + 1, position[1] - position[0] - 1));
            c2 = stoi(cmd.substr(position[1] + 1, lth - position[1] - 1));
        }
        else if (regex_match(cmd, double_com))
        {
            hf = hashDouble;
            int position = 0, lth = cmd.length();
            for (int i = 7; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position = i;
                    break;
                }
            }
            m = stoi(cmd.substr(7, position - 7));
            data = new Data[m];
            c1 = stoi(cmd.substr(position + 1, lth - position - 1));
        }
        else
        {
            throw(InvalidInstruction(cmd));
        }
    }
    while (!f.eof())
    {
        string cmd;
        getline(f, cmd);
        if (regex_match(cmd, insertVal_com))
        {
            string idf_Name = cmd.substr(7, cmd.length() - 7);
            insertVal(cmd, idf_Name, curScope);
        }
        else if (regex_match(cmd, insertFunc_com))
        {
            int position = 0, lth = cmd.length();
            for (int i = 7; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position = i;
                    break;
                }
            }

            insertFunction(cmd, cmd.substr(7, position - 7), stoi(cmd.substr(position + 1, lth - position - 1)), curScope);
        }
        else if (regex_match(cmd, assignVal_com))
        {
            int position = 0, lth = cmd.length();
            for (int i = 7; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position = i;
                    break;
                }
            }
            string name = cmd.substr(7, position - 7);
            assignImValue(cmd, name, cmd.substr(position + 1, lth - position - 1), curScope);
        }
        else if (regex_match(cmd, assignVari_com))
        {
            int position = 0, lth = cmd.length();
            for (int i = 7; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position = i;
                    break;
                }
            }
            assignWithVariable(cmd, cmd.substr(7, position - 7), cmd.substr(position + 1, lth - position - 1), curScope);
        }
        else if (regex_match(cmd, assignFunc_com))
        {
            int position = 0, lth = cmd.length();
            for (int i = 7; i < lth; i++)
            {
                if (cmd[i] == ' ')
                {
                    position = i;
                    break;
                }
            }
            assignWithFunction(cmd, cmd.substr(7, position - 7), cmd.substr(position + 1, lth - position - 1), curScope);
        }
        else if (regex_match(cmd, lookup_com))
        {
            int lth = cmd.length(), temp;
            string idf_Name = cmd.substr(7, lth - 7);
            Data *res = lookUp(cmd, idf_Name, curScope, temp);
            cout << (int)(res - data) << endl;
        }
        else if (regex_match(cmd, call_com))
        {
            callFunction(cmd, cmd.substr(5, cmd.length() - 5), curScope);
        }
        else if (cmd == "PRINT")
        {
            print();
        }
        else if (cmd == "BEGIN")
        {
            curScope++;
        }
        else if (cmd == "END")
        {
            if (curScope - 1 < 0)
            {
                throw(UnknownBlock());
            }
            deleteScope(cmd, curScope);
            curScope--;
        }
        else
        {
            throw(InvalidInstruction(cmd));
        }
    }
    if (curScope != 0)
    {
        throw(UnclosedBlock(curScope));
    }
    f.close();
}
