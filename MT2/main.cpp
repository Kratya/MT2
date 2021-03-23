#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;

struct VarTableElem
{
	VarTableElem(string name, string type, bool value) :name(name), value(value) {}
	VarTableElem() : name(""), value(0) {}
	string name;
	bool value;
};
struct place
{
	int i;
	int j;
	place(int i, int j) :i(i), j(j) {}
	place() :i(), j() {}
};
struct token
{
	int table;
	int i;
	int j;
	token() : table(0), i(0), j(0) {};
	token(int table, int i, int j) :table(table), i(i), j(j) {}
};

class ConstTable
{
public:
	vector<string> array;
	ConstTable(string fileName)
	{
		readTable(fileName);
	};
	~ConstTable()
	{
		array.~vector();
	}

	int findElem(string Name)
	{
		auto res = find(array.begin(), array.end(), Name);
		if (res == array.end())
			return -1;
		else
			return res - array.begin();
	}
	string getElem(int index)
	{
		return array[index];
	}


private:
	void readTable(string fileName)
	{
		string tmp;
		fstream in(fileName);
		while (!(in.eof()))
		{
			in >> tmp;
			array.push_back(tmp);
		}
		in.close();
	}
};

class TableVar
{
public:
	vector <vector<VarTableElem>> array;

	TableVar()
	{
		array.resize(52);
	};
	~TableVar()
	{
		array.~vector();
	}

	place findElem(string name)
	{
		place pl;
		auto hash = getHash(name);
		pl.i = hash;
		if (array[hash].size())
		{
			auto res = find_if(array[hash].begin(), array[hash].end(), [&](const VarTableElem & s)-> bool {return s.name == name; });
			if (res == array[hash].end())
			{
				array[hash].push_back(VarTableElem(name, "", 0));
				pl.j = array[hash].size() - 1;
			}

			else pl.j = res - array[hash].begin();
		}

		else
		{
			array[hash].push_back(VarTableElem(name, "", 0));
			pl.j = 0;
		}
		return pl;
	}
	void changeElem(place pl, bool value)
	{
		array[pl.i][pl.j].value = value;
	}
	VarTableElem getElem(place pl)
	{
		return array[pl.i][pl.j];
	}

private:
	int getHash(string h)
	{
		char len;
		if (h[0] >= 'A' && h[0] <= 'Z')
			len = h[0] - 'A';
		if (h[0] >= 'a' && h[0] <= 'z')
			len = h[0] - 'a' + 26;
		return (int)len;
	}
};

class TableInt
{
public:
	vector<vector<int>> array;

	TableInt()
	{
		array.resize(19);
	};
	~TableInt()
	{
		array.~vector();
	}

	place findElem(string value)
	{
		int tmp = stoi(value);
		place pl;
		auto hash = getHash(value);
		pl.i = hash;
		if (array[hash].size())
		{
			auto res = find(array[hash].begin(), array[hash].end(), tmp);
			if (res == array[hash].end())
			{
				array[hash].push_back(tmp);
				pl.j = array[hash].size() - 1;
			}

			else pl.j = res - array[hash].begin();
		}

		else
		{
			array[hash].push_back(tmp);
			pl.j = 0;
		}
		return pl;
	}
	void changeElem(place pl, int value)
	{
		array[pl.i][pl.j] = value;
	}
	int getElem(place pl) {
		return array[pl.i][pl.j];
	}

private:
	int getHash(string h)
	{
		int len;
		if (h[0] == '-')
			len = (int)h[1] - '0' + 9;
		else len = (int)h[0] - '0';
		return len;
	}
};

class Lexeme
{
public:
	ConstTable* keyWords = new ConstTable("KeyWords.txt");
	ConstTable* operations = new ConstTable("Operation.txt");
	ConstTable* delimiters = new ConstTable("Delimiter.txt");
	TableInt* tableInt = new TableInt();
	TableVar* tableVar = new TableVar();
	Lexeme(string fileName)
	{
		createTokens(fileName, "tokenFile.txt", "errorFile.txt");
	}

private:
	string str;
	int value;
	VarTableElem var;

	bool createTokens(string codeName, string tokenName, string errorName)
	{
		token tmp;
		ofstream tokenFile(tokenName), errorFile(errorName);
		ifstream code(codeName);
		char ch;
		string str;
		int flag;
		bool unexpected = true;
		place pl;
		if (code.peek() != EOF)
		{
			code.get(ch);
			while (1)
			{
				unexpected = true;
				if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z')
				{
					do
					{
						str += ch;
						if (code.peek() == EOF) { errorFile << "EOF"; return false; }
						code.get(ch);
					} while (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch >= '0' && ch <= '9' || ch == '_');

					flag = keyWords->findElem(str);
					if (flag != -1)
					{
						tmp.table = 0;
						tmp.i = flag;
						tmp.j = -1;
						tokenFile << tmp.table << " " << tmp.i << " " << tmp.j << endl;
						unexpected = false;
					}
					else
					{
						pl = tableVar->findElem(str);
						tmp.table = 4;
						tmp.i = pl.i;
						tmp.j = pl.j;
						tokenFile << tmp.table << " " << tmp.i << " " << tmp.j << endl;
						unexpected = false;
					}
					str.clear();
				}

				if (ch >= '0' && ch <= '9')
				{
					do
					{
						str += ch;
						if (code.peek() == EOF) { errorFile << "EOF";	return false; }
						code.get(ch);
						if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z')
						{
							errorFile << "error2: unexpected const";
							return false;
						}
					} while (ch >= '0' && ch <= '9');

					pl = tableInt->findElem(str);
					tmp.table = 3;
					tmp.i = pl.i;
					tmp.j = pl.j;
					tokenFile << tmp.table << " " << tmp.i << " " << tmp.j << endl;
					unexpected = false;
					str.clear();
				}

				if (ch == '=')
				{
					str = ch;
					if (code.peek() == EOF) { errorFile << "EOF";	return false; }
					code.get(ch);
					tmp.table = 1;
					tmp.i = operations->findElem(str);
					tmp.j = -1;
					tokenFile << tmp.table << " " << tmp.i << " " << tmp.j << endl;
					unexpected = false;
					str.clear();
					while (ch == ' ')
					{
						if (code.peek() == EOF) { errorFile << "EOF"; return false; }
						code.get(ch);
					}
					if (ch == '-' || ch == '+')
					{
						if (ch == '-') str = ch;
						if (code.peek() == EOF) { errorFile << "EOF"; return false; }
						code.get(ch);
					}
				}

				if (ch == '+' || ch == '-' || ch == '*' || ch == '!' || ch == '<' || ch == '>')
				{
					str += ch;
					if (code.peek() == EOF) { errorFile << "EOF";	return false; }
					code.get(ch);
					if (ch == '+' || ch == '-' || ch == '=')
					{
						str += ch;
						if (code.peek() == EOF) { errorFile << "EOF"; return false; }
						code.get(ch);
					}
					flag = operations->findElem(str);
					if (flag != -1)
					{
						tmp.table = 1;
						tmp.i = flag;
						tmp.j = -1;
						tokenFile << tmp.table << " " << tmp.i << " " << tmp.j << endl;
						unexpected = false;
					}
					else { errorFile << "error1"; return false; }
					str.clear();
				}

				if (ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ':' || ch == ';' || ch == ',' || ch == '.')
				{
					str += ch;
					flag = delimiters->findElem(str);
					if (flag != -1)
					{
						tmp.table = 2;
						tmp.i = flag;
						tmp.j = -1;
						tokenFile << tmp.table << " " << tmp.i << " " << tmp.j << endl;
						unexpected = false;
					}
					else { errorFile << "error1: unexpected symbol"; return false; }
					str.clear();
					if (code.peek() == EOF) { errorFile << "EOF";	return false; }
					code.get(ch);
				}

				if (ch == '/')
				{
					if (code.peek() == EOF) { errorFile << "error1: unexpected sym-bol";	return false; }
					code.get(ch);
					if (ch == '/')
						do
						{
							if (code.peek() == EOF) { errorFile << "EOF";	return false; }
							code.get(ch);
						} while (ch != '\n');
					else if (ch == '*')
					{
						char ch2 = ' ';
						do
						{
							if (code.peek() == EOF) { errorFile << "error3: unclosed comment";	return false; }
							code.get(ch);
							if (ch == '*')
							{
								if (code.peek() == EOF) { errorFile << "er-ror3: unclosed comment";	return false; }
								code.get(ch2);
							}
						} while (!(ch == '*' && ch2 == '/'));

					}
					else { errorFile << "error2"; return false; }
					unexpected = false;
					if (code.peek() == EOF) { errorFile << "EOF";	return false; }
					code.get(ch);
				}

				while (ch == '\n' || ch == '\t' || ch == ' ')
				{
					unexpected = false;
					if (code.peek() == EOF) { errorFile << "EOF";	return false; }
					code.get(ch);
				}

				if (unexpected)
				{
					errorFile << "error1: unexpected symbol"; return false;
				}
			}
		}
		tokenFile.close();
		errorFile.close();
		code.close();
	}
	void getElemByToken(token tmp)
	{
		switch (tmp.table)
		{
		case 0: str = keyWords->getElem(tmp.i); break;
		case 1: str = operations->getElem(tmp.i); break;
		case 2: str = delimiters->getElem(tmp.i); break;
		case 3: value = tableInt->getElem(place(tmp.i, tmp.j)); break;
		case 4: var = tableVar->getElem(place(tmp.i, tmp.j)); break;
		}
	}

};

void main()
{
	auto lexeme = new Lexeme("code.txt");
	system("pause");
}
