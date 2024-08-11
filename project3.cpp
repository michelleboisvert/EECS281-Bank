// Project Identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98

// EECS 281, Project 3 - 281Bank

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>
#include <deque>
#include <queue>   //for PQ
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <cstdlib>
//#include <cstring>
#include "xcode_redirect.hpp"
using namespace std;

class Transaction
{
  public:
    Transaction(uint64_t time, const string &sender, const string &recepient, uint64_t amount, uint64_t exec, const string &execdate, const string &feePayer, size_t transID)
    :placementTime(time), sender(sender), recepient(recepient), amount(amount), exectime(exec),  execdate(execdate), feePayer(feePayer), transID(transID) {
      fee = 0;
    }

    uint64_t getPlacementTime() const{
      return placementTime;
    }

    string getSender() {
      return sender;
    }
    
    string getRecepient() {
      return recepient;
    }

    uint64_t getAmount() const{
      return amount;
    }

    uint64_t getExecTime() const{
      return exectime;
    }

    string getExecDate() const{
      return execdate;
    }

    string getFeePayer() const{
      return feePayer;
    }

    size_t getTransID() const{
      return transID;
    }

    uint64_t getFee() const{
      return fee;
    }

    void setFee(uint64_t bankFee){
      fee = bankFee;
    }

  private:
    uint64_t placementTime;
    string sender;
    string recepient;
    uint64_t amount;
    uint64_t exectime;
    string execdate;
    string feePayer;
    size_t transID;
    uint64_t fee;
};

class TransactionCompare
{
  public:
  bool operator() (Transaction const &trans1, Transaction const &trans2){
    if(trans1.getExecTime() < trans2.getExecTime())
      return false;
    else if(trans1.getExecTime() == trans2.getExecTime()){
      if(trans1.getTransID() < trans2.getTransID())
        return false;
      return true;
    }
    return true;
  }
};

class User {
    public:
        User(uint64_t ts, string uID, string PIN, uint64_t balance)
          : timestamp(ts), userID(uID), myPin(PIN), balance(balance) {
          }

        User(){//default
          timestamp = 0;
          userID = "none";
          myPin = "12345";
          balance = 0;
        }//default

        uint64_t getStartTime() const{
          return timestamp;
        }

        string getUserID() const{
          return userID;
        }

        string getPin() const{
          return myPin;
        }

        uint64_t getBalance() const{
          return balance;
        }

        string getActiveSess() const{
          return activeUserSess;
        }

        void setActiveSess(const string &IP){
          activeUserSess = IP;
        }

        void addIP(const string &IPAddy){
          IPAddresses.insert(IPAddy);
        }

        void removeIP(const string &IPAddy){
          IPAddresses.erase(IPAddy);
          activeUserSess = "";
        }

        bool validIP(const string &IPAddy) const{
          if(IPAddresses.find(IPAddy) == IPAddresses.end())
            return false;
          return true;
        }

        bool isLoggedIn(){
          if(IPAddresses.size() >= 1)
            return true;
          return false;
        }

        void removeMoney(uint64_t amount){
          balance = balance - amount;
        }

        void addMoney(uint64_t amount){
          balance = balance + amount;
        }

        void addOutgoing(Transaction trans){
          outgoing.push_back(trans);
        }

        void addIncoming(Transaction trans){
          incoming.push_back(trans);
        }

        vector<Transaction> getOutgoing(){
          return outgoing;
        }
        
        vector<Transaction> getIncoming(){
          return incoming;
        }

    private:
        uint64_t timestamp;
        string userID;
        string myPin;
        uint64_t balance;
        string activeUserSess;
        unordered_set<string> IPAddresses;
        vector<Transaction> outgoing;
        vector<Transaction> incoming;
};

class Bank {
    public:
        Bank(bool verbose) 
          :isVerbose(verbose){
            numUsers = 0;
            numTransactions = 0;
        }

        size_t getNumUsers(){
          return numUsers;
        }

        void addUser(User newUser){
          myUsers[newUser.getUserID()] = newUser;
          numUsers++;
        }

        User* getUser(const string &uID){
          return &myUsers[uID];
        }

        /*bool userExists(const string &uID){
          if(myUsers.find(uID) == myUsers.end())
            return false;
          return true;
        }*/

        bool login(const string &uID, const string &potPin, string IP){
          User* tempUser = getUser(uID);
          if(potPin == tempUser->getPin()){
            tempUser->setActiveSess(IP);
            tempUser->addIP(IP);
            return true;
          }
          return false;
        }

        bool logout(const string &uID, string IP){
          User* tempUser = getUser(uID);
          if(tempUser->validIP(IP)){//IP is found
            tempUser->removeIP(IP);
            return true;
          }
          return false;
        }

        bool placeTransaction(string &timestamp, string &IP, string &amount, string &exec_date, const string &feePayer, const string &sName, const string &rName){
          //checks
          uint64_t threedays = 3000000;
          //timestamp = timestamp.substr(0,2) + timestamp.substr(3,2) + timestamp.substr(6,2) + timestamp.substr(9,2) + timestamp.substr(12,2) + timestamp.substr(15,2);
          //exec_date = exec_date.substr(0,2) + exec_date.substr(3,2) + exec_date.substr(6,2) + exec_date.substr(9,2) + exec_date.substr(12,2) + exec_date.substr(15,2);
          const char* exec = exec_date.c_str();
          const char* time = timestamp.c_str();
          uint64_t execnum = strtoull(exec, NULL, 10);
          uint64_t timenum = strtoull(time, NULL, 10);
          uint64_t difference = execnum - timenum;
          if(difference > threedays){
            if(isVerbose)
              cout << "Select a time less than three days in the future." << "\n";
            return false;
          }
          if(myUsers.find(sName) == myUsers.end()){
            if(isVerbose)
              cout << "Sender " << sName << " does not exist." << "\n";
            return false;
          }
          if(myUsers.find(rName) == myUsers.end()){
            if(isVerbose)
              cout << "Recipient " << rName << " does not exist." << "\n";
            return false;
          }
          User* sender = getUser(sName);
          User* recepient = getUser(rName);
          string senderName = sender->getUserID();
          string recepientName = recepient->getUserID();

          if(execnum < sender->getStartTime()){
            if(isVerbose)
              cout << "At the time of execution, sender and/or recipient have not registered." << "\n";
            return false;
          }
          if(execnum < recepient->getStartTime()){
            if(isVerbose)
              cout << "At the time of execution, sender and/or recipient have not registered." << "\n";
            return false;
          }
          if(!sender->isLoggedIn()){
            if(isVerbose)
              cout << "Sender " << senderName << " is not logged in." << "\n";
            return false;
          }
          if(!sender->validIP(IP)){
            if(isVerbose)
              cout << "Fraudulent transaction detected, aborting request." << "\n";
            return false;
          }


          executeTransaction(timestamp);

          //makeTransaction
          const char* amt = amount.c_str();
          uint64_t amtnum = strtoull(amt, NULL, 10);
          numTransactions++;
          Transaction trans = Transaction(timenum, senderName, recepientName, amtnum, execnum, exec_date, feePayer, numTransactions);
          myTransactions.push(trans);

          if(isVerbose)
            cout << "Transaction placed at " << timenum << ": $" << amount << " from " << sender->getUserID() << " to " << recepient->getUserID() << " at " << execnum << "." << "\n";
            
          return true;
        }

        bool hasTransactions(){
          if(myTransactions.size() > 0)
            return true;
          return false;
        }

        void executeTransaction(string &timestamp){
          while(!myTransactions.empty()){
            //timestamp = timestamp.substr(0,2) + timestamp.substr(3,2) + timestamp.substr(6,2) + timestamp.substr(9,2) + timestamp.substr(12,2) + timestamp.substr(15,2);
            const char* ctime = timestamp.c_str();
            uint64_t currentTime = strtoull(ctime, NULL, 10);
            Transaction temp = myTransactions.top();
            bool valid = true;
            if(temp.getExecTime() > currentTime)
              return;
            //fees o -- sender, s -- shared equally
            uint64_t fee = (temp.getAmount() * 1) / 100;
            if(fee < 10)
              fee = 10;
            else if(fee > 450)
              fee = 450;
            //discount
            uint64_t exectime = temp.getExecTime();
            User* sender = getUser(temp.getSender());
            User* recepient = getUser(temp.getRecepient());
            if((exectime - sender->getStartTime()) >= 50000000000)//5 years
              fee = (fee * 3) / 4;
            uint64_t senderFee = 0;
            uint64_t recepFee = 0;
            if(temp.getFeePayer() == "o"){//sender pays fee
              senderFee = fee;
              recepFee = 0;
            }
            else if(temp.getFeePayer() == "s"){//shared fee
              recepFee = fee / 2;
              senderFee = fee / 2;
              if(fee%2 != 0){//odd
                senderFee++;
              }
            }

            //checking if enough money
            if(sender->getBalance() < (senderFee + temp.getAmount())){
              if(isVerbose)
                cout << "Insufficient funds to process transaction " << (temp.getTransID() - 1) << "." << "\n";
              myTransactions.pop();
              valid = false;
            }
            else if(recepient->getBalance() < recepFee){
              if(isVerbose)
                cout << "Insufficient funds to process transaction " << (temp.getTransID() - 1) << "." << "\n";
              myTransactions.pop();
              valid = false;
            }
            if(valid){
              //making exchange
              sender->removeMoney(temp.getAmount() + senderFee);
              recepient->removeMoney(recepFee);
              recepient->addMoney(temp.getAmount());
              
              if(isVerbose){
                cout << "Transaction executed at " << temp.getExecTime() << ": $" << temp.getAmount() << " from " << sender->getUserID() << " to " << recepient->getUserID() << "." << "\n";
              }

              myTransactions.pop();
              temp.setFee(fee);
              queryList.push_back(temp);

              sender->addOutgoing(temp);
              recepient->addIncoming(temp);
            }
          }
        }

        void ListTransactions(string &startTime, string &endTime){
          string temptime1 = startTime.substr(0,2) + startTime.substr(3,2) + startTime.substr(6,2) + startTime.substr(9,2) + startTime.substr(12,2) + startTime.substr(15,2);
          const char* ctime1 = temptime1.c_str();
          uint64_t start = strtoull(ctime1, NULL, 10);

          string temptime2 = endTime.substr(0,2) + endTime.substr(3,2) + endTime.substr(6,2) + endTime.substr(9,2) + endTime.substr(12,2) + endTime.substr(15,2);
          const char* ctime2 = temptime2.c_str();
          uint64_t end = strtoull(ctime2, NULL, 10);

          int count = 0;
          for(size_t i = 0; i < queryList.size(); ++i){
            Transaction* temp = &queryList[i];
            uint64_t time = temp->getExecTime();
            if(start <= time && time < end){
              string d = "dollar";
              if(temp->getAmount() > 1 || temp->getAmount() == 0)
                d += 's';
              cout << (temp->getTransID() - 1) << ": " << temp->getSender() << " sent " << temp->getAmount() << " " << d << " to " << temp->getRecepient() << " at " << temp->getExecTime() << "." << '\n';
              count++;
            }
          }

          string t = "transaction";
          if(count > 1 || count == 0){
            t += 's';
            cout << "There were " << count <<  " " << t << " that were placed between time " << start << " to " << end << "." << '\n';
          }
          else {
            cout << "There was " << count <<  " " << t << " that was placed between time " << start << " to " << end << "." << '\n';
          }
        }

        uint64_t calcRevenue(uint64_t start, uint64_t end, bool isExec){
          uint64_t revenue = 0;
          for(size_t i = 0; i < queryList.size(); ++i){
            Transaction* temp = &queryList[i];
            uint64_t time = 0;
            if(isExec)
              time = temp->getExecTime();
            else
              time = temp->getPlacementTime();
            if(start <= time && time < end){
              revenue += temp->getFee();
            }
          }
          return revenue;
        }

        void BankRevenue(string &startTime, string &endTime){
          string temptime1 = startTime.substr(0,2) + startTime.substr(3,2) + startTime.substr(6,2) + startTime.substr(9,2) + startTime.substr(12,2) + startTime.substr(15,2);
          const char* ctime1 = temptime1.c_str();
          uint64_t start = strtoull(ctime1, NULL, 10);

          string temptime2 = endTime.substr(0,2) + endTime.substr(3,2) + endTime.substr(6,2) + endTime.substr(9,2) + endTime.substr(12,2) + endTime.substr(15,2);
          const char* ctime2 = temptime2.c_str();
          uint64_t end = strtoull(ctime2, NULL, 10);

          uint64_t revenue = calcRevenue(start, end, true);

          uint64_t time = end - start;
          string output = "";
          vector<string> times = {"second", "minute", "hour", "day", "month", "year"};
          size_t i = 0;
          while(time > 0){
            uint64_t num = time % 100;
            if(num > 1)
              output = to_string(num) + " " + times[i] + "s " + output;
            else if(num == 1)
              output = to_string(num) + " " + times[i] + " " + output;
            
            time /= 100;
            ++i;
          }
          if(start != end)
            output.pop_back();

          cout << "281Bank has collected " << revenue << " dollars in fees over " << output << "." << '\n';
        }

        void CustomerHistory(string &user){
          //user does not exist
          if(myUsers.find(user) == myUsers.end()){
            cout << "User " << user << " does not exist." << '\n';
            return;
          }

          User* thisUser = getUser(user);
          vector<Transaction> tempin = thisUser->getIncoming();
          vector<Transaction> tempout = thisUser->getOutgoing();

          cout << "Customer " << user << " account summary:" << '\n';
          cout << "Balance: $" << thisUser->getBalance() << '\n';
          cout << "Total # of transactions: " << (tempin.size() + tempout.size()) << '\n';
          size_t insize = tempin.size();
          cout << "Incoming " << insize << ":" << '\n';

          size_t start = 0;
          if(insize > 10)
            start = insize - 10;
          while(start < insize){
            Transaction* temp = &tempin[start];
            string d = "dollar";
            if(temp->getAmount() > 1 || temp->getAmount() == 0)
              d += "s";
            cout << (temp->getTransID() - 1) << ": " << temp->getSender() << " sent " << temp->getAmount() << " " << d << " to " << user << " at " << temp->getExecTime() << "." << '\n';
            start++;
          }

          size_t outsize = tempout.size();
          cout << "Outgoing " << outsize << ":" << '\n';
          start = 0;
          if(outsize > 10)
            start = outsize - 10;
          while(start < outsize){
            Transaction* temp = &tempout[start];
            string d = "dollar";
            if(temp->getAmount() > 1 || temp->getAmount() == 0)
              d += "s";
            cout << (temp->getTransID() - 1) << ": " << user << " sent " << temp->getAmount() << " " << d << " to " << temp->getRecepient() << " at " << temp->getExecTime() << "." << '\n';
            start++;
          }
        }

        void SummarizeDay(string timestamp){
          timestamp = timestamp.substr(0,2) + timestamp.substr(3,2) + timestamp.substr(6,2) + timestamp.substr(9,2) + timestamp.substr(12,2) + timestamp.substr(15,2);
          const char* ctime = timestamp.c_str();
          uint64_t time = strtoull(ctime, NULL, 10);
          uint64_t start = time - (time % 1000000);
          uint64_t end = time - (time % 1000000) + 1000000;

          cout << "Summary of [" << start << ", " << end << "):" << '\n';
          
          int count = 0;
          for(size_t i = 0; i < queryList.size(); ++i){
            Transaction* temp = &queryList[i];
            uint64_t time = temp->getExecTime();
            if(start <= time && time < end){
              string d = "dollar";
              if(temp->getAmount() > 1 || temp->getAmount() == 0)
                d += 's';
              cout << (temp->getTransID() - 1) << ": " << temp->getSender() << " sent " << temp->getAmount() << " " << d << " to " << temp->getRecepient() << " at " << temp->getExecTime() << "." << '\n';
              count++;
            }
          }
    
          string t = "";
          if(count > 1 || count == 0){
            t += "There were a total of " + to_string(count) + " transactions, ";
          }
          else {
            t += "There was a total of " + to_string(count) + " transaction, ";
          }

          uint64_t revenue = calcRevenue(start, end, true);

          t += "281Bank has collected " + to_string(revenue) + " dollars in fees.";

          cout << t << '\n';
        }

    private:
        unordered_map<string, User> myUsers;// key is user id, object is user
        size_t numUsers;
        bool isVerbose;
        size_t numTransactions;
        priority_queue<Transaction, vector<Transaction>, TransactionCompare> myTransactions;
        vector<Transaction> queryList;
};


void getMode(int argc, char * argv[], bool &isVerbose, string &filename) {
  // These are used with getopt_long()
  opterr = false; // Let us handle all error output for command line options
  int choice;
  int index = 0;
  option long_options[] = {
    { "help",  no_argument,       nullptr, 'h'  },
    { "file",  required_argument, nullptr, 'f'  },
    { "verbose", no_argument,     nullptr, 'v'},
    { nullptr, 0,                 nullptr, '\0' }
  };  // long_options[]

  while ((choice = getopt_long(argc, argv, "hf:v", long_options, &index)) != -1) {
    switch (choice) {
      case 'h':
        cout << "This program simulates EECS281 bank.\n";
        cout << "It takes in a registration file, follows commands, then outputs.\n";
        exit(0);
      case 'f':{
        //looking for filename and using options to set found one and saving the filename in string
        string arg{optarg};
        if(arg[arg.length() - 1] == 't')
            filename = arg;
        break;
      }// case 'f'
      case 'v':
        isVerbose = true;
        break;
      // case 'v'
      default:
        cerr << "Error: invalid option" << endl;
        exit(1);
      }  // switch ..choice
  } // while
}  // getMode()

int main(int argc, char* argv[]) {
    // This should be in all of your projects, speeds up I/O
    ios_base::sync_with_stdio(false);

    xcode_redirect(argc, argv);

    bool isVerbose = false;
    string fileName;
    getMode(argc, argv, isVerbose, fileName);
    if(fileName.empty()){
        cerr << "filename has not been specified" << endl;
        exit(1);
    }
    
    Bank myBank = Bank(isVerbose);

    //reading in reg file
    ifstream regfile(fileName, ifstream::in);
    if(regfile.good()){
        while(regfile){//still more to read
            string temptime;
            uint64_t time;
            string name;
            string pin;
            string tempnum;
            uint64_t balance;
            getline(regfile, temptime, '|');//time
            if(temptime.empty())
              break;
            temptime = temptime.substr(0,2) + temptime.substr(3,2) + temptime.substr(6,2) + temptime.substr(9,2) + temptime.substr(12,2) + temptime.substr(15,2);
            const char* ctime = temptime.c_str();
            time = strtoull(ctime, NULL, 10);
            getline(regfile, name, '|');//name
            getline(regfile, pin, '|');//pin
            getline(regfile, tempnum);//starting balance 
            const char* tempstring = tempnum.c_str();
            balance = strtoull(tempstring, NULL, 10);
            
            User tempUser = User(time, name, pin, balance);
            myBank.addUser(tempUser);
        }

    }
    else{
        cerr << "File could not be opened!" << endl;
        exit(1);
    }
    regfile.close();
  
  uint64_t prevPlaceTime = 0;
  int placed = 0;
  string currTime;
    
  //bank time!
  if(!cin.fail()){//stuff to read
    string junk;
    string temp;
    //operations
    cin >> temp;
    while(temp != "$$$"){
      switch(temp[0]){
        case '#':{
          getline(cin, junk);
          break;
        }//comment
        case 'l':{
          string uID;
          string pin;
          string IP;
          cin >> uID;
          cin >> pin;
          cin >> IP;
          bool success = myBank.login(uID, pin, IP);
          if(success){
            if(isVerbose)
              cout << "User " << uID << " logged in." << "\n";
          }
          else{
            if(isVerbose)
              cout << "Failed to log in " << uID << "." << "\n";
          }
          break;
        }//login
        case 'o':{
          string uID;
          string IP;
          cin >> uID;
          cin >> IP;
          bool success = myBank.logout(uID, IP); 
          if(success){
            if(isVerbose)
              cout << "User " << uID << " logged out." << "\n";
          }
          else{
            if(isVerbose)
              cout << "Failed to log out " << uID << "." << "\n";
          }
          break;
        }//out
        case 'p':{
          string timestamp;
          string IP;
          string sender;
          string recepient;
          string amount;
          string exec_date;
          string feePayer;
          cin >> timestamp;
          cin >> IP;
          cin >> sender;
          cin >> recepient;
          cin >> amount;
          cin >> exec_date;
          cin >> feePayer;
          timestamp = timestamp.substr(0,2) + timestamp.substr(3,2) + timestamp.substr(6,2) + timestamp.substr(9,2) + timestamp.substr(12,2) + timestamp.substr(15,2);
          exec_date = exec_date.substr(0,2) + exec_date.substr(3,2) + exec_date.substr(6,2) + exec_date.substr(9,2) + exec_date.substr(12,2) + exec_date.substr(15,2);
          const char* exec = exec_date.c_str();
          const char* time = timestamp.c_str();
          uint64_t execnum = strtoull(exec, NULL, 10);
          uint64_t timenum = strtoull(time, NULL, 10);
          //checking for errors
          if(prevPlaceTime > timenum && placed != 0){
            cerr << "Invalid decreasing timestamp in 'place' command." << endl;
            exit(1);
          }
          if(execnum < timenum){
            cerr << "You cannot have an execution date before the current timestamp." << endl;
            exit(1);
          }
          //myBank.executeTransaction(timestamp);
          bool validT = myBank.placeTransaction(timestamp, IP, amount, exec_date, feePayer, sender, recepient);
          if(validT){
            currTime = timestamp;
            prevPlaceTime = timenum;
            placed++;
          }
          break;
        }//place
      }
      //myBank.executeTransactions();
      cin >> temp;
    }
    while(myBank.hasTransactions()){
      string maxTime = "999999999999";
      myBank.executeTransaction(maxTime);
    }
    //$$$
    cin >> temp;
    //query
    while(!cin.fail()){//theres more to read in commands file
      switch(temp[0]){
        case 'l':{
          string starttime;
          string endtime;
          cin >> starttime;
          cin >> endtime;
          myBank.ListTransactions(starttime, endtime);
          break;
        }//list transactions (make sure to do 0-indexed!!)
        case 'r':{
          string starttime;
          string endtime;
          cin >> starttime;
          cin >> endtime;
          myBank.BankRevenue(starttime, endtime);
          break;
        }//bank revenus
        case 'h':{
          string user;
          cin >> user;
          myBank.CustomerHistory(user);
          break;
        }//customer history
        case 's':{
          string day;
          cin >> day;
          myBank.SummarizeDay(day);
          break;
        }//summarize day
      }
      cin >> temp;
    }
  }
    return 0;
}   // main()
