#include <gecode/driver.hh>
//#include <gecode/int.hh>
//#include <gecode/minimodel.hh>

using namespace Gecode;

const int N = 6;

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
    static const int size(int i) {
        return N - i;
    }

    int smallestSide(int n) {
        // s = lambda n: math.floor(math.sqrt((n*(n-1))*(2*n+1)/6))
        // q = lambda n: math.floor(math.sqrt(sum(n*n for n in range(n+1))))
        int sumOfSquares = 0;
        for (int i = 1; i < n + 1; i++) {
            sumOfSquares += i * i;
        }
        return (int) std::floor(std::sqrt(sumOfSquares));
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
//            rel(*this, xCoords[i], IRT_LQ, sizeOfSquare.max() - size(i));
//            rel(*this, yCoords[i], IRT_LQ, sizeOfSquare.max() - size(i));
            rel(*this, yCoords[i] <= sizeOfSquare - size(i));
            rel(*this, xCoords[i] <= sizeOfSquare - size(i));
        }

        //Symmetry-break constraint for largest square
        dom(*this, xCoords[0], 0, (int) (1 + std::floor((sizeOfSquare.max() -  size(0)) / 2)));
        rel(*this, yCoords[0] <= xCoords[0]);

        // Remove forbidden gaps from borders due to dominance
        for (int i = 0; i < N; i++) {
            int s = size(i);
            int forbiddenDistanceFromBorder = -1;
            if (s == 45) {
                forbiddenDistanceFromBorder = 10;
            } else if (s >= 34) {
                forbiddenDistanceFromBorder = 9;
            } else if (s >= 30) {
                forbiddenDistanceFromBorder = 8;
            } else if (s >= 22) {
                forbiddenDistanceFromBorder = 7;
            } else if (s >= 18) {
                forbiddenDistanceFromBorder = 6;
            } else if (s >= 12) {
                s = 5;
            } else if (s >= 9) {
                s = 4;
            } else if (s >= 5) {
                s = 3;
            } else if (s >= 4) {
                s = 2;
            } else if (s >= 3) {
                s = 2;
                // Apply specific domain 2
                rel(*this, xCoords[i], IRT_NQ, 3);
                rel(*this, xCoords[i] + s + 3 != sizeOfSquare);
                rel(*this, yCoords[i], IRT_NQ, 3);
                rel(*this, yCoords[i] + s + 3 != sizeOfSquare);
            } else if (s >= 2) {
                s = 1;
                // Apply specific domain 2
                rel(*this, xCoords[i], IRT_NQ, 2);
                rel(*this, xCoords[i] + s + 2 != sizeOfSquare);
                rel(*this, yCoords[i], IRT_NQ, 2);
                rel(*this, yCoords[i] + s + 3 != sizeOfSquare);
            }

            rel(*this, xCoords[i], IRT_NQ, forbiddenDistanceFromBorder);
            rel(*this, xCoords[i] + s + forbiddenDistanceFromBorder != sizeOfSquare);
            rel(*this, yCoords[i], IRT_NQ, forbiddenDistanceFromBorder);
            rel(*this, yCoords[i] + s + forbiddenDistanceFromBorder != sizeOfSquare);
        }

        // Constraint for non-overlapping squares.
        for (int square = 0; square < N; square++) {
            // Cannot overlap with any smaller squares
            // Since larger squares have already been checked in the previous
            // runs of the loop they are unnecessary to check again.
            for (int otherSquare = square + 1; otherSquare < N; otherSquare++) {
                BoolVar squareLeftOfOtherSquare(*this, 0, 1);
                BoolVar otherSquareLeftOfSquare(*this, 0, 1);

                BoolVar squareTopOfOtherSquare(*this, 0, 1);
                BoolVar otherSquareTopOfSquare(*this, 0, 1);
//                BoolVar square(*this, 0, 1);
                // The right edge of square must be left of the left edge of otherSquare
                rel(*this, (xCoords[square] + size(square) <= xCoords[otherSquare]) == squareLeftOfOtherSquare);
//                rel(*this, (xCoords[square] + size(square) <= xCoords[otherSquare]));

                // The right edge of otherSquare must be left of the left edge of square
//                rel(*this, (xCoords[otherSquare] + size(otherSquare) <= xCoords[square]));
                rel(*this, (xCoords[otherSquare] + size(otherSquare) <= xCoords[square] == otherSquareLeftOfSquare));

//                rel(*this, squareLeftOfOtherSquare + otherSquareLeftOfSquare == 1);

                // The bottom edge of square  must be above the top edge of otherSquare
//                rel(*this, (yCoords[square] + size(square) <= yCoords[otherSquare]));
                rel(*this, (yCoords[square] + size(square) <= yCoords[otherSquare]) == squareTopOfOtherSquare);

                // The bottom edge of otherSquare must be above the top edge of square
//                rel(*this, (yCoords[otherSquare] + size(otherSquare) <= yCoords[square]));
                rel(*this, (yCoords[otherSquare] + size(otherSquare) <= yCoords[square] == otherSquareTopOfSquare));


                rel(*this, squareTopOfOtherSquare + otherSquareTopOfSquare + squareLeftOfOtherSquare + otherSquareLeftOfSquare == 1);
            }
        }

        IntArgs sizesOfSquares(N);
        for (int i = 0; i < N; i++) {
            sizesOfSquares[i] = size(i);
        }

        // TODO: Fix the column and row reified propagators described in #3 in instructions
        // Constraint for columns of x coordinates
        for (int outer = 0; outer < N; outer++) {
            BoolVarArgs reifiedRows(*this, N, 0, 1);
            BoolVarArgs reifiedColumns(*this, N, 0, 1);

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

            //the sum of the products of sizesOfSquares[i] and reifiedColumns[i] must be less than or equal to the size of the enclosing square.
            linear(*this, sizesOfSquares, reifiedColumns, IRT_LQ, sizeOfSquare);

            //the sum of the products of sizesOfSquares[i] and reifiedRows[i] must be less than or equal to the size of the enclosing square.
            linear(*this, sizesOfSquares, reifiedRows, IRT_LQ, sizeOfSquare);
        }

        //TODO write a good branching heuristic
        branch(*this, sizeOfSquare, INT_VAL_MIN());
        branch(*this, xCoords, INT_VAR_MIN_MAX(), INT_VAL_MIN());
        branch(*this, yCoords, INT_VAR_MIN_MAX(), INT_VAL_MIN());
    }

    // Copy constructor
    Square(bool share, Square &s) : Script(share, s) {
        sizeOfSquare.update(*this, share, s.sizeOfSquare);
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
        os << "x-coordinates: " << xCoords << std::endl;
        os << "y-coordinates: " << yCoords << std::endl;
        os << "size of enclosing square: " << sizeOfSquare << std::endl;
        os << std::endl;
//
//        int *matrix = new int[sizeOfSquare.val() * sizeOfSquare.val()];
//        int matrix[sizeOfSquare.val()][sizeOfSquare.val()];
//        int matrix[][] = malloc(sizeOfSquare.val() * sizeOfSquare.val());
        int matrix[15][15];


        for (int row = 0; row < sizeOfSquare.val(); row++) {
            for (int column = 0; column < sizeOfSquare.val(); column++) {
                matrix[row][column] = -1;
            }
        }

        for (int square = 0; square < N; square++) {
            for (int row = yCoords[square].val(); row < yCoords[square].val() + size(square); row++) {
                for (int column = xCoords[square].val(); column < xCoords[square].val() + size(square); column++) {
                    matrix[row][column] = square;
                }
            }
        }

        for (int row = 0; row < sizeOfSquare.val(); row++) {
            for (int column = 0; column < sizeOfSquare.val(); column++) {
                if (matrix[row][column] == -1) {
                    os << " " << " ";
                } else {
                    os << matrix[row][column] << " ";
                }
            }
            os << std::endl;
        }
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
