#include <gecode/int.hh>
#include <gecode/driver.hh>

using namespace Gecode;

namespace {
    static int examples[][9][9] = {
            {
                    {0,0,0, 2,0,5, 0,0,0},
                    {0,9,0, 0,0,0, 7,3,0},
                    {0,0,2, 0,0,9, 0,6,0},
                    {2,0,0, 0,0,0, 4,0,9},
                    {0,0,0, 0,7,0, 0,0,0},
                    {6,0,9, 0,0,0, 0,0,1},
                    {0,8,0, 4,0,0, 1,0,0},
                    {0,6,3, 0,0,0, 0,8,0},
                    {0,0,0, 6,0,8, 0,0,0}
            },
            {
                    {3,0,0, 9,0,4, 0,0,1},
                    {0,0,2, 0,0,0, 4,0,0},
                    {0,6,1, 0,0,0, 7,9,0},
                    {6,0,0, 2,4,7, 0,0,5},
                    {0,0,0, 0,0,0, 0,0,0},
                    {2,0,0, 8,3,6, 0,0,4},
                    {0,4,6, 0,0,0, 2,3,0},
                    {0,0,9, 0,0,0, 6,0,0},
                    {5,0,0, 3,0,9, 0,0,8}
            },
            {
                    {0,0,0, 0,1,0, 0,0,0},
                    {3,0,1, 4,0,0, 8,6,0},
                    {9,0,0, 5,0,0, 2,0,0},
                    {7,0,0, 1,6,0, 0,0,0},
                    {0,2,0, 8,0,5, 0,1,0},
                    {0,0,0, 0,9,7, 0,0,4},
                    {0,4,8, 0,0,6, 9,0,7},
                    {0,0,0, 0,8,0, 0,0,0}
            },
            {	// Fiendish puzzle April 21, 2005 Times London
                    {0,0,4, 0,0,3, 0,7,0},
                    {0,8,0, 0,7,0, 0,0,0},
                    {0,7,0, 0,0,8, 2,0,5},
                    {4,0,0, 0,0,0, 3,1,0},
                    {9,0,0, 0,0,0, 0,0,8},
                    {0,1,5, 0,0,0, 0,0,4},
                    {1,0,6, 9,0,0, 0,3,0},
                    {0,0,0, 0,2,0, 0,6,0},
                    {0,2,0, 4,0,0, 5,0,0}
            },
            {	// This one requires search
                    {0,4,3, 0,8,0, 2,5,0},
                    {6,0,0, 0,0,0, 0,0,0},
                    {0,0,0, 0,0,1, 0,9,4},
                    {9,0,0, 0,0,4, 0,7,0},
                    {0,0,0, 6,0,8, 0,0,0},
                    {0,1,0, 2,0,0, 0,0,3},
                    {8,2,0, 5,0,0, 0,0,0},
                    {0,0,0, 0,0,0, 0,0,5},
                    {0,3,4, 0,9,0, 7,1,0}
            },
            {	// Hard one from http://www.cs.mu.oz.au/671/proj3/node5.html
                    {0,0,0, 0,0,3, 0,6,0},
                    {0,0,0, 0,0,0, 0,1,0},
                    {0,9,7, 5,0,0, 0,8,0},
                    {0,0,0, 0,9,0, 2,0,0},
                    {0,0,8, 0,7,0, 4,0,0},
                    {0,0,3, 0,6,0, 0,0,0},
                    {0,1,0, 0,0,2, 8,9,0},
                    {0,4,0, 0,0,0, 0,0,0},
                    {0,5,0, 1,0,0, 0,0,0}
            },
            { // Puzzle 1 from http://www.sudoku.org.uk/bifurcation.htm
                    {1,0,0, 9,0,7, 0,0,3},
                    {0,8,0, 0,0,0, 0,7,0},
                    {0,0,9, 0,0,0, 6,0,0},
                    {0,0,7, 2,0,9, 4,0,0},
                    {4,1,0, 0,0,0, 0,9,5},
                    {0,0,8, 5,0,4, 3,0,0},
                    {0,0,3, 0,0,0, 7,0,0},
                    {0,5,0, 0,0,0, 0,4,0},
                    {2,0,0, 8,0,6, 0,0,9}
            },
            { // Puzzle 2 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,0, 3,0,2, 0,0,0},
                    {0,5,0, 7,9,8, 0,3,0},
                    {0,0,7, 0,0,0, 8,0,0},
                    {0,0,8, 6,0,7, 3,0,0},
                    {0,7,0, 0,0,0, 0,6,0},
                    {0,0,3, 5,0,4, 1,0,0},
                    {0,0,5, 0,0,0, 6,0,0},
                    {0,2,0, 4,1,9, 0,5,0},
                    {0,0,0, 8,0,6, 0,0,0}
            },
            { // Puzzle 3 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,0, 8,0,0, 0,0,6},
                    {0,0,1, 6,2,0, 4,3,0},
                    {4,0,0, 0,7,1, 0,0,2},
                    {0,0,7, 2,0,0, 0,8,0},
                    {0,0,0, 0,1,0, 0,0,0},
                    {0,1,0, 0,0,6, 2,0,0},
                    {1,0,0, 7,3,0, 0,0,4},
                    {0,2,6, 0,4,8, 1,0,0},
                    {3,0,0, 0,0,5, 0,0,0}
            },
            { // Puzzle 4 from http://www.sudoku.org.uk/bifurcation.htm
                    {3,0,5, 0,0,4, 0,7,0},
                    {0,7,0, 0,0,0, 0,0,1},
                    {0,4,0, 9,0,0, 0,3,0},
                    {4,0,0, 0,5,1, 0,0,6},
                    {0,9,0, 0,0,0, 0,4,0},
                    {2,0,0, 8,4,0, 0,0,7},
                    {0,2,0, 0,0,7, 0,6,0},
                    {8,0,0, 0,0,0, 0,9,0},
                    {0,6,0, 4,0,0, 2,0,8}
            },
            { // Puzzle 5 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,0, 7,0,0, 3,0,0},
                    {0,6,0, 0,0,0, 5,7,0},
                    {0,7,3, 8,0,0, 4,1,0},
                    {0,0,9, 2,8,0, 0,0,0},
                    {5,0,0, 0,0,0, 0,0,9},
                    {0,0,0, 0,9,3, 6,0,0},
                    {0,9,8, 0,0,7, 1,5,0},
                    {0,5,4, 0,0,0, 0,6,0},
                    {0,0,1, 0,0,9, 0,0,0}
            },
            { // Puzzle 6 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,0, 6,0,0, 0,0,4},
                    {0,3,0, 0,9,0, 0,2,0},
                    {0,6,0, 8,0,0, 7,0,0},
                    {0,0,5, 0,6,0, 0,0,1},
                    {6,7,0, 3,0,1, 0,5,8},
                    {9,0,0, 0,5,0, 4,0,0},
                    {0,0,6, 0,0,3, 0,9,0},
                    {0,1,0, 0,8,0, 0,6,0},
                    {2,0,0, 0,0,6, 0,0,0}
            },
            { // Puzzle 7 from http://www.sudoku.org.uk/bifurcation.htm
                    {8,0,0, 0,0,1, 0,4,0},
                    {2,0,6, 0,9,0, 0,1,0},
                    {0,0,9, 0,0,6, 0,8,0},
                    {1,2,4, 0,0,0, 0,0,9},
                    {0,0,0, 0,0,0, 0,0,0},
                    {9,0,0, 0,0,0, 8,2,4},
                    {0,5,0, 4,0,0, 1,0,0},
                    {0,8,0, 0,7,0, 2,0,5},
                    {0,9,0, 5,0,0, 0,0,7}
            },
            { // Puzzle 8 from http://www.sudoku.org.uk/bifurcation.htm
                    {6,5,2, 0,4,8, 0,0,7},
                    {0,7,0, 2,0,5, 4,0,0},
                    {0,0,0, 0,0,0, 0,0,0},
                    {0,6,4, 1,0,0, 0,7,0},
                    {0,0,0, 0,8,0, 0,0,0},
                    {0,8,0, 0,0,4, 5,6,0},
                    {0,0,0, 0,0,0, 0,0,0},
                    {0,0,8, 6,0,7, 0,2,0},
                    {2,0,0, 8,9,0, 7,5,1}
            },
            { // Puzzle 9 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,6, 0,0,2, 0,0,9},
                    {1,0,0, 5,0,0, 0,2,0},
                    {0,4,7, 3,0,6, 0,0,1},
                    {0,0,0, 0,0,8, 0,4,0},
                    {0,3,0, 0,0,0, 0,7,0},
                    {0,1,0, 6,0,0, 0,0,0},
                    {4,0,0, 8,0,3, 2,1,0},
                    {0,6,0, 0,0,1, 0,0,4},
                    {3,0,0, 4,0,0, 9,0,0}
            },
            { // Puzzle 10 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,4, 0,5,0, 9,0,0},
                    {0,0,0, 0,7,0, 0,0,6},
                    {3,7,0, 0,0,0, 0,0,2},
                    {0,0,9, 5,0,0, 0,8,0},
                    {0,0,1, 2,0,4, 3,0,0},
                    {0,6,0, 0,0,9, 2,0,0},
                    {2,0,0, 0,0,0, 0,9,3},
                    {1,0,0, 0,4,0, 0,0,0},
                    {0,0,6, 0,2,0, 7,0,0}
            },
            { // Puzzle 11 from http://www.sudoku.org.uk/bifurcation.htm
                    {0,0,0, 0,3,0, 7,9,0},
                    {3,0,0, 0,0,0, 0,0,5},
                    {0,0,0, 4,0,7, 3,0,6},
                    {0,5,3, 0,9,4, 0,7,0},
                    {0,0,0, 0,7,0, 0,0,0},
                    {0,1,0, 8,2,0, 6,4,0},
                    {7,0,1, 9,0,8, 0,0,0},
                    {8,0,0, 0,0,0, 0,0,1},
                    {0,9,4, 0,1,0, 0,0,0}
            },
            { // From http://www.sudoku.org.uk/discus/messages/29/51.html?1131034031
                    {2,5,8, 1,0,4, 0,3,7},
                    {9,3,6, 8,2,7, 5,1,4},
                    {4,7,1, 5,3,0, 2,8,0},
                    {7,1,5, 2,0,3, 0,4,0},
                    {8,4,9, 6,7,5, 3,2,1},
                    {3,6,2, 4,1,0, 0,7,5},
                    {1,2,4, 9,0,0, 7,5,3},
                    {5,9,3, 7,4,2, 1,6,8},
                    {6,8,7, 3,5,1, 4,9,2}
            }
    };
}

class Sudoku : public Script {
protected:
    IntVarArray matrixData; //data in the matrix
public:
    // Branching variants
    enum {
        BRANCH_NONE,        ///< Use lexicographic ordering
        BRANCH_SIZE,        ///< Use minimum size
        BRANCH_SIZE_DEGREE, ///< Use minimum size over degree
        BRANCH_SIZE_AFC,    ///< Use minimum size over afc
        BRANCH_AFC          ///< Use maximum afc
    };

    //Constructor
    Sudoku(const SizeOptions& opt) : Script(opt),
                                     matrixData(*this, 9*9, 1, 9) {
        //Create the m interface of the array, to make use of .col and .row
        Matrix<IntVarArray> m(matrixData, 9, 9);

        // Constraints for rows and columns
        for (int rowIndex = 0; rowIndex < 9; rowIndex++) {
            distinct(*this, m.row(rowIndex), opt.ipl());
            distinct(*this, m.col(rowIndex), opt.ipl());
        }

        // Constraints for squares
        for (int rowIndex = 0; rowIndex < 9; rowIndex += 3) {
            for (int colIndex = 0; colIndex < 9; colIndex += 3) {
                distinct(*this, m.slice(rowIndex, rowIndex + 3, colIndex, colIndex + 3), opt.ipl());
            }
        }
        //Fill in predefined values
        for (int rowIndex = 0; rowIndex < 9; ++rowIndex) {
            for (int colIndex = 0; colIndex < 9; ++colIndex) {
              int cellValue = examples[opt.size()][rowIndex][colIndex];
                if(cellValue != 0) {
                    rel(*this, m(colIndex,rowIndex), IRT_EQ, cellValue);
                }
            }
        }

        //branching options
        if (opt.branching() == BRANCH_NONE) {
            branch(*this, matrixData, INT_VAR_NONE(), INT_VAL_SPLIT_MIN());
        } else if (opt.branching() == BRANCH_SIZE) {
            branch(*this, matrixData, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MIN());
        } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
            branch(*this, matrixData, INT_VAR_DEGREE_SIZE_MAX(), INT_VAL_SPLIT_MIN());
        } else if (opt.branching() == BRANCH_SIZE_AFC) {
            branch(*this, matrixData, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
        } else if (opt.branching() == BRANCH_AFC) {
            branch(*this, matrixData, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
        }
    }
    /// Constructor
    // Copy constructor
    Sudoku(bool share, Sudoku& s) : Script(share, s) {
        matrixData.update(*this, share, s.matrixData);
    }

    // Copy method
    virtual Space* copy(bool share) {
        return new Sudoku(share, *this);
    }

    /// Print solution
    virtual void
    print(std::ostream& os) const {
        os << "  ";
        for (int i = 0; i<81; i++) {
            if (matrixData[i].assigned()) {
                if (matrixData[i].val()<10)
                    os << matrixData[i] << " ";
                else
                    os << (char)(matrixData[i].val()+'A'-10) << " ";
            }
            else
                os << ". ";
            if((i+1)%(9) == 0)
                os << std::endl << "  ";
        }
        os << std::endl;
    }
};

    int main(int argc, char* argv[]) {
        SizeOptions opt("Sudoku");
        opt.size(0);
        opt.ipl(IPL_DOM);
        opt.solutions(0);
        opt.branching(Sudoku::BRANCH_SIZE_AFC);
        opt.branching(Sudoku::BRANCH_NONE, "none", "none");
        opt.branching(Sudoku::BRANCH_SIZE, "size", "min size");
        opt.branching(Sudoku::BRANCH_SIZE_DEGREE, "sizedeg", "min size over degree");
        opt.branching(Sudoku::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
        opt.branching(Sudoku::BRANCH_AFC, "afc", "maximum afc");
        opt.parse(argc,argv);
        Script::run<Sudoku,DFS,SizeOptions>(opt);
    }
