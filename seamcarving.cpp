#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        cout << "Filename, horizontal count, and vertical count required as arguments.\n";
        return 1;
    }

    string fname = argv[1];
    int hCount = atoi(argv[2]);
    int vCount = atoi(argv[3]);

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
        }
    }

    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols && !ifs.eof(); col++)
        {
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

            int rowAboveDiff;
            int rowBelowDiff;
            int colBeforeDiff;
            int colAfterDiff;

            // if first row
            if (rowAbove < 0)
            {
                rowAboveDiff = 0;
            }
            else
            {
                rowAboveDiff = abs(pgmValues[row][col]-pgmValues[rowAbove][col]);
            }

            // if last row
            if (rowBelow == numRows - 1)
            {
                rowBelowDiff = 0;
            }
            else
            {
                rowBelowDiff = abs(pgmValues[row][col]-pgmValues[rowBelow][col]);
            }

            // if first col
            if (colBefore < 0)
            {
                colBeforeDiff = 0;
            }
            else
            {
                colBeforeDiff = abs(pgmValues[row][col]-pgmValues[row][colBefore]);
            }

            // if last col
            if (colAfter == numCols - 1)
            {
                colAfterDiff = 0;
            }
            else
            {
                colAfterDiff = abs(pgmValues[row][col]-pgmValues[row][colAfter]);
            }

            energyMatrix[row][col] = 
                rowAboveDiff + // the one above
                rowBelowDiff + // the one below
                colBeforeDiff + // the one to the left
                colAfterDiff; // the one to the right
        }
    }

    cout << endl;
    
    for (int row = 0; row < numRows; row++)
    {
        for (int col = 0; col < numCols && !ifs.eof(); col++)
        {
            cout << energyMatrix[row][col] << " ";
        }
        cout << endl;
    }
}