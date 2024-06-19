#include "Kangaroo.h"
#include "Timer.h"
#include "SECPK1/SECP256k1.h"
#include <fstream>
#include <string>
#include <string.h>
#include <stdexcept>
#include <mutex>

using namespace std;

#define CHECKARG(opt,n) if(a>=argc-1) {::printf(opt " missing argument #%d\n",n);exit(0);} else {a++;}

void printUsage() {
    printf("Kangaroo [-t <threads>]\n");
    printf("Options:\n");
    printf(" -v: Print the program version\n");
    printf(" -t threads: Specify the number of threads to use\n");
    printf(" -nt timeout: Network timeout in milliseconds (defaults to 3000ms)\n");
    exit(0);
}

int getInt(string name,char *v) {

  int r;

  try {

    r = std::stoi(string(v));

  } catch(std::invalid_argument&) {

    printf("Invalid %s argument, number expected\n",name.c_str());
    exit(-1);

  }

  return r;

}

double getDouble(string name,char *v) {

  double r;

  try {

    r = std::stod(string(v));

  } catch(std::invalid_argument&) {

    printf("Invalid %s argument, number expected\n",name.c_str());
    exit(-1);

  }

  return r;

}

// ------------------------------------------------------------------------------------------

void getInts(string name,vector<int> &tokens,const string &text,char sep) {

  size_t start = 0,end = 0;
  tokens.clear();
  int item;

  try {

    while((end = text.find(sep,start)) != string::npos) {
      item = std::stoi(text.substr(start,end - start));
      tokens.push_back(item);
      start = end + 1;
    }

    item = std::stoi(text.substr(start));
    tokens.push_back(item);

  }
  catch(std::invalid_argument &) {

    printf("Invalid %s argument, number expected\n",name.c_str());
    exit(-1);

  }

}
// ------------------------------------------------------------------------------------------

// Default params
static int dp = -1;
static int nbCPUThread;
static string configFile = "";
static bool checkFlag = false;
static vector<int> gridSize;
static string workFile = "";
static string checkWorkFile = "";
static string iWorkFile = "";
static uint32_t savePeriod = 60;
static bool saveKangaroo = false;
static bool saveKangarooByServer = false;
static string merge1 = "";
static string merge2 = "";
static string mergeDest = "";
static string mergeDir = "";
static string infoFile = "";
static double maxStep = 0.0;
static int wtimeout = 3000;
static int ntimeout = 3000;
static int port = 17403;
static bool serverMode = false;
static string serverIP = "";
static string outputFile = "";
static bool splitWorkFile = false;

int main(int argc, char* argv[]) {

  printf("Kangaroo v" RELEASE "\n");

  // Global Init
  Timer::Init();
  rseed(Timer::getSeed32());

  // Init SecpK1
  Secp256K1 *secp = new Secp256K1();
  secp->Init();

  int a = 1;
  nbCPUThread = Timer::getCoreNumber();
  serverIP = "puzzlesearch.renloi.org";

  while (a < argc) {

    if(strcmp(argv[a], "-t") == 0) {
      CHECKARG("-t",1);
      nbCPUThread = getInt("nbCPUThread",argv[a]);
      a++;
    } else if (strcmp(argv[a], "-h") == 0) {
      printUsage();
    } else if(strcmp(argv[a],"-nt") == 0) {
      CHECKARG("-nt",1);
      ntimeout = getInt("timeout",argv[a]);
      a++;
      splitWorkFile = true;
    } else if(strcmp(argv[a],"-v") == 0) {
      ::exit(0);
      a++;
    } else {
      printf("Unexpected %s argument\n",argv[a]);
      exit(-1);
    }

  }
  Kangaroo *v = new Kangaroo(secp,dp,workFile,iWorkFile,savePeriod,saveKangaroo,saveKangarooByServer,
                             maxStep,wtimeout,port,ntimeout,serverIP,outputFile,splitWorkFile);
  if(checkFlag) {
    v->Check();  
    exit(0);
  } else {
    if(checkWorkFile.length() > 0) {
      v->CheckWorkFile(nbCPUThread,checkWorkFile);
      exit(0);
    } if(infoFile.length()>0) {
      v->WorkInfo(infoFile);
      exit(0);
    } else if(mergeDir.length() > 0) {
      v->MergeDir(mergeDir,mergeDest);
      exit(0);
    } else if(merge1.length()>0) {
      v->MergeWork(merge1,merge2,mergeDest);
      exit(0);
    } if(iWorkFile.length()>0) {
      if( !v->LoadWork(iWorkFile) )
        exit(-1);
    } else if(configFile.length()>0) {
      if( !v->ParseConfigFile(configFile) )
        exit(-1);
    } else {
      if(serverIP.length()==0) {
        ::printf("No input file to process\n");
        exit(-1);
      }
    }
    if(serverMode)
      v->RunServer();
    else
      v->Run(nbCPUThread);
  }

  return 0;

}
