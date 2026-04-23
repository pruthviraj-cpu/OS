#include<iostream>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;

ifstream infile("input.txt");
ofstream outfile("output.txt");
char memory[100][4];
int SI;
int IC;
char IR[4];
int jobid, TTL, TLL;


void load();
void execute();
void mos();
void read();
void write();
void stop();



int main(){
    
    
    memset(memory,' ',sizeof(memory));

    load();
    infile.close();
    outfile.close();
    
}

void load(){//load from the input file to memory
    string line;
    int mptr=0;
    while(getline(infile,line)){
        if(line.substr(0,4)=="$AMJ"){
            jobid = stoi(line.substr(4,4));
            TTL   = stoi(line.substr(8,4));
            TLL   = stoi(line.substr(12,4));
            mptr = 0;
            memset(memory,' ',sizeof(memory));
        }else if(line.substr(0,4)=="$DTA"){
            execute();
        }else if(line.substr(0,4)=="$END"){
            continue;
        }else{
            for(int i=0;i<line.length();i++){
                memory[mptr][i%4]=line[i];
                if(i%4==3){
                    mptr++;
                }
            }
            if(line.length()%4!=0){
                mptr++;
            }
        }
    }
}

void execute(){//execute the iinstructions from the memory 
    IC = 0;

    while(true)
    {
        for(int i=0;i<4;i++){
            IR[i] = memory[IC][i];
        
        }
        IC++;
        if(IR[0]=='G' && IR[1]=='D')
        {
            SI = 1;
            mos();
        }else if(IR[0]=='P' && IR[1]=='D')
        {
            SI = 2;
            mos();
        }else if(IR[0]=='H')
        {
            SI = 3;
            mos();
            break;
        }
    }
}

void mos(){//call the read write stop

    if(SI == 1){
        read();
    }else if(SI == 2){
        write();
    }else if(SI == 3){
        stop();
    }
}

void read(){
    string line;
    getline(infile,line);

    int loc = (IR[2]-'0')*10 + (IR[3]-'0');//'gd10' ir[2]=1 , ir[3]=0
    int k=0;

    for(int i=loc;i<loc+10;i++)//taking the hello word and storing in memory
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

void write()
{
    int loc = (IR[2]-'0')*10 + (IR[3]-'0');//pd10

    for(int i=loc;i<loc+10;i++)//taking from the memory to output file
    {
        for(int j=0;j<4;j++)
            outfile << memory[i][j];
    }

    outfile << endl;
}



void stop(){
    cout<<"Program is Terminated";
}



