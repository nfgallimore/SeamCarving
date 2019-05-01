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
    SeamCarver(string fileName, int horizontalSeamsToRemove, int verticalSeamsToRemove);
    void printImage();

private:
    void parseFile(string fileName);
    void createEnergyMatrix();
    void createCumulativeEnergyMatrixHorizontal();
    void createCumulativeEnergyMatrixVertical();
    void getHorizontalSeamToRemove();
    void getVerticalSeamToRemove();
    void removeHorizontalSeam();
    void removeVerticalSeam();
    void clearMatricesHorizontal();
    void clearMatricesVertical();

    // printing functions
    void printEnergyMatrix();
    void printCumulativeEnergyMatrixHorizontal();
    void printCumulativeEnergyMatrixVertical();
    void printHorizontalSeamToRemove();
    void printVerticalSeamToRemove();
    void writeImageToFile();

    // member variables
    vector<vector<int> > m_image;
    vector<vector<int> > m_energyMatrix;
    vector<vector<int> > m_cumulativeEnergyMatrixHorizontal;
    vector<vector<int> > m_cumulativeEnergyMatrixVertical;
    vector<int> m_horizontalSeamToRemove;
    vector<int> m_verticalSeamToRemove;
    int m_numRows;
    int m_numCols;
    int m_horizontalSeamsToRemove;
    int m_verticalSeamsToRemove;
    string m_fileName;
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

SeamCarver::SeamCarver(string fileName, int verticalSeamsToRemove, int horizontalSeamsToRemove)
{
    m_verticalSeamsToRemove = verticalSeamsToRemove;
    m_horizontalSeamsToRemove = horizontalSeamsToRemove;
    m_fileName = fileName;
    parseFile(fileName);

    while (verticalSeamsToRemove--)
    {
        createEnergyMatrix();
        createCumulativeEnergyMatrixVertical();
        getVerticalSeamToRemove();
        removeVerticalSeam();
        clearMatricesVertical();
    }

    while (horizontalSeamsToRemove--)
    {
        createEnergyMatrix();
        createCumulativeEnergyMatrixHorizontal();
        getHorizontalSeamToRemove();
        removeHorizontalSeam();
        clearMatricesHorizontal();
    }
    writeImageToFile();
}

void SeamCarver::clearMatricesHorizontal()
{
    m_energyMatrix.clear();
    m_cumulativeEnergyMatrixHorizontal.clear();
    m_horizontalSeamToRemove.clear();
}

void SeamCarver::clearMatricesVertical()
{
    m_energyMatrix.clear();
    m_cumulativeEnergyMatrixVertical.clear();
    m_verticalSeamToRemove.clear();
}

void SeamCarver::parseFile(string fileName)
{
    ifstream ifs(fileName);
    if (!ifs.is_open())
    {
        std::runtime_error(fileName + " was not found or is not accessible");
    }

    // analyze image file header
    string tmp;

     // skip `P2` literal
    getline(ifs, tmp);

    // Check if this line is a comment
    getline(ifs, tmp);

    if (tmp[0] == '#')
    {
        // get image's m_numCols and m_numRows from next line
        ifs >> m_numCols;
        ifs >> m_numRows;

        // skip the new line char for dimensions line
        getline(ifs, tmp);
    }
    else
    {
        stringstream ss;
        ss << tmp;
        ss >> m_numCols;
        ss >> m_numRows;
    }

    if (m_numRows - m_horizontalSeamsToRemove < 1)
    {
        stringstream msg;
        msg << "cannot carve " << m_horizontalSeamsToRemove 
        << " from image with " << m_numRows << " rows";
        throw std::logic_error(msg.str());
    }
    if (m_numCols - m_verticalSeamsToRemove < 1)
    {
        stringstream msg;
        msg << "cannot carve " << m_verticalSeamsToRemove 
        << " from image with " << m_numCols << " columns";
        throw std::logic_error(msg.str());
    }

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

    if (m_image.size() != m_numRows)
    {
        stringstream msg;
        msg << "image file header says there are "
        << m_numRows << " rows, but actual row count is " 
        << m_image.size();
        throw std::logic_error(msg.str());
    }

    if (m_image[0].size() != m_numCols)
    {
        stringstream msg;
        msg << "image file header says there are "
        << m_numCols << " cols, but actual column count is " 
        << m_image[0].size();
        throw std::logic_error(msg.str());
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

void SeamCarver::createCumulativeEnergyMatrixVertical()
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
        if (m_cumulativeEnergyMatrixVertical[m_numRows - 1][j] < min)
        {
            minIndex = j;
            min = m_cumulativeEnergyMatrixVertical[m_numRows - 1][j];
        }
    }
    m_verticalSeamToRemove.push_back(minIndex);

    for (int i = m_numRows - 2; i >= 0; i--)
    {
        int j = m_verticalSeamToRemove.back();
        int min = numeric_limits<int>::max();
        int minIndex;

        // if not left most col && if left above col is smaller
        if (j > 0 && m_cumulativeEnergyMatrixVertical[i][j - 1] < min) 
        {
            min = m_cumulativeEnergyMatrixVertical[i][j - 1];
            minIndex = j - 1;
        }

        // check if directly above col is smaller
        if (m_cumulativeEnergyMatrixVertical[i][j] < min)
        {
            min = m_cumulativeEnergyMatrixVertical[i][j];
            minIndex = j;
        }

        // if not right most col && if right above col is smaller
        if (j != m_numCols - 1 && m_cumulativeEnergyMatrixVertical[i][j + 1] < min) 
        {
            min = m_cumulativeEnergyMatrixVertical[i][j + 1];
            minIndex = j + 1;
        }

        m_verticalSeamToRemove.push_back(minIndex);
    }
    reverse(m_verticalSeamToRemove.begin(), m_verticalSeamToRemove.end());
}

void SeamCarver::getHorizontalSeamToRemove()
{
    int min = numeric_limits<int>::max();
    int minIndex;
    for (int i = 0; i < m_numRows; i++)
    {
        if (m_cumulativeEnergyMatrixHorizontal[i][m_numCols - 1] < min)
        {
            minIndex = i;
            min = m_cumulativeEnergyMatrixHorizontal[i][m_numCols - 1];
        }
    }
    m_horizontalSeamToRemove.push_back(minIndex);

    for (int j = m_numCols - 2; j >= 0; j--)
    {
        int i = m_horizontalSeamToRemove.back();
        min = numeric_limits<int>::max();

        // if not top row && top left is smaller
        if (i != 0 && m_cumulativeEnergyMatrixHorizontal[i - 1][j] < min)
        {
            min = m_cumulativeEnergyMatrixHorizontal[i - 1][j];
            minIndex = i - 1;
        }

        // if left is smaller
        if (m_cumulativeEnergyMatrixHorizontal[i][j] < min)
        {
            min = m_cumulativeEnergyMatrixHorizontal[i][j];
            minIndex = i;
        }

        // if not bot row && bottom left is smaller
        if (i != m_numRows - 1 && m_cumulativeEnergyMatrixHorizontal[i + 1][j] < min)
        {
            min = m_cumulativeEnergyMatrixHorizontal[i + 1][j];
            minIndex = i + 1;
        }
        m_horizontalSeamToRemove.push_back(minIndex);
    }
    reverse(m_horizontalSeamToRemove.begin(), m_horizontalSeamToRemove.end());
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
    cout << endl;
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
    cout << endl;
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
    cout << endl;
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
    cout << endl;
}

void SeamCarver::printVerticalSeamToRemove()
{
    for (int i = 0; i < m_verticalSeamToRemove.size(); i++)
    {
        cout << m_verticalSeamToRemove[i] << " ";
    }
    cout << endl << endl;
}

void SeamCarver::printHorizontalSeamToRemove()
{
    for (int i = 0; i < m_horizontalSeamToRemove.size(); i++)
    {
        cout << m_horizontalSeamToRemove[i] << " ";
    }
    cout << endl << endl;
}

void SeamCarver::writeImageToFile()
{
    // get index of last period to remove .pgm
    size_t lastIndex = m_fileName.find_last_of(".");

    string newFileName = m_fileName.substr(0, lastIndex);

    ofstream ofs(newFileName + "_processed.pgm");
    ofs << "P2" << endl;
    ofs << "# " << newFileName << "_processed" << endl;
    ofs << m_numCols << " " << m_numRows << endl;
    ofs << 255 << endl;
    for (int i = 0; i < m_numRows; i++)
    {
        for (int j = 0; j < m_numCols; j++)
        {
            ofs << m_image[i][j];
            if (j != m_numCols - 1)
            {
                ofs << " ";
            }
        }
        ofs << endl;
    }
}
