#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <sstream>
#pragma comment(lib, "ws2_32.lib") // Winsock Library

using namespace std;

struct Dataset 
{
    vector<double> x;
    vector<double> y;
};

double w_total = 0, b_total = 0;
int client_id = 1; // Global counter to track the number of clients

void ReadData(const string& fileName, Dataset& data) 
{
    ifstream file(fileName);
    if (!file.is_open()) 
    {
        cerr << "Error opening file " << fileName << endl;
        return;
    }
    string line;
    while (getline(file, line)) 
    {
        stringstream ss(line);
        double xVal, yVal;
        ss >> xVal >> yVal;
        data.x.push_back(xVal);
        data.y.push_back(yVal);
    }
    file.close();
}

void handle_client(SOCKET client_socket) 
{
    // Generate file name for the client
    string filename = "trainset_" + to_string(client_id) + ".txt"; 

    // Send the file name to the client
    send(client_socket, filename.c_str(), filename.length(), 0);

    double w = 0, b = 0;
    int recvSize = recv(client_socket, (char*)&w, sizeof(w), 0);
    if (recvSize > 0) 
    {
        cout << "Received weight (w) from client " << client_id << ": " << w << endl;
    }

    recvSize = recv(client_socket, (char*)&b, sizeof(b), 0);
    if (recvSize > 0) 
    {
        cout << "Received bias (b) from client " << client_id << ": " << b << endl;
    }

    // Accumulate totals
    w_total += w;
    b_total += b;

    closesocket(client_socket); // Close the client socket
    client_id += 1; // Increment client ID for the next connection
}

DWORD WINAPI ClientThreadFunc(LPVOID lpParam) 
{
    SOCKET client_socket = (SOCKET)(intptr_t)lpParam; // Cast the parameter back to SOCKET
    handle_client(client_socket);
    return 0;
}

double CalculateRMSE(const Dataset& data, double w, double b) 
{
    double sumError = 0;
    int n = data.x.size();
    for (int i = 0; i < n; i++) {
        double prediction = w * data.x[i] + b;
        sumError += pow(data.y[i] - prediction, 2);
    }
    return sqrt(sumError / n);
}

int main() 
{
    Dataset testData;
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_address_len = sizeof(client_address);
    HANDLE hThread;
    double avg_w = 0;
    double avg_b = 0;

    // Initialize Winsock
    cout << "\nInitialising Winsock...";
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
    {
        cerr << "Failed. Error Code : " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Initialised.\n";

    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
        cerr << "Could not create socket : " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Socket created.\n";

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(3001);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == SOCKET_ERROR) 
    {
        cerr << "Bind failed with error code : " << WSAGetLastError() << endl;
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << "Bind done.\n";

    // Listen for incoming connections
    listen(server_socket, 3);
    cout << "Waiting for incoming connections...\n";

    while (client_id <= 9) 
    { 
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == INVALID_SOCKET) 
        {
            cerr << "Accept failed with error code : " << WSAGetLastError() << endl;
            continue;
        }
        cout << "Connection accepted.\n";

        // Create a new thread for each client
        hThread = CreateThread(NULL, 0, ClientThreadFunc, (LPVOID)(intptr_t)client_socket, 0, NULL);
        if (hThread == NULL) 
        {
            cerr << "Thread creation failed: " << GetLastError() << endl;
            closesocket(client_socket);
        } else 
        {
            CloseHandle(hThread); // Close the thread handle in the parent thread
        }
    }

    // Clean up
    closesocket(server_socket);
    WSACleanup();

    // Calculate the averages after all 9 clients have connected
    if (client_id > 1) 
    { 
        avg_w = w_total / 9; // Average weight
        avg_b = b_total / 9; // Average bias

        cout << "Server shutting down...\n";
        cout << "Average weight (w): " << avg_w << endl;
        cout << "Average bias (b): " << avg_b << endl;

        // Load test data and calculate RMSE
        ReadData("testset_10.txt", testData); 
        double rmse = CalculateRMSE(testData, avg_w, avg_b);

        cout << "The RMSE on the test set is: " << rmse << endl;
    } else 
    {
        cout << "No clients connected. No averages to calculate." << endl;
    }

    return 0;
}
