#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;


vector<pid_t> childPIDs; 
bool running=false;
// Function to display available commands
void handle_signal(int){
    if(running) return;
    else exit(0);
    
}


void showHelp() {
    cout << "Available commands:\n";
    cout << "  help          - Show this help message\n";
    cout << "  pwd           - Show current working directory\n";
    cout << "  run <program> <args> - Run a program and wait for it to finish\n";
    cout << "  bg <program> <args>  - Run a program in the background\n";
}

// Function to print current directory
void printWorkingDirectory() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        cout << cwd << endl;
    } else {
        perror("getcwd");
    }
}
// Function to execute a program in a new process
void executeProgramInNewProcess(void (*func)(), const char* funcName) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) { // Child process
        // Execute the function in the child process
        cout << "Executing " << funcName << " in process with PID " << getpid() << endl;
        func();  // Call the function directly
        exit(EXIT_SUCCESS); // Exit after function call
    } else { // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child to finish
        cout << funcName << " process exited with status: " << WEXITSTATUS(status) << endl;
    }
}

// Function to execute a program (either in foreground or background)
void executeProgram(vector<string> &args, bool background) {



    if (args.empty()) {
        cerr << "Error: No program specified.\n";
        return;
    }


    // Convert vector<string> to char* array for execvp
    vector<char *> c_args;
    for (const auto &arg : args) {
        c_args.push_back(const_cast<char *>(arg.c_str()));
    }
    c_args.push_back(nullptr); // Null-terminate the array

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    running=true;
    if (pid == 0) { // Child process
        execvp(c_args[0], c_args.data());
        perror("execvp"); // Only executed if execvp fails
        exit(EXIT_FAILURE);
    } else { // Parent process
        childPIDs.push_back(pid);  // Keep track of the child PID
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
            cout << "Process exited with status: " << WEXITSTATUS(status) << endl;

        } else {
            cout << "Started background process with PID " << pid << endl;
        }
    }

    running=false;
}
// Function to parse user input into a vector of arguments
vector<string> parseInput(const string &input) {
    vector<string> args;
    istringstream iss(input);
    string word;
    while (iss >> word) {
        args.push_back(word);
    }
    return args;
}
void killAllChildProcesses() {
    for (pid_t pid : childPIDs) {
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        if(result==0){
        cout << "Killing process with PID: " << pid << endl;
        kill(pid, SIGKILL);  // Send SIGKILL to terminate the process
        }
    }
    childPIDs.clear();  // Clear the list after killing all processes
}



int main() {
    string input;
    signal(SIGINT, handle_signal);
    while (true) {
        cout << "> ";
        getline(cin, input);

        // Trim and parse input
        vector<string> args = parseInput(input);
        if (args.empty()) continue;

        string command = args[0];
        args.erase(args.begin()); // Remove command from arguments

        if (command == "help") {
            executeProgramInNewProcess(showHelp, "showHelp");
        } else if (command == "pwd") {
            executeProgramInNewProcess(printWorkingDirectory, "printWorkingDirectory");
        } else if (command == "run") {
            executeProgram(args, false);
        } else if (command == "bg") {
            executeProgram(args, true);
        } else if (command == "exit") {
            cout << "Exiting...\n";
            break;
        } else {
            cout << "Unknown command: " << command << endl;
        }
    }
    killAllChildProcesses();
    return 0;
}
