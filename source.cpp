#include <cstdio>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>

using namespace std;

// @author: afgedemenli

const string USER = "USER";
const string PROMPTER = " >>> ";
//
const string COMMAND_EXIT = "exit";
const string COMMAND_LISTDIR = "listdir";
const string COMMAND_LISTDIR_A = "listdir -a";
const string COMMAND_CURRENTPATH = "currentpath";
const string COMMAND_FOOTPRINT = "footprint";
const string COMMAND_PRINTFILE = "printfile";
const string COMMAND_INVALID = "Invlaid command";

int main(){
	// Variables needed
	string username = getenv(USER.c_str());
	string input;
	vector <string> footprint;
	//
	while(1){
		// Greeting with the username
		cout << username << PROMPTER;
		// Getting the input
		getline(cin, input);
		string original_input = input;
		// Make input clear. Get rid of unnecessary spaces at the beginning
		while(input.length()>0 && input.at(0) == ' ') {
			input = input.substr(1);
		}
		// Make input clear. Get rid of unnecessary spaces at the end
		while(input.length()>0 && input.at(input.length()-1) == ' ') {
			input = input.substr(0, input.length()-1);
		}
		// Make input clear. Get rid of unnecessary spaces in the middle
		for(int i = 1; i<input.length(); i++) {
			if((input.at(i)==' ' || input.at(i)=='|' || input.at(i)=='>') && input.at(i-1)==' ') {
				input = input.substr(0,i-1) + input.substr(i);
				i--;
			}
			else if(input.at(i)==' ' && (input.at(i-1)=='|' || input.at(i-1)=='>')) {
				input = input.substr(0, i) + input.substr(i+1);
				i--;
			}
		}
		// Check input
		int cntgreater = 0;
		int cntbar = 0;
		for(int i = 0; i<input.size(); i++) {
			if(input.at(i) == '>'){
				cntgreater++;
			}
			else if(input.at(i) == '|'){
				cntbar++;
			}
		}
		if(cntbar > 1 || cntgreater > 1) {
			cout << COMMAND_INVALID << endl;
			continue;
		}
		// Below here, we find the exact command the user wants and execute it.
		// If it is invalid, then print invalid.
		// All commands are implemented with a small explanation comment
		// Meanwhile, push all commands into a vector in order to run "footprint" properly.
		//
		// If user enters "exit", our shell terminates
		if(input == COMMAND_EXIT) {
			break;
		}
		// If the command is "listdir", run ls command with fork()
		// THIS DOES NOT HANDLE GREP&PIPE
		else if(input == COMMAND_LISTDIR) {
			footprint.push_back(original_input);	
			if(fork()==0) {
				execlp("ls", "-l", NULL);
			}
			else {
				wait(0);
			}
		}
		// If the command is "listdir -a", run ls -a command with fork()
		// THIS DOES NOT HANDLE GREP&PIPE
		else if(input == COMMAND_LISTDIR_A) {
			footprint.push_back(original_input); 
			if(fork()==0){
				execl("/bin/ls", "ls", "-a", NULL);
			}
			else {
				wait(0);
			}
		}
		// If the command is "currentpath", run pwd command with fork()
		else if(input == COMMAND_CURRENTPATH) {
			footprint.push_back(original_input);
			if(fork()==0) {
				execlp("pwd", "pwd", NULL);
			}
			else {
				wait(0);
			}
		}
		// If the command is "footprint", print last 15 commands
		else if(input == COMMAND_FOOTPRINT) {
			footprint.push_back(original_input);
			int begin = (footprint.size() > 15 ? footprint.size() - 15 : 0);
			for(int i = begin; i < footprint.size(); i++) {
				cout << i-begin+1 << " " << footprint[i] << endl;
			}
		}
		// If the command is printfile, print that file, with the case of redirection
		else if(input.substr(0,9) == COMMAND_PRINTFILE && input.at(9) == ' ' && input.substr(10).find(" ") == string::npos) {
			footprint.push_back(original_input);
			string rest = input.substr(10);
			size_t index = rest.find(">");
			if(index == string::npos) {
				if(fork() == 0) {
					execl("/bin/cat", "cat", rest.c_str(), NULL);
				}
				else {
					wait(0);
				}
			}
			else {
				if(fork() == 0){
					execl("/bin/cp", "cp", rest.substr(0,index).c_str(), rest.substr(index+1).c_str(), NULL);
				}
				else {
					wait(0);
				}
			}
		}
		// Listdir command with pipe and grep
		else if(input.substr(0,13) == "listdir|grep ") {
			string rest = input.substr(13);
			if(rest.find(" ")!=string::npos || rest.find(">")!=string::npos){
				cout << COMMAND_INVALID << endl;
				continue;
			}
			// Get rid of " characters
			while(rest.length() > 0 && rest.at(0) == '"') {
				rest = rest.substr(1);
			}
			while(rest.length() > 0 && rest.at(rest.length()-1) == '"'){
				rest = rest.substr(0, rest.length()-1);
			}
			footprint.push_back(original_input);
			int fd[2];
			pipe(fd);
			if(fork() == 0){
			    dup2(fd[1], STDOUT_FILENO);
			    close(fd[1]);
			    execlp("ls", "-l", NULL);
			}
			else{ 
			    if(fork()==0){
			        dup2(fd[0], STDIN_FILENO);
			        close(fd[0]);
		            execlp("grep", "grep", rest.c_str(),NULL);
			    }
			    else wait(0);
			}
		}
		// Listdir -a command with pipe and grep
		else if(input.substr(0,16) == "listdir -a|grep ") {
			string rest = input.substr(16);
			if(rest.find(" ")!=string::npos || rest.find(">")!=string::npos){
				cout << COMMAND_INVALID << endl;
				continue;
			}
			// Get rid of " characters
			while(rest.length() > 0 && rest.at(0) == '"') {
				rest = rest.substr(1);
			}
			while(rest.length() > 0 && rest.at(rest.length()-1) == '"'){
				rest = rest.substr(0, rest.length()-1);
			}
			footprint.push_back(original_input);
			int fd[2];
			pipe(fd);
			if(fork() == 0){
			    dup2(fd[1], STDOUT_FILENO);
			    close(fd[1]);
				execl("/bin/ls", "ls", "-a", NULL);
			}
			else{ 
			    if(fork()==0){
			        dup2(fd[0], STDIN_FILENO);
			        close(fd[0]);
		            execlp("grep", "grep", rest.c_str(),NULL);
			    }
			    else wait(0);
			}
		}
		// Invalid command
		else {
			cout << COMMAND_INVALID << endl;
		}
	}

	return 0;
}