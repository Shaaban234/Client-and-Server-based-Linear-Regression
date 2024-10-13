#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>
#include <winsock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib")

using namespace std;

struct Dataset 
{
    vector<double> x;
    vector<double> y;
};

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

void TrainLinearRegression(const Dataset& data, double& w, double& b) 
{
    // calculate linear regression using the derivative formula for w and b
    double learningRate = 0.01;  // Learning rate for gradient descent
    int maxIterations = 10000;   // Maximum number of iterations
    int n = data.x.size();

    for (int iter = 0; iter < maxIterations; iter++) 
    {
        double gradW = 0.0, gradB = 0.0;

        // Calculate the gradient for w and b
        for (int i = 0; i < n; i++) 
        {
            double prediction = w * data.x[i] + b;
            double error = prediction - data.y[i];
            gradW += error * data.x[i];
            gradB += error;
        }
        gradW /= n;
        gradB /= n;

        // Update the parameters
        w -= learningRate * gradW;
        b -= learningRate * gradB;
    }
}

int main() 
{
    WSADATA wsa;
    SOCKET sock;
    struct sockaddr_in server_address;
    double w = 0, b = 0;
    char filename[256];
    int recvSize;
    Dataset trainingData;
    // Initialize Winsock
    cout << "\nInitialising Winsock..." << endl;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
    {
        cerr << "Failed. Error Code : " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Initialised." << endl;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
        cerr << "Could not create socket : " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Socket created." << endl;

    // Setup server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Localhost
    server_address.sin_port = htons(3001);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
    {
        cerr << "Connection failed with error code : " << WSAGetLastError() << endl;
        closesocket(sock);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << "Connected to the server." << endl;

   
    recvSize = recv(sock, (char*)&filename, sizeof(filename), 0);
    // Null-terminate the received string
    if (recvSize < sizeof(filename)) 
    {
        filename[recvSize] = '\0';
    } else 
    {
        filename[sizeof(filename) - 1] = '\0'; // Ensure null-termination if max length reached
    }
    cout << "Received filename from client: " << filename << endl;
    ReadData(filename, trainingData); 
    TrainLinearRegression(trainingData, w, b);
    
    cout << "Final weight (w): " << w << endl;
    cout << "Final bias (b): " << b << endl;

    // Send the values of w and b back to the server
    send(sock, (char*)&w, sizeof(w), 0);
    send(sock, (char*)&b, sizeof(b), 0);

    // Close the socket
    closesocket(sock);
    WSACleanup();

    return 0;
}
