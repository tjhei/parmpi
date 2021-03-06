#include <iostream>
#include <vector>
#include <mpi.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

inline std::string to_string(int i)
{
  std::stringstream sstm;
  sstm << i;
  return sstm.str();
}

int main(int argc, char * argv[])
{
  MPI_Init(&argc, &argv); 
  
  int myid, numprocs;
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  
  std::vector<string> commands;
  unsigned int idx_shipped = 0;
  unsigned int done = 0;
  
  string cmd;

  if (isatty(fileno(stdin)))
  {
    std::cerr << "usage: mpirun -n 3 parmpi < list_of_commands.txt" << endl;
    return 0;
  }
  
  while (!cin.eof())
  {
    getline ( cin, cmd );
    commands.push_back(cmd);
  }
      
  if (numprocs==1)
  {
    std::cerr << "[parmpi]: need more than one proc!" << std::endl;
    return -1;
  }
  
  if (commands.size()==0)
    return 0;
    
  if (myid==0)
  {
    while (idx_shipped < commands.size())
    {
      int buf[1]={0};
      MPI_Status status;
      
      MPI_Recv(buf, 1, MPI_INT, MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, &status);
      
      done += buf[0];
      
      if (done)
	std::cout << "[parmpi]: done with " << done << " / " << commands.size() << endl;
      
      MPI_Send((char*)commands[idx_shipped].c_str(), 
	       commands[idx_shipped].size()+1, 
	       MPI_CHAR, 
	       status.MPI_SOURCE, 
	       1000, 
	       MPI_COMM_WORLD);
      
      idx_shipped++;
    }
    
    //cleanup phase
    for (int i=1;i<numprocs;++i)
    {
      int buf[1]={0};
      MPI_Status status;
      
      MPI_Recv(buf, 1, MPI_INT, MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, &status);
      done += buf[0];
      if (done)
	std::cout << "[parmpi]: done with " << done << " / " << commands.size() << endl;
      MPI_Send(buf, 
	       1, 
	       MPI_INT, 
	       status.MPI_SOURCE, 
	       1001, 
	       MPI_COMM_WORLD);
    }
  }
  else
  {
    unsigned int n_done = 0;
    while (true)
    {
      int buf[1]={0};
      if (n_done>0)
	buf[0]=1;
      MPI_Send(buf, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
      
      char command[4096];
      MPI_Status status;
      MPI_Recv(command, 4096, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (status.MPI_TAG == 1000)
      {
	command[4095]=0;
	system(command);
	n_done++;
      }
      else
      {
	break;
      }
    }
  }
  
  MPI_Finalize(); 
  return 0;
}

