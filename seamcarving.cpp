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

private:
    void parseFile(string fname);
    void createEnergyMatrix();
    void createCumulativeEnergyMatrixHorizontal();
    void createCumulativeEnergyMatrixVertical();
    void getHorizontalSeamToRemove();
    void getVerticalSeamToRemove();
    void removeHorizontalSeam();
    void removeVerticalSeam();

    // printing functions
    void printEnergyMatrix();
    void printCumulativeEnergyMatrixHorizontal();
    void printCumulativeEnergyMatrixVertical();
    void printHorizontalSeamToRemove();
    void printVerticalSeamToRemove();

    // member variables
    vector<vector<int> > m_image;
    vector<vector<int> > m_energyMatrix;
    vector<vector<int> > m_cumulativeEnergyMatrixHorizontal;
    vector<vector<int> > m_cumulativeEnergyMatrixVertical;
    vector<int> m_horizontalSeamToRemove;
    vector<int> m_verticalSeamToRemove;
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
}

SeamCarver::SeamCarver(string fname, int numColsToRemove, int numRowsToRemove)
{
    m_numColsToRemove = numColsToRemove;
    m_numRowsToRemove = numRowsToRemove;
    parseFile(fname);

    printImage();
    cout << endl;

    createEnergyMatrix();
    printEnergyMatrix();
    cout << endl;

    createCumulativeEnergyMatrixVertical();
    printCumulativeEnergyMatrixVertical();
    cout << endl;

    getVerticalSeamToRemove();
    cout << endl;

    printVerticalSeamToRemove();
    cout << endl;

    removeVerticalSeam();
    printImage();
}

void SeamCarver::parseFile(string fname)
{
    ifstream ifs(fname);
    if (!ifs.is_open())
    {
        std::runtime_error(fname + " was not found or is not accessible");
    }

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

    if (m_numRows - m_numRowsToRemove < 1)
    {
        stringstream msg;
        msg << "cannot carve " << m_numRowsToRemove 
        << " from image with " << m_numRows << " rows";
        throw std::logic_error(msg.str());
    }
    if (m_numCols - m_numColsToRemove < 1)
    {
        stringstream msg;
        msg << "cannot carve " << m_numColsToRemove 
        << " from image with " << m_numCols << " columns";
        throw std::logic_error(msg.str());
    }

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

    for (int j = 1; j < m_numCols; j++)
    {
        for (int i = 0; i < m_numRows; i++)
        {
            int energy = m_energyMatrix[i][j];
            if (i == 0) // is top row
            {
                int sameRow = m_cumulativeEnergyMatrixHorizontal[i][j - 1];
                int belowRow = m_cumulativeEnergyMatrixHorizontal[i + 1][j - 1];
                int min = energy + std::min(sameRow, belowRow);
                m_cumulativeEnergyMatrixHorizontal[i].push_back(min);
            }
            else if (i == m_numRows - 1) // is last row
            {
                int sameRow = m_cumulativeEnergyMatrixHorizontal[i][j - 1];
                int aboveRow = m_cumulativeEnergyMatrixHorizontal[i - 1][j - 1];
                int min = energy + std::min(sameRow, aboveRow);
                m_cumulativeEnergyMatrixHorizontal[i].push_back(min);
            }
            else
            {
                int aboveRow = m_cumulativeEnergyMatrixHorizontal[i - 1][j - 1];
                int sameRow = m_cumulativeEnergyMatrixHorizontal[i][j - 1];
                int belowRow = m_cumulativeEnergyMatrixHorizontal[i + 1][j - 1];
                int min = m_energyMatrix[i][j] + std::min(std::min(aboveRow, sameRow), belowRow);
                m_cumulativeEnergyMatrixHorizontal[i].push_back(min);
            }
        }
    }
}

void SeamCarver::createCumulativeEnergyMatrixVertical ()
{
    vector<int> v;
    for (int i = 0; i < m_numCols; i++)
    {
        v.push_back(m_energyMatrix[0][i]);
    }
    m_cumulativeEnergyMatrixVertical.push_back(v);

    for (int i = 1; i < m_numRows; i++)
    {
        vector<int> cumulativeEnergyRow;
        for (int j = 0; j < m_numCols; j++)
        {
            int energy = m_energyMatrix[i][j];

            if (j == 0) // is left most col
            {
                int aboveCol = m_cumulativeEnergyMatrixVertical[i - 1][j];
                int rightCol = m_cumulativeEnergyMatrixVertical[i - 1][j + 1];
                int min = energy + std::min(aboveCol, rightCol);
                cumulativeEnergyRow.push_back(min);
            }
            else if (j == m_numCols - 1) // is right most col
            {
                int leftCol = m_cumulativeEnergyMatrixVertical[i - 1][j - 1];
                int aboveCol = m_cumulativeEnergyMatrixVertical[i - 1][j];
                int min = energy + std::min(leftCol, aboveCol);
                cumulativeEnergyRow.push_back(min);
            }
            else
            {
                int leftCol = m_cumulativeEnergyMatrixVertical[i - 1][j - 1];
                int aboveCol = m_cumulativeEnergyMatrixVertical[i - 1][j];
                int rightCol = m_cumulativeEnergyMatrixVertical[i - 1][j + 1];
                int min = energy + std::min(std::min(leftCol, aboveCol), rightCol);
                cumulativeEnergyRow.push_back(min);
            }
        }
        m_cumulativeEnergyMatrixVertical.push_back(cumulativeEnergyRow);
    }
}

void SeamCarver::getVerticalSeamToRemove()
{
    int min = numeric_limits<int>::max();
    int minIndex;
    for (int j = 0; j < m_numCols; j++)
    {
        if (m_cumulativeEnergyMatrixVertical[0][j] < min)
        {
            minIndex = j;
            min = m_cumulativeEnergyMatrixVertical[0][j];
        }
    }
    m_verticalSeamToRemove.push_back(minIndex);

    for (int i = 1; i < m_numRows; i++)
    {
        int j = m_verticalSeamToRemove.back();
        int min = numeric_limits<int>::max();
        int minIndex;

        // check if directly below col is smaller
        if (m_cumulativeEnergyMatrixVertical[i][j] < min)
        {
            min = m_cumulativeEnergyMatrixVertical[i][j];
            minIndex = j;
        }

        // if not left most col && if left col below is smaller
        if (j > 0 && m_cumulativeEnergyMatrixVertical[i][j - 1] < min) 
        {
            min = m_cumulativeEnergyMatrixVertical[i][j - 1];
            minIndex = j - 1;
        }

        // if not right most col && if right col below is smaller
        if (j != m_numCols - 1 && m_cumulativeEnergyMatrixVertical[i][j + 1] < min) 
        {
            min = m_cumulativeEnergyMatrixVertical[i][j + 1];
            minIndex = j + 1;
        }

        m_verticalSeamToRemove.push_back(minIndex);
    }
}

void SeamCarver::getHorizontalSeamToRemove()
{
    // find starting row
    int min = m_cumulativeEnergyMatrixHorizontal[0][0];
    int minIndex = 0;

    for (int row = 1; row < m_numRows; row++)
    {
        if (m_cumulativeEnergyMatrixHorizontal[row][0] < min)
        {
            minIndex = row;
            min = m_cumulativeEnergyMatrixHorizontal[row][0];
        }
    }
    m_horizontalSeamToRemove.push_back(minIndex);

    for (int col = 1; col < m_numCols; col++)
    {
        int prevRowIndex = minIndex;
        min = numeric_limits<int>::max(); // has to be reset to some value

        // check same row
        if (m_cumulativeEnergyMatrixHorizontal[prevRowIndex][col] < min)
        {
            min = m_cumulativeEnergyMatrixHorizontal[prevRowIndex][col];
            minIndex = prevRowIndex;
        }

        // check row above
        if (prevRowIndex != 0) // not top row
        {
            if (m_cumulativeEnergyMatrixHorizontal[prevRowIndex - 1][col] < min)
            {
                min = m_cumulativeEnergyMatrixHorizontal[prevRowIndex - 1][col];
                minIndex = prevRowIndex - 1;
            }
        }

        // check row below
        if (prevRowIndex != m_numRows - 1) // not bot row
        {
            if (m_cumulativeEnergyMatrixHorizontal[prevRowIndex + 1][col] < min)
            {
                min = m_cumulativeEnergyMatrixHorizontal[prevRowIndex + 1][col];
                minIndex = prevRowIndex + 1;
            }
        }
        m_horizontalSeamToRemove.push_back(minIndex);
    }
}

// removes cols, at index 0 remove image[colsToRemove[0], 0]
// col is the index, row is the value of colsToRemove
void SeamCarver::removeVerticalSeam()
{
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = m_verticalSeamToRemove[i]; j < m_numCols - 1; j++)
        {
            m_image[i][j] = m_image[i][j + 1];
        }
    }
    m_numCols--;
}

// removes rows, at index 0 remove image[0, rowsToRemove[0]]
// row is the index, column is the value of rowsToRemove
void SeamCarver::removeHorizontalSeam()
{
    for (int i = 0; i < m_numCols; i++)
    {
        for (int j = m_horizontalSeamToRemove[i]; j < m_numRows - 1; j++)
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
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = 0; j < m_numCols; j++)
        {
            cout << m_cumulativeEnergyMatrixHorizontal[i][j] << " ";
        }
        cout << endl;
    }
}

void SeamCarver::printCumulativeEnergyMatrixVertical()
{
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = 0; j < m_numCols; j++)
        {
            cout << m_cumulativeEnergyMatrixVertical[i][j] << " ";
        }
        cout << endl;
    }
}

void SeamCarver::printVerticalSeamToRemove()
{
    for (int i = 0; i < m_verticalSeamToRemove.size(); i++)
    {
        cout << m_verticalSeamToRemove[i] << " ";
    }
}

void SeamCarver::printHorizontalSeamToRemove()
{
    for (int i = 0; i < m_horizontalSeamToRemove.size(); i++)
    {
        cout << m_horizontalSeamToRemove[i] << " ";
    }
}
