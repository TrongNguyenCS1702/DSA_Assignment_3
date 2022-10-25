#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include "main.h"

regex linear_com("LINEAR [0-9]+ [0-9]+");
regex quad_com("QUADRATIC [0-9]+ [0-9]+ [0-9]+");
regex double_com("DOUBLE [0-9]+ [0-9]+");
regex insertVal_com("INSERT [a-z][a-zA-Z0-9_]*");
regex insertFunc_com("INSERT [a-z][a-zA-Z0-9_]* [0-9]+");
regex assignVal_com("ASSIGN [a-z][a-zA-Z0-9_]* ([0-9]+|\'[a-zA-Z0-9 ]*\')");
regex assignVari_com("ASSIGN [a-z][a-zA-Z0-9_]* [a-z][a-zA-Z0-9_]*");
regex assignFunc_com(R"(ASSIGN [a-z][a-zA-Z0-9_]* [a-z][a-zA-Z0-9_]*\((|(([0-9]+|\'[a-zA-Z0-9 ]*\'|[a-z][a-zA-Z0-9_]*)((,[0-9]+)|(,\'[a-zA-Z0-9 ]*\')|(,[a-z][a-zA-Z0-9_]*))*))\))");
regex lookup_com("(LOOKUP )([a-z][a-zA-Z0-9_]*)");
regex call_com(R"(CALL [a-z][a-zA-Z0-9_]*\((|(([0-9]+|'[a-zA-Z0-9 ]*'|[a-z][a-zA-Z0-9_]*)((,[0-9]+)|(,'[a-zA-Z0-9 ]*')|(,[a-z][a-zA-Z0-9_]*))*))\))");

class Node
{
public:
    int position;
    Node *next;
    Node()
    {
        position = -1;
        next = NULL;
    }
    Node(int p, Node *n)
    {
        position = p;
        next = n;
        return;
    }
};
class Ls
{
public:
    Node *head;
    Ls()
    {
        head = NULL;
        return;
    }
    void push(int position)
    {
        if (!head)
        {
            head = new Node(position, NULL);
            return;
        }
        Node *newNode = new Node(position, head);
        head = newNode;
        return;
    }
    void pop()
    {
        if (!head)
            return;
        Node *temp = head;
        head = head->next;
        delete temp;
        return;
    }

    ~Ls()
    {
        while (head)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
        }
        head = NULL;
        return;
    }
};

class Data
{
public:
    string ins, type, argL;
    int level;
    Data()
    {
        ins = type = argL = "";
        level = -2;
        // level = -2: there is no element and tombstone here.
    }
    Data(string ins, int level, string type, string L)
    {
        this->ins = ins;
        this->level = level;
        this->type = type;
        this->argL = L;
    }
};

class SymbolTable
{
public:
    Ls nodeL;
    int m, c1, c2, size;
    int (*hf)(string, int, int, int, int);
    Data *data;
    SymbolTable()
    {
        m = c1 = c2 = size = 0;
        hf = NULL;
        data = NULL;
        nodeL = Ls();
    }
    ~SymbolTable()
    {
        if (data)
            delete[] data;
    }
    void run(string filename);

    void insertVal(string cmd, string ins, int level)
    {
        string key = encodeKey(ins, level);
        int i = 0;
        for (; i < m; i++)
        {
            int posHash = hf(key, i, m, c1, c2);
            if (data[posHash].ins == ins && data[posHash].level == level)
            {
                throw(Redeclared(ins));
            }
            if (data[posHash].level < 0)
            {
                nodeL.push(posHash);
                data[posHash].ins = ins;
                data[posHash].level = level;
                break;
            }
        }
        if (i == m)
        {
            throw(Overflow(cmd));
        }
        cout << i << endl;
        size++;
        return;
    }
    string encodeKey(string s, int level)
    {
        string res = to_string(level);
        int lth = s.length();
        for (int i = 0; i < lth; i++)
        {
            res += to_string((int)(s[i] - 48));
        }
        return res;
    }
    void insertFunction(string cmd, string ins, int numberArg, int level)
    {
        string key = encodeKey(ins, level);
        int i = 0;
        for (; i < m; i++)
        {
            int posHash = hf(key, i, m, c1, c2);
            if (data[posHash].ins == ins && data[posHash].level == level)
            {
                throw(Redeclared(ins));
            }
            if (data[posHash].level < 0)
            {
                data[posHash].ins = ins;
                data[posHash].level = 0;
                data[posHash].argL = string(numberArg + 1, 'N');
                data[posHash].type = "function";
                break;
            }
        }
        if (level > 0)
        {
            throw(InvalidDeclaration(ins));
        }
        if (i == m)
        {
            throw(Overflow(cmd));
        }

        cout << i << endl;
        size++;
        return;
    }

    void assignImValue(string cmd, string ins, string val, int level)
    {
        int countPass = 0;
        Data *foundElement = lookUp(cmd, ins, level, countPass);
        string valtype = (regex_match(val, regex("[0-9]+"))) ? "number" : "string";
        if (foundElement->type == "")
        {
            foundElement->type = valtype;
            cout << countPass << endl;
            return;
        }
        if (foundElement->type != valtype)
        {
            throw(TypeMismatch(cmd));
        }
        return;
    }
    void assignWithVariable(string cmd, string ins, string variable, int level)
    {
        int countPass = 0;
        Data *foundVariable = lookUp(cmd, variable, level, countPass);
        Data *foundID = lookUp(cmd, ins, level, countPass);
        if (foundVariable->type == "" && foundID->type == "")
        {
            throw(TypeCannotBeInferred(cmd));
        }
        else if (foundVariable->type == "function" || foundID->type == "function")
        {
            throw(TypeMismatch(cmd));
        }
        else if (foundVariable->type != "" && foundID->type == "")
        {
            foundID->type = foundVariable->type;
        }
        else if (foundVariable->type == "" && foundID->type != "")
        {
            foundVariable->type = foundID->type;
        }
        else if (foundVariable->type != foundID->type)
        {
            throw(TypeMismatch(cmd));
        }
        cout << countPass << endl;
        return;
    }
    void assignWithFunction(string cmd, string ins, string func, int level)
    {
        int start_arg = 0, end_arg = 0, lth = func.length();
        int num_pass = 0;
        for (int i = 0; i < lth; i++)
        {
            if (func[i] == '(')
            {
                start_arg = i;
                break;
            }
        }
        string functionName = func.substr(0, start_arg);
        Data *funcElement = lookUp(cmd, functionName, level, num_pass);
        string agrLs = funcElement->argL;
        int length_agrLs = funcElement->argL.length(), index = 0, count = 0;
        if (funcElement->type != "function")
        {
            throw(TypeMismatch(cmd));
        }

        for (int i = index + 1; i < lth; i++)
        {
            if (func[i] == ',')
            {
                count++;
            }
        }
        if (count == 0)
        {
            if (func[start_arg + 1] == ')')
                count = 0;
            else
                count = 1;
        }
        else
            count = count + 1;
        if (count != length_agrLs - 1)
        {
            throw(TypeMismatch(cmd));
        }
        end_arg = start_arg + 1;

        if (length_agrLs != 1)
        {
            while (end_arg != lth)
            {
                if (func[end_arg] == ')' || func[end_arg] == ',')
                {
                    string argstr = func.substr(start_arg + 1, end_arg - start_arg - 1);
                    if (index >= length_agrLs)
                    {
                        throw(TypeMismatch(cmd));
                    }
                    if (regex_match(argstr, regex("[a-z][a-zA-Z0-9_]*")))
                    {
                        Data *temp = lookUp(cmd, argstr, level, num_pass);
                        if (temp->type == "" && agrLs[index] == 'N')
                        {
                            throw(TypeCannotBeInferred(cmd));
                        }
                        else if (temp->type == "function")
                        {
                            throw(TypeMismatch(cmd));
                        }
                        else if (temp->type != "" && agrLs[index] == 'N')
                        {
                            funcElement->argL[index] = (temp->type == "number") ? '0' : '1';
                        }
                        else if (temp->type == "" && agrLs[index] != 'N')
                        {
                            temp->type = (agrLs[index] == '0') ? "number" : "string";
                        }
                        else if ((temp->type == "number" && agrLs[index] == '1') || (temp->type == "string" && agrLs[index] == '0'))
                        {
                            throw(TypeMismatch(cmd));
                        }
                    }
                    else if (regex_match(argstr, regex("[0-9]+")))
                    {
                        if (agrLs[index] == 'N')
                        {
                            funcElement->argL[index] = '0';
                        }
                        else if (agrLs[index] == '1')
                        {
                            throw(TypeMismatch(cmd));
                        }
                    }
                    else if (regex_match(argstr, regex("\'[a-zA-Z0-9 ]*\'")))
                    {
                        if (agrLs[index] == 'N')
                        {
                            funcElement->argL[index] = '1';
                        }
                        else if (agrLs[index] == '0')
                        {
                            throw(TypeMismatch(cmd));
                        }
                    }
                    start_arg = end_arg;
                    index++;
                }
                end_arg++;
            }
        }
        if (index != length_agrLs - 1)
        {
            throw(TypeMismatch(cmd));
        }
        Data *foundID = lookUp(cmd, ins, level, num_pass);
        if (funcElement->argL[length_agrLs - 1] == '2')
        {
            throw(TypeMismatch(cmd));
        }
        if (foundID->type == "function")
        {
            throw(TypeMismatch(cmd));
        }
        if (agrLs[length_agrLs - 1] == 'N' && foundID->type == "")
        {
            throw(TypeCannotBeInferred(cmd));
        }
        else if ((agrLs[length_agrLs - 1] == '0' && foundID->type == "string") || (agrLs[length_agrLs - 1] == '1' && foundID->type == "number"))
        {
            throw(TypeMismatch(cmd));
        }
        else if (agrLs[length_agrLs - 1] == 'N')
        {
            funcElement->argL[length_agrLs - 1] = (foundID->type == "string") ? '1' : '0';
        }
        else if (foundID->type == "")
        {
            foundID->type = (agrLs[length_agrLs - 1] == '0') ? "number" : "string";
        }

        cout << num_pass << endl;
        return;
    }

    void callFunction(string cmd, string func, int level)
    {
        int start_arg = 0, end_arg = 0, lth = func.length();
        int num_pass = 0;
        for (int i = 0; i < lth; i++)
        {
            if (func[i] == '(')
            {
                start_arg = i;
                break;
            }
        }
        string functionName = func.substr(0, start_arg);
        Data *funcElement = lookUp(cmd, functionName, level, num_pass);
        string agrLs = funcElement->argL;
        int length_agrLs = funcElement->argL.length(), index = 0, count = 0;
        if (funcElement->type != "function")
        {
            throw(TypeMismatch(cmd));
        }
        if (funcElement->argL[length_agrLs - 1] == 'N')
        {
            funcElement->argL[length_agrLs - 1] = '2';
        }
        if (funcElement->argL[length_agrLs - 1] != '2')
        {
            throw(TypeMismatch(cmd));
        }

        for (int i = index + 1; i < lth; i++)
        {
            if (func[i] == ',')
            {
                count++;
            }
        }
        if (count == 0)
        {
            if (func[start_arg + 1] == ')')
                count = 0;
            else
                count = 1;
        }
        else
            count = count + 1;

        if (count != length_agrLs - 1)
        {
            throw(TypeMismatch(cmd));
        }
        end_arg = start_arg + 1;

        if (length_agrLs != 1)
        {
            while (end_arg != lth)
            {
                if (func[end_arg] == ')' || func[end_arg] == ',')
                {
                    string argstr = func.substr(start_arg + 1, end_arg - start_arg - 1);
                    if (index >= length_agrLs)
                    {
                        throw(TypeMismatch(cmd));
                    }
                    if (regex_match(argstr, regex("[a-z][a-zA-Z0-9_]*")))
                    {
                        Data *temp = lookUp(cmd, argstr, level, num_pass);
                        if (temp->type == "" && agrLs[index] == 'N')
                        {
                            throw(TypeCannotBeInferred(cmd));
                        }
                        else if (temp->type == "function")
                        {
                            throw(TypeMismatch(cmd));
                        }
                        else if (temp->type != "" && agrLs[index] == 'N')
                        {
                            funcElement->argL[index] = (temp->type == "number") ? '0' : '1';
                        }
                        else if (temp->type == "" && agrLs[index] != 'N')
                        {
                            temp->type = (agrLs[index] == '0') ? "number" : "string";
                        }
                        else if ((temp->type == "number" && agrLs[index] == '1') || (temp->type == "string" && agrLs[index] == '0'))
                        {
                            throw(TypeMismatch(cmd));
                        }
                    }
                    else if (regex_match(argstr, regex("[0-9]+")))
                    {
                        if (agrLs[index] == 'N')
                        {
                            funcElement->argL[index] = '0';
                        }
                        else if (agrLs[index] == '1')
                        {
                            throw(TypeMismatch(cmd));
                        }
                    }
                    else if (regex_match(argstr, regex("\'[a-zA-Z0-9 ]*\'")))
                    {
                        if (agrLs[index] == 'N')
                        {
                            funcElement->argL[index] = '1';
                        }
                        else if (agrLs[index] == '0')
                        {
                            throw(TypeMismatch(cmd));
                        }
                    }
                    start_arg = end_arg;
                    index++;
                }
                end_arg++;
            }
        }
        if (index != length_agrLs - 1)
        {
            throw(TypeMismatch(cmd));
        }
        cout << num_pass << endl;
        return;
    }
    void deleteScope(string cmd, int level)
    {
        Node *temp = nodeL.head;
        while (temp && data[temp->position].level == level)
        {
            data[temp->position].ins = "";
            data[temp->position].type = "";
            data[temp->position].level = -1;
            size--;
            temp = temp->next;
            nodeL.pop();
        }
        return;
    }

    Data *lookUp(string cmd, string ins, int level, int &countPass)
    {
        int count;
        while (level > -1)
        {
            string key = encodeKey(ins, level);
            int i = 0;
            count = 0;
            while (i < m)
            {
                int posHash = hf(key, i, m, c1, c2);
                if (data[posHash].level == -2)
                    break;
                else if (data[posHash].ins == ins && data[posHash].level == level)
                {
                    countPass += count;
                    return (data + posHash);
                }
                count += 1;
                i++;
            }
            level--;
        }
        throw(Undeclared(ins));
        return NULL;
    }

    void print()
    {
        int count = 0;
        for (int i = 0; i < m; i++)
        {
            if (data[i].level >= 0 && count < size - 1)
            {
                count++;
                cout << i << " " << data[i].ins << "//" << data[i].level << ";";
            }
            else if (data[i].level >= 0)
            {
                cout << i << " " << data[i].ins << "//" << data[i].level << endl;
            }
        }
        return;
    }
};
int modString(string str, int m)
{
    int n = str.size();
    int parts = n % 7;
    long long res = (parts == 0) ? 0 : stol(str.substr(0, parts)) % m;
    for (; parts < n; parts += 7)
    {
        res = (res * 10000000 + stol(str.substr(parts, 7))) % m;
    }
    return res;
}

int hashLinear(string key, int i, int m, int c1, int c2)
{
    int temp = modString(key, m);
    long long temp1 = (long long)(temp + c1 * i);
    return temp1 % m;
}
int hashQuadratic(string key, int i, int m, int c1, int c2)
{
    int temp = modString(key, m);
    long long temp1 = (long long)(temp + c1 * i + c2 * i * i);
    return temp1 % m;
}
int hashDouble(string key, int i, int m, int c1, int c2)
{
    int h1 = modString(key, m);
    int h2 = 1 + modString(key, m - 2);
    long long temp = (long long)(h1 + c1 * i * h2);
    return temp % m;
}

#endif
