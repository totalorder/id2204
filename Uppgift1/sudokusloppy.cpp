#include <gecode/int.hh>
#include <gecode/driver.hh>
#include <gecode/minimodel.hh>

//The file including the examples
#include "boards.cpp"

using namespace Gecode;

class Sudoku : public Space {
protected:
    IntVarArray matrixData; //values in the matrix
public:
    //Constructor
    Sudoku(int board[9][9]) :
            matrixData(*this, 9*9, 1, 9) {

        /*Create the matrix interface for the array, this way we can use matrix.row and matrix.col
          for easy access to columns and rows
        */
        Matrix <IntVarArray> matrix(matrixData, 9, 9);

        //Constraints for rows and columns
        for (int rowIndex = 0; rowIndex < 9; rowIndex++) {
            distinct(*this, matrix.row(rowIndex));
            distinct(*this, matrix.col(rowIndex));
        }
        //Constraints for blocks
        for (int rowIndex = 0; rowIndex < 9; rowIndex +=3) {
            for (int colIndex = 0; colIndex < 9; colIndex +=3) {
                distinct(*this, matrix.slice(rowIndex, rowIndex+3, colIndex, colIndex+3));
            }
        }
        //"Fill" in the predefined values by setting an EQUALS constraint.
        for(int rowIndex = 0; rowIndex < 9; rowIndex++) {
            for (int colIndex = 0; colIndex < 9; colIndex++) {
                int matrixValue = board[rowIndex][colIndex];
                if(matrixValue != 0) {
                    rel(*this, matrix(colIndex, rowIndex), IRT_EQ, matrixValue);
                }
            }
        }

        //branching
        branch(*this, matrixData, INT_VAR_NONE(), INT_VAL_MIN());
    }
    // Copy constructor
    Sudoku(bool share, Sudoku& s) : Space(share, s) {
        matrixData.update(*this, share, s.matrixData);
    }

    // Copy method
    virtual Space* copy(bool share) {
        return new Sudoku(share, *this);
    }

    // Print the solution as a matrix
    void print(void) const {
        for (int rowIdx = 0; rowIdx < 9; rowIdx++) {
            bool isHorizontal = (rowIdx + 1) % 3 == 0 && rowIdx < 8;
            for (int colIdx = 0; colIdx < 9; colIdx++) {
                IntVar cellValue = matrixData[colIdx + rowIdx * 9];
                std::cout << cellValue;
                bool isVertical = (colIdx + 1) % 3 == 0 && colIdx < 8;
                if (isVertical) {
                    std::cout << " | ";
                } else {
                    std::cout << "   ";
                }
            }

            std::cout << std::endl;
            if (isHorizontal) {
                std::cout << "----------+-----------+------------" << std::endl;
            }
        }
    }
};

// Find and print solution for the given board
void solveBoard(int board[9][9]) {
    // Create the model and search engine
    Sudoku* m = new Sudoku(board);
    DFS<Sudoku> e(m);
    delete m;

    // Search for and and print the first solution
    while (Sudoku* s = e.next()) {
        s->print(); delete s;
        return;
    }
}

// Main function
// Loop over all examples defined in "boards.cpp", solving
int main(int argc, char* argv[]) {
    int numExamples = sizeof(examples) / sizeof(examples[0]);
    for (int boardIdx = 0; boardIdx < numExamples; boardIdx++) {
        std::cout << std::endl;
        std::cout << "Example idx " << boardIdx << ":" << std::endl;
        solveBoard(examples[boardIdx]);
    }



    return 0;
}
