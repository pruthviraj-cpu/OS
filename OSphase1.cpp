#include<iostream>
#include<fstream>
#include<string>
#include<cstring>

using namespace std;

ifstream infile("input2.txt");
ofstream outfile("output2.txt");


char memory[100][4];



char IR[4];     
char R[4];      

int IC;         
int SI;         

bool C = false; 


int jobid, TTL, TLL;


void load();
void execute();
void mos();

void read();
void write();
void stop();

int addressMap();


int main()
{
    memset(memory,' ',sizeof(memory));

    load();

    infile.close();
    outfile.close();

    return 0;
}


void load()
{
    string line;

    int mptr = 0;

    while(getline(infile,line))
    {

        if(line.substr(0,4) == "$AMJ")
        {
            jobid = stoi(line.substr(4,4));
            TTL   = stoi(line.substr(8,4));
            TLL   = stoi(line.substr(12,4));

            mptr = 0;

            memset(memory,' ',sizeof(memory));

            memset(IR,' ',sizeof(IR));
            memset(R,' ',sizeof(R));

            IC = 0;
            C = false;
        }

        else if(line.substr(0,4) == "$DTA")
        {
            execute();
        }


        else if(line.substr(0,4) == "$END")
        {
            continue;
        }

        else
        {
                    int i = 0;

            while(i < line.length())
            {

                if(line[i] == 'H')
                {
                    memory[mptr][0] = 'H';
                    memory[mptr][1] = ' ';
                    memory[mptr][2] = ' ';
                    memory[mptr][3] = ' ';

                    mptr++;
                    i++;
                }

                else
                {
                    for(int j=0;j<4 && i<line.length();j++)
                    {
                        memory[mptr][j] = line[i];
                        i++;
                    }

                    mptr++;
                }
            }
        }
    }
}


int addressMap()//converting loc or address from str to int
{
    return (IR[2]-'0')*10 + (IR[3]-'0');
}


void execute()
{
    IC = 0;

    while(true)
    {

        for(int i=0;i<4;i++)
        {
            IR[i] = memory[IC][i];
        }

        cout << "\n---------------------------------" << endl;

        cout << "IC : " << IC << endl;

        cout << "IR : ";

        for(int i=0;i<4;i++)
        {
            cout << IR[i];
        }

        cout << endl;

        IC++;

        int loc = addressMap();

        if(IR[0]=='G' && IR[1]=='D')//get data
        {
            cout << "Instruction : GD (Get Data)" << endl;
            cout << "Reading data into memory location : " << loc << endl;

            SI = 1;
            mos();
        }

        else if(IR[0]=='P' && IR[1]=='D')//put data
        {
            cout << "Instruction : PD (Put Data)" << endl;
            cout << "Writing data from memory location : " << loc << endl;

            SI = 2;
            mos();
        }

        else if(IR[0]=='H')//halt
        {
            cout << "Instruction : H (Halt)" << endl;

            SI = 3;
            mos();
            break;
        }

        else if(IR[0]=='L' && IR[1]=='R')//load reg -> from the given address in memory load the data to R 
        {
            cout << "Instruction : LR (Load Register)" << endl;
            cout << "Loading from memory[" << loc << "] into Register R" << endl;

            for(int i=0;i<4;i++)
            {
                R[i] = memory[loc][i];
            }

            cout << "Register R : ";

            for(int i=0;i<4;i++)
            {
                cout << R[i];
            }

            cout << endl;
        }

        else if(IR[0]=='S' && IR[1]=='R')//store reg-> store data from the R to given memory loc
        {
            cout << "Instruction : SR (Store Register)" << endl;
            cout << "Storing Register R into memory[" << loc << "]" << endl;

            for(int i=0;i<4;i++)
            {
                memory[loc][i] = R[i];
            }
        }


        else if(IR[0]=='C' && IR[1]=='R')//compare reg-> compare the R and given the memory loc 
        {
            cout << "Instruction : CR (Compare Register)" << endl;
            cout << "Comparing Register R with memory[" << loc << "]" << endl;

            C = true;

            for(int i=0;i<4;i++)
            {
                if(R[i] != memory[loc][i])
                {
                    C = false;
                    break;
                }
            }

            if(C == true)
            {
                cout << "Comparison Result : TRUE" << endl;
            }
            else
            {
                cout << "Comparison Result : FALSE" << endl;
            }
        }


        else if(IR[0]=='B' && IR[1]=='T')//branch true -> if true then jump to given addres 
        {
            cout << "Instruction : BT (Branch True)" << endl;

            if(C == true)
            {
                cout << "C flag TRUE -> Jumping to address : " << loc << endl;

                IC = loc;
            }
            else
            {
                cout << "C flag FALSE -> No Jump" << endl;
            }
        }
    }
}


void mos()//master mode for kernel 
{
    if(SI == 1)
    {
        read();
    }

    else if(SI == 2)
    {
        write();
    }

    else if(SI == 3)
    {
        stop();
    }
}



void read()//read the data from the job card and store in memory
{
    string line;

    getline(infile,line);

    cout << "\nData Read : " << line << endl;

    int loc = addressMap();

    int k = 0;

    for(int i=loc;i<loc+10;i++)
    {
        for(int j=0;j<4;j++)
        {
            if(k < line.length())
                memory[i][j] = line[k++];
            else
                memory[i][j] = ' ';
        }
    }
}


void write()//writing code in output file
{
    int loc = addressMap();

    cout << "\nOutput Written : ";

    for(int i=loc;i<loc+10;i++)
    {
        for(int j=0;j<4;j++)
        {
            outfile << memory[i][j];
            cout << memory[i][j];
        }
    }

    cout << endl;

    outfile << endl;
}


void stop()//terminating program
{
    outfile << "\n\n";

    cout << "Program Terminated" << endl;
}