#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

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
    double learningRate = 0.01;  // Learning rate for gradient descent
    int maxIterations = 10000;    // Maximum number of iterations
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

double CalculateRMSE(const Dataset& data, double w, double b) 
{
    double sumError = 0;
    int n = data.x.size();
    for (int i = 0; i < n; i++) 
    {
        double prediction = w * data.x[i] + b;
        sumError += pow(data.y[i] - prediction, 2);
    }
    return sqrt(sumError / n);
}

int main() 
{
    Dataset trainingData, testData;
    double w = 0, b = 0;

    // Read training data from nine files
    for (int i = 1; i <= 9; i++) 
    {
        ReadData("trainset_" + to_string(i) + ".txt", trainingData);
    }

    // Train the model using gradient descent
    TrainLinearRegression(trainingData, w, b);
    
    cout << "Final weight (w): " << w << endl;
    cout << "Final bias (b): " << b << endl;

    // Read test data
    ReadData("testset_10.txt", testData); 

    // Calculate RMSE on test data
    double rmse = CalculateRMSE(testData, w, b);

    cout << "The RMSE on the test set is: " << rmse << endl;

    return 0;
}
