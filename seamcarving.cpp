#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <vector>
#include <algorithm>

using namespace std;

class SeamCarver
{
public:
    SeamCarver(string fname, int m_numRowsToRemove, int m_numColsToRemove);
    void printImage();
    void printEnergyMatrix();
    void printCumulativeEnergyMatrixHorizontal();
    void printCumulativeEnergyMatrixVertical();

private:
    void createEnergyMatrix();
    void createCumulativeEnergyMatrixHorizontal();
    void createCumulativeEnergyMatrixVertical();
    void getColsToRemoveGreedily();
    void getRowsToRemoveGreedily();
    void removeRows(vector<int>& colsToRemove);
    void removeCols(vector<int>& rowsToRemove);
    vector<vector<int >> m_image;
    vector<vector<int >> m_energyMatrix;
    vector<vector<int >> m_cumulativeEnergyMatrixHorizontal;
    vector<int> m_colsToRemove;
    vector<int> m_rowsToRemove;
    int m_numRows;
    int m_numCols;
    int m_numRowsToRemove;
    int m_numColsToRemove;
};

int main(int argc, char** argv)
{
    // Handle arguments
    if (argc != 4)
    {
        cout << "Args: [filename] [numRowsToRemove] [numColsToRemove]\n";
        return 1;
    }

    SeamCarver sc(argv[1], atoi(argv[2]), atoi(argv[3]));
    sc.printImage();
    cout << endl;
    sc.printEnergyMatrix();
    cout << endl;
    sc.printCumulativeEnergyMatrixHorizontal();
}

SeamCarver::SeamCarver(string fname, int numRowsToRemove, int numColsToRemove)
{
    ifstream ifs(fname);
    if (!ifs.is_open())
    {
        std::runtime_error(fname + " was not found or is not accessible.\n");
    }

    // set m_numRowsToRemove and m_numColsToRemove
    m_numColsToRemove = numColsToRemove;
    m_numRowsToRemove = numRowsToRemove;

    // analyze image file header
    string tmp;

     // skip `P2` literal
    getline(ifs, tmp);
    cout << tmp;

    //** ASSUME IMAGE FILES HAS COMMENTS **//
    getline(ifs, tmp); 
    cout << tmp;

    // get image's m_numCols and m_numRows
    ifs >> m_numCols;
    ifs >> m_numRows;


    // skip the new line char for dimensions line
    getline(ifs, tmp);

    // skip max grayscale line
    getline(ifs, tmp); 

    // store values into matrix
    for (int row = 0; row < m_numRows; row++)
    {
        vector<int> colVector;
        for (int col = 0; col < m_numCols && !ifs.eof(); col++)
        {
            int val;
            ifs >> val;
            colVector.push_back(val);
        }
        m_image.push_back(colVector);
    }

    createEnergyMatrix();
    createCumulativeEnergyMatrixHorizontal();
}

void SeamCarver::createEnergyMatrix()
{
    int m_numRows = m_image.size();

    for (int row = 0; row < m_image.size(); row++)
    {
        vector<int> energyMatrixCol;
        int m_numCols = m_image[row].size();

        for (int col = 0; col < m_numCols; col++)
        {
            int rowAbove = row - 1;
            int rowBelow = row + 1;
            int colBefore = col - 1;
            int colAfter = col + 1;

            int aboveDiff;
            int belowDiff;
            int leftDiff;
            int rightDiff;

            int current = m_image[row][col];

            // if first row
            if (row == 0)
            {
                aboveDiff = 0;
            }
            else
            {
                aboveDiff = abs(current - m_image[rowAbove][col]);
            }

            // if last row
            if (row == m_numRows - 1)
            {
                belowDiff = 0;
            }
            else
            {
                belowDiff = abs(current - m_image[rowBelow][col]);
            }

            // if first col
            if (col == 0)
            {
                leftDiff = 0;
            }
            else
            {
                leftDiff = abs(current - m_image[row][colBefore]);
            }

            // if last col
            if (col == m_numCols - 1)
            {
                rightDiff = 0;
            }
            else
            {
                rightDiff = abs(current - m_image[row][colAfter]);
            }

            energyMatrixCol.push_back( 
                aboveDiff + // the one above
                belowDiff + // the one below
                leftDiff + // the one to the left
                rightDiff); // the one to the right
        }
        m_energyMatrix.push_back(energyMatrixCol);
    }
}

void SeamCarver::createCumulativeEnergyMatrixHorizontal()
{
    for (int i = 0; i < m_numRows; i++)
    {
        vector<int> v;
        v.push_back(m_energyMatrix[i][0]);
        m_cumulativeEnergyMatrixHorizontal.push_back(v);
    }

    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = 1; j < m_numCols; j++)
        {
            if (i == 0)
            {
                m_cumulativeEnergyMatrixHorizontal[i].push_back(
                    m_energyMatrix[i][j] + std::min(
                        m_cumulativeEnergyMatrixHorizontal[i][j - 1],
                        m_cumulativeEnergyMatrixHorizontal[i + 1][j - 1]));
            }
            else if (i == m_numRows - 1)
            {
                m_cumulativeEnergyMatrixHorizontal[i].push_back(
                    m_energyMatrix[i][j] + std::min(
                        m_cumulativeEnergyMatrixHorizontal[i][j - 1],
                        m_cumulativeEnergyMatrixHorizontal[i - 1][j - 1]));
            }
            else
            {
                m_cumulativeEnergyMatrixHorizontal[i].push_back(
                    m_energyMatrix[i][j] + std::min(std::min(
                        m_cumulativeEnergyMatrixHorizontal[i][j - 1],
                        m_cumulativeEnergyMatrixHorizontal[i + 1][j - 1]),
                        m_cumulativeEnergyMatrixHorizontal[i - 1][j - 1]));
            }
        }
    }
}

void SeamCarver::getColsToRemoveGreedily()
{
    for (int i = 0; i < m_numColsToRemove; i++)
    {
        // find starting col
        std::vector<int> colsToRemove;

        int min = m_energyMatrix[0][0];
        int minIndex = 0;

        for (int col = 1; col < m_numCols; col++)
        {
            if (m_energyMatrix[0][col] < min)
            {
                minIndex = col;
                min = m_energyMatrix[0][col];
            }
        }

        colsToRemove.push_back(minIndex);

        for (int row = 1; row < m_numRows; row++)
        {
            int prevColIndex = minIndex;
            min = numeric_limits<int>::max(); // has to be reset to some value

            // check below
            if (m_energyMatrix[row][prevColIndex] < min)
            {
                min = m_energyMatrix[row][prevColIndex];
                minIndex = prevColIndex;
            }

            // check to left
            if (prevColIndex != 0) // not left most col
            {
                if (m_energyMatrix[row][prevColIndex - 1] < min)
                {
                    min = m_energyMatrix[row][prevColIndex - 1];
                    minIndex = prevColIndex - 1;
                }
            }

            // check to right
            if (prevColIndex != m_numCols - 1) // not right most col
            {
                if (m_energyMatrix[row][prevColIndex + 1] < min)
                {
                    min = m_energyMatrix[row][prevColIndex + 1];
                    minIndex = prevColIndex + 1;
                }
            }
            colsToRemove.push_back(minIndex);
        }
    }
}

void SeamCarver::getRowsToRemoveGreedily()
{
    for (int i = 0; i < m_numRowsToRemove; i++)
    {
        // find starting row
        std::vector<int> rowsToRemove;

        int min = m_energyMatrix[0][0];
        int minIndex = 0;

        for (int row = 1; row < m_numRows; row++)
        {
            if (m_energyMatrix[row][0] < min)
            {
                minIndex = row;
                min = m_energyMatrix[row][0];
            }
        }
        rowsToRemove.push_back(minIndex);

        for (int col = 1; col < m_numCols; col++)
        {
            int prevRowIndex = minIndex;
            min = numeric_limits<int>::max(); // has to be reset to some value

            // check same row
            if (m_energyMatrix[prevRowIndex][col] < min)
            {
                min = m_energyMatrix[prevRowIndex][col];
                minIndex = prevRowIndex;
            }

            // check row above
            if (prevRowIndex != 0) // not top row
            {
                if (m_energyMatrix[prevRowIndex - 1][col] < min)
                {
                    min = m_energyMatrix[prevRowIndex - 1][col];
                    minIndex = prevRowIndex - 1;
                }
            }

            // check row below
            if (prevRowIndex != m_numRows - 1) // not bot row
            {
                if (m_energyMatrix[prevRowIndex + 1][col] < min)
                {
                    min = m_energyMatrix[prevRowIndex + 1][col];
                    minIndex = prevRowIndex + 1;
                }
            }
            rowsToRemove.push_back(minIndex);
        }
    }
}

// removes cols, at index 0 remove image[colsToRemove[0], 0]
// col is the index, row is the value of colsToRemove
void SeamCarver::removeCols(vector<int>& colsToRemove)
{
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = colsToRemove[i]; j < m_numCols - 1; j++)
        {
            m_image[i][j] = m_image[i][j + 1];
        }
    }
    m_numCols--;
}

// removes rows, at index 0 remove image[0, rowsToRemove[0]]
// row is the index, column is the value of rowsToRemove
void SeamCarver::removeRows(vector<int>& rowsToRemove)
{
    for (int i = 0; i < m_numCols; i++)
    {
        for (int j = rowsToRemove[i]; j < m_numRows - 1; j++)
        {
            m_image[j][i] = m_image[j + 1][i];
        }
    }
    m_numRows--;
}

void SeamCarver::printImage()
{
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = 0; j < m_numCols; j++)
        {
            cout << m_image[i][j] << " ";
        }
        cout << endl;
    }
}

void SeamCarver::printEnergyMatrix()
{
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = 0; j < m_numCols; j++)
        {
            cout << m_energyMatrix[i][j] << " ";
        }
        cout << endl;
    }
}

void SeamCarver::printCumulativeEnergyMatrixHorizontal()
{
    for (int i = 0; i < m_cumulativeEnergyMatrixHorizontal.size(); i++)
    {
        for (int j = 0; j < m_cumulativeEnergyMatrixHorizontal[i].size(); j++)
        {
            cout << m_cumulativeEnergyMatrixHorizontal[i][j] << " ";
        }
        cout << endl;
    }
}