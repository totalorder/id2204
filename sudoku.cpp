#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/gist.hh>
#include <gecode/driver.hh>

#include "boards.cpp"

using namespace Gecode;

class Sudoku : public Space {
    protected:
        // Array with int for each cell
        IntVarArray matrixData;

    public:
        Sudoku(int board[9][9]):
                // Initialize to size 9*9, one for each cell in the puzzle,
                // with values between 1-9
                matrixData(*this, 9*9, 1, 9) {

            // Create a matrix representing the board, backed by matrixData
            // of size 9x9
            Matrix<IntVarArray> m(matrixData, 9, 9);

            // Unique constraint for each 3x3 square
            // Find the top-left corner of each 3x3 square, and add a distinct constraint
            // on the 3x3 matrix slice starting from there
            for (int rowIdx = 0; rowIdx < 9; rowIdx = rowIdx + 3) {
                for (int colIdx = 0; colIdx < 9; colIdx = colIdx + 3) {
                    distinct(*this, m.slice(colIdx, colIdx+3, rowIdx, rowIdx+3));
                }
            }

            // Unique constraint for rows, columns + given values equality constraints
            for (int rowIdx = 0; rowIdx < 9; rowIdx++) {
                // Apply unique constraints for each row and column
                distinct(*this, m.row(rowIdx));
                distinct(*this, m.col(rowIdx));

                // Apply equality constraints for all given non-zero values in the board
                for (int colIdx = 0; colIdx < 9; colIdx++) {
                    int cellValue = board[rowIdx][colIdx];
                    if (cellValue != 0) {
                        rel(*this, m(colIdx, rowIdx), IRT_EQ, cellValue);
                    }
                }
            }

            // Branch on variables in the given order, starting with the smallest values
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