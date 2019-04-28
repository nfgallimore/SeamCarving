#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cout << "Args: [filename] [numRowsToRemove] [numColsToRemove]\n";
        return 1;
    }

    string fname = argv[1];
    int numRowsToRemove = atoi(argv[2]);
    int numColsToRemove = atoi(argv[3]);

    std::ifstream ifs(fname);
    if (!ifs.is_open())
    {
        std::cout << "File could not be opened.\n";
        return 1;
    }

    // analyze image file header
    string tmp;
    getline(ifs, tmp); // P2 literal
    getline(ifs, tmp); // comments (assume it has comments)

    // get file's x and y
    int numCols, numRows;
    ifs >> numCols;
    ifs >> numRows;
    
    getline(ifs, tmp); // skip rest of dimensions line
    getline(ifs, tmp); // skip max grayscale line

    // setup file matrix
    int pgmValues[numRows][numCols]; // pgmValues[row][column]

    // store values into matrix
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols && !ifs.eof(); col++)
        {
            ifs >> pgmValues[row][col];
            cout << pgmValues[row][col] << " ";
        }
        cout << endl;
    }

    // create energy matrix
    int energyMatrix[numRows][numCols];

    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols && !ifs.eof(); col++)
        {
            int rowAbove = row - 1;
            int rowBelow = row + 1;
            int colBefore = col - 1;
            int colAfter = col + 1;

            int aboveDiff;
            int belowDiff;
            int leftDiff;
            int rightDiff;

            int above = pgmValues[rowAbove][col];
            int below = pgmValues[rowBelow][col];
            int left = pgmValues[row][colBefore];
            int right = pgmValues[row][colAfter];

            int current = pgmValues[row][col];

            // if first row
            if (row == 0)
            {
                aboveDiff = 0;
            }
            else
            {
                aboveDiff = abs(current - above);
            }

            // if last row
            if (row == numRows - 1)
            {
                belowDiff = 0;
            }
            else
            {
                belowDiff = abs(current - below);
            }

            // if first col
            if (col == 0)
            {
                leftDiff = 0;
            }
            else
            {
                leftDiff = abs(current - left);
            }

            // if last col
            if (col == numCols - 1)
            {
                rightDiff = 0;
            }
            else
            {
                rightDiff = abs(current - right);
            }

            energyMatrix[row][col] = 
                aboveDiff + // the one above
                belowDiff + // the one below
                leftDiff + // the one to the left
                rightDiff; // the one to the right
        }
    }

    // print energy matrix
    cout << endl;
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            cout << energyMatrix[row][col] << " ";
        }
        cout << endl;
    }

    // Remove cols
    for (int i = 0; i < numColsToRemove; i++)
    {
        // find starting col
        std::vector<int> colsToRemove;

        int min = energyMatrix[0][0];
        int minIndex = 0;

        for (int col = 1; col < numCols; col++)
        {
            if (energyMatrix[0][col] < min)
            {
                minIndex = col;
                min = energyMatrix[0][col];
            }
        }

        colsToRemove.push_back(minIndex);

        for (int row = 1; row < numRows; row++)
        {
            int prevColIndex = minIndex;
            min = numeric_limits<int>::max(); // has to be reset to some value

            // check below
            if (energyMatrix[row][prevColIndex] < min)
            {
                min = energyMatrix[row][prevColIndex];
                minIndex = prevColIndex;
            }

            // check to left
            if (prevColIndex != 0) // not left most col
            {
                if (energyMatrix[row][prevColIndex - 1] < min)
                {
                    min = energyMatrix[row][prevColIndex - 1];
                    minIndex = prevColIndex - 1;
                }
            }

            // check to right
            if (prevColIndex != numCols - 1) // not right most col
            {
                if (energyMatrix[row][prevColIndex + 1] < min)
                {
                    min = energyMatrix[row][prevColIndex + 1];
                    minIndex = prevColIndex + 1;
                }
            }

            colsToRemove.push_back(minIndex);
        }

        // remove the columns from pgmValues array
        for (int i = 0; i < numRows; i++)
        {
            for (int j = colsToRemove[i]; j < numCols - 1; j++)
            {
                pgmValues[i][j] = pgmValues[i][j + 1];
            }
        }
        numCols--;

        cout << "\nCols to remove: ";
        for (int i = 0; i < colsToRemove.size(); i++)
        {
            cout << colsToRemove[i] << " ";
        }
        cout << endl;
    }

    // Remove rows
    for (int i = 0; i < numRowsToRemove; i++)
    {
        // find starting row
        std::vector<int> rowsToRemove;

        int min = energyMatrix[0][0];
        int minIndex = 0;

        for (int row = 1; row < numRows; row++)
        {
            if (energyMatrix[row][0] < min)
            {
                minIndex = row;
                min = energyMatrix[row][0];
            }
        }
        rowsToRemove.push_back(minIndex);

        for (int col = 1; col < numCols; col++)
        {
            int prevRowIndex = minIndex;
            min = numeric_limits<int>::max(); // has to be reset to some value

            // check same row
            if (energyMatrix[prevRowIndex][col] < min)
            {
                min = energyMatrix[prevRowIndex][col];
                minIndex = prevRowIndex;
            }

            // check row above
            if (prevRowIndex != 0) // not top row
            {
                if (energyMatrix[prevRowIndex - 1][col] < min)
                {
                    min = energyMatrix[prevRowIndex - 1][col];
                    minIndex = prevRowIndex - 1;
                }
            }

            // check row below
            if (prevRowIndex != numRows - 1) // not bot row
            {
                if (energyMatrix[prevRowIndex + 1][col] < min)
                {
                    min = energyMatrix[prevRowIndex + 1][col];
                    minIndex = prevRowIndex + 1;
                }
            }

            rowsToRemove.push_back(minIndex);
        }

        // remove the rows from pgmValues array
        for (int i = 0; i < numCols; i++)
        {
            for (int j = rowsToRemove[i]; j < numRows - 1; j++)
            {
                pgmValues[j][i] = pgmValues[j + 1][i];
            }
        }
        numRows--;

        cout << "\nRows to remove: ";
        for (int i = 0; i < rowsToRemove.size(); i++)
        {
            cout << rowsToRemove[i] << " ";
        }
        cout << endl;
    }

    // print resulting image
    cout << endl;
    
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols; col++)
        {
            cout << pgmValues[row][col] << " ";
        }
        cout << endl;
    }
}