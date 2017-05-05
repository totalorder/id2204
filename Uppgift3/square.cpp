#include <gecode/driver.hh>
//#include <gecode/int.hh>
//#include <gecode/minimodel.hh>

using namespace Gecode;

const int N = 5;

class Square : public Script {
protected:
    // the size of the surrounding square
    IntVar sizeOfSquare;
    // the x-coordinates for the packed squares
    IntVarArray xCoords;
    // the y-coordinates for the packed squares
    IntVarArray yCoords;
public:

    // Hardcoded number of squares
    int size(int i) {
        return N - i;
    }

    int smallestSide(int n) {
        // s = lambda n: math.floor(math.sqrt((n*(n-1))*(2*n+1)/6))
        // q = lambda n: math.floor(math.sqrt(sum(n*n for n in range(n+1))))
        int sumOfSquares = 0;
        for (int i = 1; i < n + 1; i++) {
            sumOfSquares += i * i;
        }
        return std::floor(std::sqrt(sumOfSquares));
    }

    int longestSide(int n) {
        return n * (n + 1) / 2;
    }

    // Constructor
    Square(const SizeOptions &opt)
            : Script(opt),
              sizeOfSquare(*this, smallestSide(N), longestSide(N)),
              xCoords(*this, N, 0, sizeOfSquare.max() - 1),
              yCoords(*this, N, 0, sizeOfSquare.max() - 1) {

        // Constraint for "lower-right corner" to make sure the squares fit.
        for (int i = 0; i < N; i++) {
            rel(*this, xCoords[i], IRT_LQ, sizeOfSquare.max() - size(i));
            rel(*this, yCoords[i], IRT_LQ, sizeOfSquare.max() - size(i));
        }
        // Constraint for non-overlapping squares.
        for (int square = 0; square < N; square++) {
            // Cannot overlap with any smaller squares
            // Since larger squares have already been checked in the previous
            // runs of the loop they are unnecessary to check again.
            for (int otherSquare = square + 1; otherSquare < N; otherSquare++) {
                BoolVar b(*this, 0, 1);
                // The right edge of square must be left of the left edge of otherSquare 
                rel(*this, (xCoords[square] + size(square) <= xCoords[otherSquare]) == b);
                
                // The right edge of otherSquare must be left of the left edge of square
                rel(*this, (xCoords[otherSquare] + size(otherSquare) <= xCoords[square] == b));
                
                // The bottom edge of square  must be above the top edge of otherSquare
                rel(*this, (yCoords[square] + size(square) <= yCoords[otherSquare]) == b);

                // The bottom edge of otherSquare must be above the top edge of square
                rel(*this, (yCoords[otherSquare] + size(otherSquare) <= yCoords[square] == b));
            }
        }

        // TODO: Fix the column and row reified propagators described in #3 in instructions
        // Constraint for columns of x coordinates
        for (int outer = 0; outer < N; outer++) {
            BoolVarArgs reifiedRows(*this, sizeOfSquare.max(), 0, 1);
            BoolVarArgs reifiedColumns(*this, sizeOfSquare.max(), 0, 1);
//            IntArgs rowsSquareSizes(n);
            for (int inner = 0; inner < N; inner++) {
                // The x coordinate of all squares must be between
                // the column index - size of square + 1 and column index
                // xCoords[inner] must be less than outer-(n-inner)+1 and bigger than outer
                dom(*this, xCoords[inner], outer - size(inner) + 1, outer, reifiedRows[inner]);
//                rowsSquareSizes[inner] = xCoords[inner];

                // The y coordinate of all squares must be between
                // the row index - size of square + 1 and row index
                dom(*this, yCoords[inner], outer - size(inner) + 1, outer, reifiedColumns[inner]);

            }

            // The sum of the sizes of all squares in column must be less or
            // equal to the size of square
//            linear(*this, n, bx, IRT_EQ, sizeOfSquare);
        }

        //TODO write a good branching heuristic
        branch(*this, xCoords, INT_VAR_MIN_MIN(), INT_VAL_MIN());
        branch(*this, yCoords, INT_VAR_MIN_MIN(), INT_VAL_MIN());
    }

    // Copy constructor
    Square(bool share, Square &s) : Script(share, s) {
        xCoords.update(*this, share, s.xCoords);
        yCoords.update(*this, share, s.yCoords);
    }

    // Copy method
    virtual Space *copy(bool share) {
        return new Square(share, *this);
    }

    /// Print solution
    virtual void
    print(std::ostream& os) const {
        os << "x: " << xCoords << std::endl;
        os << "y: " << xCoords << std::endl;
        os << "s: " << sizeOfSquare << std::endl;
    }
};

int main(int argc, char* argv[]) {
    SizeOptions opt("Square");
    opt.size(5);
    opt.ipl(IPL_DOM);
    opt.solutions(1);
    opt.parse(argc,argv);
    if (opt.size() < 1) {
        std::cerr << "Error: number of squares to place must be at least 1!"
                  << std::endl;
        return 1;
    }

    Script::run<Square, DFS, SizeOptions>(opt);
}
