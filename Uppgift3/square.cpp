#include <gecode/driver.hh>
#include "no-overlap.cpp"

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
    /// Propagation to use for model
    enum {
        PROP_DEFAULT, /// Use default propagation
        PROP_SPECIAL_NO_OVERLAP_PROPAGATOR,   /// Use special no-overlap propagator
    };

    // Hardcoded number of squares
    static const int size(int i) {
        return N - i;
    }

    int smallestSide(int n) {
        int sumOfSquares = 0;
        for (int i = 1; i < n + 1; i++) {
            sumOfSquares += i * i;
        }
        return (int) std::floor(std::sqrt(sumOfSquares));
    }

    int longestSide(int n) {
        return n * (n + 1) / 2;
    }

    void forbidDistanceFromBorder(int squareIndex, int distanceFromBorder) {
        rel(*this, xCoords[squareIndex] != distanceFromBorder);
        rel(*this, yCoords[squareIndex] != distanceFromBorder);
    }

    // Constructor
    Square(const SizeOptions &opt)
            : Script(opt),
              sizeOfSquare(*this, smallestSide(N), longestSide(N)),
              xCoords(*this, N - 1, 0, sizeOfSquare.max() - 1),
              yCoords(*this, N - 1, 0, sizeOfSquare.max() - 1) {

        // Constraint for "lower-right corner" to make sure the squares fit.
        for (int i = 0; i < N - 1; i++) {
            rel(*this, yCoords[i] <= sizeOfSquare - size(i));
            rel(*this, xCoords[i] <= sizeOfSquare - size(i));
        }

        //Symmetry-break constraint for largest square
        dom(*this, xCoords[0], 0, (int) (1 + std::floor((sizeOfSquare.max() -  size(0)) / 2)));
        rel(*this, yCoords[0] <= xCoords[0]);

        // Remove forbidden gaps from borders due to dominance
        for (int i = 0; i < N - 1; i++) {
            int s = size(i);
            if (s == 45) {
                forbidDistanceFromBorder(i, 10);
            } else if (s >= 34) {
                forbidDistanceFromBorder(i, 9);
            } else if (s >= 30) {
                forbidDistanceFromBorder(i, 8);
            } else if (s >= 22) {
                forbidDistanceFromBorder(i, 7);
            } else if (s >= 18) {
                forbidDistanceFromBorder(i, 6);
            } else if (s >= 12) {
                forbidDistanceFromBorder(i, 5);
            } else if (s >= 9) {
                forbidDistanceFromBorder(i, 4);
            } else if (s >= 5) {
                forbidDistanceFromBorder(i, 3);
            } else if (s >= 4) {
                forbidDistanceFromBorder(i, 2);
            } else if (s >= 3) {
                forbidDistanceFromBorder(i, 2);
                forbidDistanceFromBorder(i, 3);
            } else if (s >= 2) {
                forbidDistanceFromBorder(i, 1);
                forbidDistanceFromBorder(i, 2);
            }
        }

        IntArgs sizesOfSquares(N - 1);
        for (int i = 0; i < N - 1; i++) {
            sizesOfSquares[i] = size(i);
        }

        if (opt.propagation() == PROP_SPECIAL_NO_OVERLAP_PROPAGATOR) {
            // Constraint for non-overlapping squares.
            no_overlap(*this, xCoords, sizesOfSquares, yCoords, sizesOfSquares);
        } else {
            // Constraint for non-overlapping squares.
            for (int square = 0; square < N - 1; square++) {
                // Cannot overlap with any smaller squares
                // Since larger squares have already been checked in the previous
                // runs of the loop they are unnecessary to check again.
                for (int otherSquare = square + 1; otherSquare < N - 1; otherSquare++) {
                    BoolVar squareLeftOfOtherSquare(*this, 0, 1);
                    BoolVar otherSquareLeftOfSquare(*this, 0, 1);

                    BoolVar squareTopOfOtherSquare(*this, 0, 1);
                    BoolVar otherSquareTopOfSquare(*this, 0, 1);
                    // The right edge of square must be left of the left edge of otherSquare
                    rel(*this, (xCoords[square] + size(square) <= xCoords[otherSquare]) == squareLeftOfOtherSquare);

                    // The right edge of otherSquare must be left of the left edge of square
                    rel(*this,
                        (xCoords[otherSquare] + size(otherSquare) <= xCoords[square] == otherSquareLeftOfSquare));

                    // The bottom edge of square  must be above the top edge of otherSquare
                    rel(*this, (yCoords[square] + size(square) <= yCoords[otherSquare]) == squareTopOfOtherSquare);

                    // The bottom edge of otherSquare must be above the top edge of square
                    rel(*this, (yCoords[otherSquare] + size(otherSquare) <= yCoords[square] == otherSquareTopOfSquare));


                    rel(*this, squareTopOfOtherSquare + otherSquareTopOfSquare + squareLeftOfOtherSquare +
                               otherSquareLeftOfSquare == 1);
                }
            }
        }

        // Constraint for columns of x coordinates
        for (int outer = 0; outer < N - 1; outer++) {
            BoolVarArgs reifiedRows(*this, N - 1, 0, 1);
            BoolVarArgs reifiedColumns(*this, N - 1, 0, 1);

            for (int inner = 0; inner < N - 1; inner++) {
                // The x coordinate of all squares must be between
                // the column index - size of square + 1 and column index
                dom(*this, xCoords[inner], outer - size(inner) + 1, outer, reifiedRows[inner]);

                // The y coordinate of all squares must be between
                // the row index - size of square + 1 and row index
                dom(*this, yCoords[inner], outer - size(inner) + 1, outer, reifiedColumns[inner]);
            }

            // The sum of the products of sizesOfSquares[i] and reifiedColumns[i] must be less than or equal to the size of the enclosing square.
            linear(*this, sizesOfSquares, reifiedColumns, IRT_LQ, sizeOfSquare);

            // The sum of the products of sizesOfSquares[i] and reifiedRows[i] must be less than or equal to the size of the enclosing square.
            linear(*this, sizesOfSquares, reifiedRows, IRT_LQ, sizeOfSquare);
        }

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

        int matrix[64][64];

        for (int row = 0; row < sizeOfSquare.val(); row++) {
            for (int column = 0; column < sizeOfSquare.val(); column++) {
                matrix[row][column] = -1;
            }
        }

        for (int square = 0; square < N - 1; square++) {
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
    opt.ipl(IPL_DOM);
    opt.solutions(1);
    opt.parse(argc,argv);
    opt.propagation(Square::PROP_SPECIAL_NO_OVERLAP_PROPAGATOR, "special",
                    "special no-overlap-propagator");
    opt.propagation(Square::PROP_SPECIAL_NO_OVERLAP_PROPAGATOR);

    Script::run<Square, DFS, SizeOptions>(opt);
}
