#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


struct Instruction
{
    char write = ' ';
    char move = 0;
    unsigned int state = INT_MAX - 3;
};


class Tape
{
    int head;
    int firstchunk;
    std::vector<char*> chunks;

    void FillTape()
    {
        while (firstchunk * 256 > head)
        {
            firstchunk--;
            chunks.insert(chunks.begin(), new char[256]);
            for (int i = 0; i < 256; i++)
            {
                chunks.front()[i] = '0';
            }
        }

        while (head >= (firstchunk + (int)chunks.size()) * 256)
        {
            chunks.push_back(new char[256]);
            for (int i = 0; i < 256; i++)
            {
                chunks.back()[i] = '0';
            }
        }
    }

public:
    Tape()
    {
        head = 0;
        firstchunk = 0;
    }

    Tape(std::string input, int start)
    {
        head = start;
        firstchunk = 0;

        if (input != "")
        {
            for (int i = 0; i < input.size(); i++)
            {
                if (i % 256 == 0)
                {
                    chunks.push_back(new char[256]);
                }

                chunks[i / 256][i % 256] = input[i];
            }

            for (int i = input.size(); i % 256 != 0; i++)
            {
                chunks.back()[i] = '0';
            }
        }
    }

    ~Tape()
    {
        for (char* i : chunks)
        {
            delete[] i;
        }
    }

    void MoveHead(int d)
    {
        head += d;
    }

    char Read()
    {
        FillTape();

        return chunks[std::floor(head / 256.f) - firstchunk][(head % 256 + 256) % 256];
    }

    void Write(char c)
    {
        FillTape();  // is this necessary?

        chunks[std::floor(head / 256.f) - firstchunk][(head % 256 + 256) % 256] = c;
    }

    std::string Print(bool isClean = true)
    {
        std::string output = "";

        for (char* i : chunks)
        {
            for (int j = 0; j < 256; j++)
            {
                output += i[j];
            }
        }
        if (isClean)
        {
            for (int i = 0; i < output.size(); i++)
                if (output[i] != '0')
                {
                    output = output.substr(i);
                    break;
                }
            for (int i = output.size() - 1; i >= 0; i--)
                if (output[i] != '0')
                {
                    output = output.substr(0, i + 1);
                    break;
                }
        }

        return output;
    }
};


int RunTM(Instruction* instructions, Tape& tape)
{
    int state = 0;

    while (state < INT_MAX - 3)
    {
        char currentSymbol = tape.Read();
        tape.Write(instructions[state * 256 + currentSymbol].write);
        tape.MoveHead(instructions[state * 256 + currentSymbol].move);

        if (instructions[state * 256 + currentSymbol].state == INT_MAX - 3)
        {
            std::cout << "Error at state " << state << ", symbol " << currentSymbol << ".\n";
        }

        state = instructions[state * 256 + currentSymbol].state;
    }

    return INT_MAX - state;
}


int main()
{
    std::cout << "Enter TM filename: ";
    std::string filename;
    std::cin >> filename;

    std::cout << "Reading...";
    std::ifstream t(filename);

    if (t.fail())
    {
        std::cout << "File read failed. Exiting...";
        return 0;
    }

    std::stringstream buffer;
    buffer << t.rdbuf();
    std::string string = buffer.str();

    std::vector<std::string> lines;
    int j = 0;
    for (int i = 0; i < string.size(); i++)
    {
        if (string[i] == '\n')
        {
            lines.push_back(string.substr(j, i - j));
            j = i + 1;
        }
    }
    lines.push_back(string.substr(j));
    std::string dict = lines.front();
    lines.erase(lines.begin());

    Instruction* instructions = new Instruction[256 * lines.size()]();

    for (int i = 0; i < dict.size(); i++)
    {
        if (dict[i] != ' ' and dict[i] != '|')
        {
            for (int j = 0; j < lines.size(); j++)
            {
                if (lines[j].size() <= i) continue;

                if (lines[j][i] != ' ' and lines[j][i] != '|')
                {
                    if (lines[j].substr(i, 4) == "halt" or lines[j].substr(i, 4) == "HALT")
                    {
                        instructions[256 * j + dict[i]].write = dict[i];
                        instructions[256 * j + dict[i]].move = lines[j][i + 1] == 0;
                        instructions[256 * j + dict[i]].state = INT_MAX;
                    }
                    else if (lines[j].substr(i, 4) == "TRUE" or lines[j].substr(i, 4) == "true")
                    {
                        instructions[256 * j + dict[i]].write = dict[i];
                        instructions[256 * j + dict[i]].move = lines[j][i + 1] == 0;
                        instructions[256 * j + dict[i]].state = INT_MAX - 1;
                    }
                    else if(lines[j].substr(i, 5) == "FALSE" or lines[j].substr(i, 5) == "false")
                    {
                        instructions[256 * j + dict[i]].write = dict[i];
                        instructions[256 * j + dict[i]].move = lines[j][i + 1] == 0;
                        instructions[256 * j + dict[i]].state = INT_MAX - 2;
                    }
                    else
                    {
                        instructions[256 * j + dict[i]].write = lines[j][i];
                        instructions[256 * j + dict[i]].move = lines[j][i + 1] == 'L' ? -1 : 1;
                        instructions[256 * j + dict[i]].state = std::stoi(lines[j].substr(i + 2));
                    }
                }
            }
        }
    }

    std::cout << "Done!\nHow would you like to run?\n c - Feed initialised to 0\n i - Feed initialised with text input\n f - Feed initialised with file input\n t - Feed initialised with tests\nEnter selection: ";
    std::cin >> filename;

    int head = 0;

    if (filename == "i")
    {
        std::cout << "Please enter the input: ";
        std::cin >> filename;
        std::cout << "Please enter the starting position of the head: ";
        std::cin >> head;
    }
    else if (filename == "f")
    {
        std::cout << "Please enter the input filename: ";
        std::cin >> filename;

        std::cout << "Reading...";
        std::ifstream file(filename);

        if (file.fail())
        {
            std::cout << "File read failed. Exiting...";
            return 0;
        }

        std::stringstream buffer2;
        buffer2 << file.rdbuf();
        filename = buffer2.str();

        std::cout << "Please enter the starting position of the head: ";
        std::cin >> head;
    }
    else if (filename == "t")
    {
        std::cout << "Please enter the test filename: ";
        std::cin >> filename;

        std::cout << "Reading...\n";
        std::ifstream file(filename);

        if (file.fail())
        {
            std::cout << "File read failed. Exiting...";
            return 0;
        }

        std::stringstream buffer2;
        buffer2 << file.rdbuf();
        filename = buffer2.str();
        if (filename[filename.size()-1] != '\n')
            filename = filename + '\n';

        while (filename.size())
        {
            Tape tape(filename.substr(0, filename.find('|')), 0);
            if (RunTM(instructions, tape) != 3 and tape.Print() == filename.substr(filename.find('|') + 1, filename.find('\n') - filename.find('|') - 1))
            {
                std::cout << "Test Passed!\n";
            }
            else
            {
                std::cout << "Test Failed!!! Output: " << tape.Print() << "\n";
            }
            filename = filename.substr(filename.find('\n') + 1);
        }
        return 0;
    }
    else
    {
        filename = "";
    }

    Tape tape(filename, head);

    std::cout << "Tape initialised. Running program...";

    switch (RunTM(instructions, tape))
    {
    case 3:
        std::cout << "Error: Turing Machine reached empty instruction. Exiting...\n";
        break;
    case 2:
        std::cout << "Turing Machine has exited FALSE.\n";
        break;
    case 1:
        std::cout << "Turing Machine has exited TRUE.\n";
        break;
    case 0:
        std::cout << "Turing Machine has exited.\n";
        break;
    }

    std::cout << "Feed:\n";
    std::cout << tape.Print();
    std::cout << "\n\n\nUntrimmed Feed:\n";
    std::cout << tape.Print(false);
    std::cout << std::endl;
}
