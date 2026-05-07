// OS PHASE 2 SIMULATOR - FIXED VERSION

#include<iostream>
#include<fstream>
#include<string>
#include<cstring>
#include<cstdlib>
#include<ctime>

using namespace std;

ifstream infile("p2input.txt");
ofstream outfile("p2output.txt");

char memory[300][4];

char IR[4];        // instruction register
char R[4];         // general purpose register

int IC;            // instruction counter
int SI;            // system interrupt
int PI;            // program interrupt
int TI;            // time interrupt

bool C = false;    // toggle register

int jobid;
int TTL;           // total time limit
int TLL;           // total line limit

int TTC = 0;       // total time counter
int TLC = 0;       // total line counter

int PTR;           // page table register

int usedFrames[30];
int usedIndex = -1;

// EM error codes:
// 0 = No Error
// 1 = Out of Data
// 2 = Line Limit Exceeded
// 3 = Time Limit Exceeded
// 4 = Operation Code Error
// 5 = Operand Error
// 6 = Invalid Page Fault

void init();
void load();
void execute();
void mos();

void read();
void write();
void halt();
void terminate(int em);         

int allocate();
int addressMap(int VA);
bool frameUsed(int frame);

void printMemory();           

int main()
{
    srand(time(0));

   
    memset(memory, ' ', sizeof(memory));

    load();

    infile.close();
    outfile.close();

    return 0;
}


void init()
{
    // Clear ALL memory between jobs
    memset(memory, ' ', sizeof(memory));

    memset(IR, ' ', sizeof(IR));
    memset(R,  ' ', sizeof(R));

    IC = 0;
    SI = 0;
    PI = 0;
    TI = 0;

    TTC = 0;
    TLC = 0;

    C = false;


    usedIndex = -1;
    memset(usedFrames, -1, sizeof(usedFrames));
}

bool frameUsed(int frame)
{
    for(int i = 0; i <= usedIndex; i++)
    {
        if(usedFrames[i] == frame)
            return true;
    }
    return false;
}

int allocate()
{
    int frame;
    while(true)
    {
        frame = rand() % 30;
        if(!frameUsed(frame))
        {
            usedIndex++;
            usedFrames[usedIndex] = frame;
            return frame;
        }
    }
}

void load()
{
    string line;
    int pageTableIndex = 0;

    while(getline(infile, line))
    {
        if(line.substr(0, 4) == "$AMJ")
        {
            init();

            jobid = stoi(line.substr(4, 4));
            TTL   = stoi(line.substr(8, 4));
            TLL   = stoi(line.substr(12, 4));

            PTR = allocate() * 10;

            cout << "\n====================================" << endl;
            cout << "JOB ID : " << jobid << endl;
            cout << "TTL    : " << TTL << endl;
            cout << "TLL    : " << TLL << endl;
            cout << "PTR    : " << PTR << endl;


            for(int i = PTR; i < PTR + 10; i++)
            {
                memory[i][0] = '0';   // valid bit = 0 (not loaded)
                memory[i][1] = ' ';
                memory[i][2] = '*';
                memory[i][3] = '*';
            }

            pageTableIndex = 0;
        }

        else if(line.substr(0, 4) == "$DTA")
        {
            execute();
        }

        else if(line.substr(0, 4) == "$END")
        {
            cout << "JOB ENDED" << endl;
        }

        else
        {
            int frame = allocate();
            int RA = frame * 10;


            memory[PTR + pageTableIndex][0] = '1';
            memory[PTR + pageTableIndex][2] = frame / 10 + '0';
            memory[PTR + pageTableIndex][3] = frame % 10 + '0';

            cout << "\nProgram Page " << pageTableIndex
                 << " allocated to Frame " << frame << endl;

            pageTableIndex++;

            int k = 0;
            int word = 0;

            while(k < (int)line.length())
            {
                if(line[k] == 'H')
                {
                    memory[RA + word][0] = 'H';
                    memory[RA + word][1] = ' ';
                    memory[RA + word][2] = ' ';
                    memory[RA + word][3] = ' ';
                    word++;
                    k++;
                }
                else
                {
                    for(int j = 0; j < 4 && k < (int)line.length(); j++)
                        memory[RA + word][j] = line[k++];
                    word++;
                }
            }
        }
    }
}


int addressMap(int VA)
{
    int page   = VA / 10;
    int offset = VA % 10;

    int pageTableEntry = PTR + page;

    if(memory[pageTableEntry][0] == '0')
    {
        PI = 3;   // page fault
        return -1;
    }

    // Check frame digits are valid numbers
    char d1 = memory[pageTableEntry][2];
    char d2 = memory[pageTableEntry][3];
    if(d1 < '0' || d1 > '9' || d2 < '0' || d2 > '9')
    {
        PI = 2;   
        return -1;
    }

    int frame = (d1 - '0') * 10 + (d2 - '0');
    int RA    = frame * 10 + offset;
    return RA;
}

bool operandValid()
{
    return (IR[2] >= '0' && IR[2] <= '9' &&
            IR[3] >= '0' && IR[3] <= '9');
}

void execute()
{
    IC = 0;

    while(true)
    {
        // --- FETCH PHASE ---
        PI = 0;
        int RA = addressMap(IC);

        if(PI == 3)
        {
            // Invalid page fault on fetch (IC page not in memory)
            terminate(6);
            return;
        }

        for(int i = 0; i < 4; i++)
            IR[i] = memory[RA][i];

        cout << "\n----------------------------------" << endl;
        cout << "IC : " << IC << endl;
        cout << "IR : " << IR[0] << IR[1] << IR[2] << IR[3] << endl;

        IC++;
        TTC++;

        // Check time limit
        if(TTC > TTL)
        {
            TI = 2;
        }

        int operand = -1;
        if(operandValid())
            operand = (IR[2] - '0') * 10 + (IR[3] - '0');

        if(IR[0] == 'G' && IR[1] == 'D')
        {
            cout << "Instruction : GD" << endl;

            if(!operandValid())
            {
                PI = 2;
            }
            else
            {
                int page  = operand / 10;
                int entry = PTR + page;

                if(memory[entry][0] == '0')
                {
                    int newFrame = allocate();
                    memory[entry][0] = '1';
                    memory[entry][2] = newFrame / 10 + '0';
                    memory[entry][3] = newFrame % 10 + '0';
                    cout << "VALID PAGE FAULT -> Frame " << newFrame << " allocated" << endl;
                }

                SI = 1;
            }
        }

        else if(IR[0] == 'P' && IR[1] == 'D')
        {
            cout << "Instruction : PD" << endl;

            if(!operandValid())
            {
                PI = 2;
            }
            else
            {
                SI = 2;
                TLC++;
            }
        }

        else if(IR[0] == 'H')
        {
            cout << "Instruction : H" << endl;
            SI = 3;
        }

        else if(IR[0] == 'L' && IR[1] == 'R')
        {
            cout << "Instruction : LR" << endl;

            if(!operandValid()) { PI = 2; }
            else
            {
                PI = 0;
                int realAddress = addressMap(operand);
                if(PI == 0)
                {
                    for(int i = 0; i < 4; i++)
                        R[i] = memory[realAddress][i];
                    cout << "Register Loaded : " << R[0]<<R[1]<<R[2]<<R[3] << endl;
                }
            }
        }

        else if(IR[0] == 'S' && IR[1] == 'R')
        {
            cout << "Instruction : SR" << endl;

            if(!operandValid()) { PI = 2; }
            else
            {
                int page  = operand / 10;
                int entry = PTR + page;

                if(memory[entry][0] == '0')
                {
                    int newFrame = allocate();
                    memory[entry][0] = '1';
                    memory[entry][2] = newFrame / 10 + '0';
                    memory[entry][3] = newFrame % 10 + '0';
                    cout << "VALID PAGE FAULT -> Frame " << newFrame << " allocated" << endl;
                }

                PI = 0;
                int realAddress = addressMap(operand);
                if(PI == 0)
                {
                    for(int i = 0; i < 4; i++)
                        memory[realAddress][i] = R[i];
                    cout << "Register Stored" << endl;
                }
            }
        }

        else if(IR[0] == 'C' && IR[1] == 'R')
        {
            cout << "Instruction : CR" << endl;

            if(!operandValid()) { PI = 2; }
            else
            {
                PI = 0;
                int realAddress = addressMap(operand);
                if(PI == 0)
                {
                    C = true;
                    for(int i = 0; i < 4; i++)
                    {
                        if(R[i] != memory[realAddress][i])
                         { C = false; break;
                         }
                    }
                    cout << (C ? "Comparison TRUE" : "Comparison FALSE") << endl;
                }
            }
        }

        else if(IR[0] == 'B' && IR[1] == 'T')
        {
            cout << "Instruction : BT" << endl;

            if(!operandValid()) { PI = 2; }
            else if(C == true)
            {
                IC = operand;
                cout << "Branch Taken to " << IC << endl;
            }
            else
            {
                cout << "Branch Not Taken" << endl;
            }
        }

        else
        {
            PI = 1;   // operation code error
            cout << "INVALID OPCODE" << endl;
        }


        if(TI == 2)
        {
            // Time limit exceeded — check PI/SI for combined errors
            if(SI != 0 || PI != 0)
            {
                // Handle write before terminate if SI==2
                if(SI == 2 && PI == 0)
                {
                    mos();          // write
                    terminate(3);   // then time limit
                    return;
                }
                // Combined errors
                if(PI == 1)       { terminate(34); return; }
                if(PI == 2)       { terminate(35); return; }
                terminate(3);
            }
            else
            {
                terminate(3);
            }
            return;
        }

        if(PI != 0)
        {
            if(PI == 1) { terminate(4); return; }
            if(PI == 2) { terminate(5); return; }
            if(PI == 3)
            {
                // Page fault during execute — invalid (program page not loaded at load time)
                terminate(6);
                return;
            }
        }

        if(SI != 0)
        {
            mos();
            if(SI == 3) return;    // halt ends execution
            SI = 0;
        }

        // Line limit check (after mos wrote)
        if(TLC > TLL)
        {
            terminate(2);
            return;
        }
    }
}

void mos()
{
    if(SI == 1) read();
    else if(SI == 2) write();
    else if(SI == 3) halt();
}

void read()
{
    string line;

   
    if(!getline(infile, line) || line.substr(0, 4) == "$END")
    {
        
        terminate(1);
       
        return;
    }

    int operand = (IR[2] - '0') * 10 + (IR[3] - '0');
    int RA = addressMap(operand);

    int k = 0;
    for(int i = RA; i < RA + 10; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            memory[i][j] = (k < (int)line.length()) ? line[k++] : ' ';
        }
    }

    cout << "Data Read : " << line << endl;
}

void write()
{
    int operand = (IR[2] - '0') * 10 + (IR[3] - '0');
    int RA = addressMap(operand);

    cout << "Output : ";

    for(int i = RA; i < RA + 10; i++)
    {
        bool empty = true;
        for(int j = 0; j < 4; j++)
            if(memory[i][j] != ' ') { empty = false; break; }
        if(empty) break;

        for(int j = 0; j < 4; j++)
            outfile << memory[i][j];
    }

    outfile << "\n";
    cout << endl;
}

void halt()
{
    cout << "PROGRAM HALTED" << endl;
    terminate(0);
}

void terminate(int em)
{
    outfile << "\n\n";   
    outfile << "JOB ID : " << jobid << "\n";

    switch(em)
    {
        case 0:
            outfile << "NO ERROR\n";
            break;
        case 1:
            outfile << "OUT OF DATA ERROR\n";
            break;
        case 2:
            outfile << "LINE LIMIT EXCEED ERROR\n";
            break;
        case 3:
            outfile << "TIME LIMIT EXCEED ERROR\n";
            break;
        case 4:
            outfile << "OPCODE ERROR\n";
            break;
        case 5:
            outfile << "OPERAND ERROR\n";
            break;
        case 6:
            outfile << "INVALID PAGE FAULT\n";
            break;
        case 34:
            outfile << "TIME LIMIT EXCEED AND OPCODE ERROR\n";
            break;
        case 35:
            outfile << "TIME LIMIT EXCEED AND OPERAND ERROR\n";
            break;
        default:
            outfile << "UNKNOWN ERROR\n";
    }

    outfile << "IC : " << IC << "\n";
    outfile << "IR : " << IR[0]<<IR[1]<<IR[2]<<IR[3] << "\n";
    outfile << "TTC : " << TTC << "\n";
    outfile << "LLC : " << TLC << "\n";

    // printMemory();

    cout << "\nPROGRAM TERMINATED (EM=" << em << ")\n";
    printMemory();
    
}


void printMemory()
{
    cout << "\n+-------+----------+" << endl;
    cout << "| ADDR  |  CONTENT |" << endl;
    cout << "+-------+----------+" << endl;
    // outfile << "\n+-------+----------+" << endl;
    // outfile << "| ADDR  |  CONTENT |" << endl;
    // outfile << "+-------+----------+" << endl;

    for(int i = 0; i < 300; i++)
    {
        bool empty = true;
        for(int j = 0; j < 4; j++)
            if(memory[i][j] != ' ') { empty = false; break; }

        cout << "| ";
        // outfile << "| ";
        
        if(i < 10)        cout << "  " << i;
        else if(i < 100)  cout << " "  << i;
        else              cout << i;

        cout << "   |  ";
        // cout << "   |  ";
        
        if(empty)
        {
            cout << "[    ]  |" << endl;
        }
        else
        {
            cout<<R[0]<<R[1]<<R[2]<<R[3];
            cout << memory[i][0] << memory[i][1]
                 << memory[i][2] << memory[i][3]
                 << "    |" << endl;
        }
    }

    cout << "+-------+----------+" << endl;
}