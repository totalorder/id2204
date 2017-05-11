#include <gecode/driver.hh>
#include <gecode/int.hh>

using namespace Gecode;
using namespace Gecode::Int;

//Number of squares to place
const int N = 6;

void no_overlap(Home home,
                const IntVarArgs& x, const IntArgs& w,
                const IntVarArgs& y, const IntArgs& h);

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
    //Comment out the line below to use Gecode's propagator for nooverlap.
    opt.propagation(Square::PROP_SPECIAL_NO_OVERLAP_PROPAGATOR);

    Script::run<Square, DFS, SizeOptions>(opt);
}

// The no-overlap propagator

/*
 *  Main author:
 *     Christian Schulte <cschulte@kth.se>
 *
 *  Copyright:
 *     Christian Schulte, 2009
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

class NoOverlap : public Propagator {
protected:
    // The x-coordinates
    ViewArray<IntView> x;
    // The width (array)
    int* w;
    // The y-coordinates
    ViewArray<IntView> y;
    // The heights (array)
    int* h;
public:
    // Create propagator and initialize
    NoOverlap(Home home,
              ViewArray<IntView>& x0, int w0[],
              ViewArray<IntView>& y0, int h0[])
            : Propagator(home), x(x0), w(w0), y(y0), h(h0) {
        x.subscribe(home,*this,PC_INT_BND);
        y.subscribe(home,*this,PC_INT_BND);
    }
    // Post no-overlap propagator
    static ExecStatus post(Home home,
                           ViewArray<IntView>& x, int w[],
                           ViewArray<IntView>& y, int h[]) {
        // Only if there is something to propagate
        if (x.size() > 1)
            (void) new (home) NoOverlap(home,x,w,y,h);
        return ES_OK;
    }

    // Copy constructor during cloning
    NoOverlap(Space& home, bool share, NoOverlap& p)
            : Propagator(home,share,p) {
        x.update(home,share,p.x);
        y.update(home,share,p.y);
        // Also copy width and height arrays
        w = home.alloc<int>(x.size());
        h = home.alloc<int>(y.size());
        for (int i=x.size(); i--; ) {
            w[i]=p.w[i]; h[i]=p.h[i];
        }
    }
    // Create copy during cloning
    virtual Propagator* copy(Space& home, bool share) {
        return new (home) NoOverlap(home,share,*this);
    }

    // Re-schedule function after propagator has been re-enabled
    virtual void reschedule(Space& home) {
        x.reschedule(home,*this,PC_INT_BND);
        y.reschedule(home,*this,PC_INT_BND);
    }

    // Return cost (defined as cheap quadratic)
    virtual PropCost cost(const Space&, const ModEventDelta&) const {
        return PropCost::quadratic(PropCost::LO,2*x.size());
    }

    bool isOverlapping(int a, int u) {
        bool isAssignedRightOfUnassigned = x[a].min() >= x[u].max() + w[u];
        bool isAssignedLeftOfUnassigned = x[a].max() + w[a] <= x[u].min();
        bool isAssignedUnderOfUnassigned = y[a].min() >= y[u].max() + h[u];
        bool isAssignedOverOfUnassigned = y[a].max() + h[a] <= y[u].min();
        bool isOverlappingHorizontally = !isAssignedRightOfUnassigned && !isAssignedLeftOfUnassigned;
        bool isOverlappingVertically = !isAssignedUnderOfUnassigned && !isAssignedOverOfUnassigned;
        return isOverlappingHorizontally && isOverlappingVertically;
    }

    // Perform propagation
    virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
        bool overlapFound = false;

        for (int a = 0; a < x.size(); a++) {
            for (int u = a + 1; u < x.size(); u++) {
                // Check if any squares are overlapping, for reporting subsumption
                if (isOverlapping(a, u)) {
                    overlapFound = true;
                }

                // When one square is assigned, and one axis of the other square is assigned
                // values are removed for the unassigned axis
                if(x[a].assigned() && y[a].assigned()) {
                    if (y[u].assigned()) {
                        bool isAssignedUnderOfUnassigned = y[a].val() >= y[u].val() + h[u];
                        bool isAssignedOverOfUnassigned = y[a].val() + h[a] <= y[u].val();
                        bool isOverlappingVertically = !isAssignedUnderOfUnassigned && !isAssignedOverOfUnassigned;

                        // If the squares are overlapping vertically, remove all values where
                        // they are overlapping horizontally
                        if (isOverlappingVertically) {
                            for (int i = x[a].val() - w[u] + 1; i < x[a].val() + w[a]; i++) {
                                GECODE_ME_CHECK(x[u].nq(home, i));
                            }
                        }
                    }

                    if (x[u].assigned()) {
                        bool isAssignedRightOfUnassigned = x[a].val() >= x[u].val() + w[u];
                        bool isAssignedLeftOfUnassigned = x[a].val() + w[a] <= x[u].val();
                        bool isOverlappingHorizontally = !isAssignedRightOfUnassigned && !isAssignedLeftOfUnassigned;
                        // If the squares are overlapping horizontally, remove all values where
                        // they are overlapping vertically
                        if (isOverlappingHorizontally) {
                            for (int i = y[a].val() - h[u] + 1; i < y[a].val() + h[a]; i++) {
                                GECODE_ME_CHECK(y[u].nq(home, i));
                            }
                        }
                    }
                }
            }
        }

        if (!overlapFound) {
            return home.ES_SUBSUMED(*this);
        }

        return ES_FIX;
    }

    // Dispose propagator and return its size
    virtual size_t dispose(Space& home) {
        x.cancel(home,*this,PC_INT_BND);
        y.cancel(home,*this,PC_INT_BND);
        (void) Propagator::dispose(home);
        return sizeof(*this);
    }
};

/*
 * Post the constraint that the rectangles defined by the coordinates
 * x and y and width w and height h do not overlap.
 *
 * This is the function that you will call from your model. The best
 * is to paste the entire file into your model.
 */
void no_overlap(Home home,
                const IntVarArgs& x, const IntArgs& w,
                const IntVarArgs& y, const IntArgs& h) {
    // Check whether the arguments make sense
    if ((x.size() != y.size()) || (x.size() != w.size()) ||
        (y.size() != h.size()))
        throw ArgumentSizeMismatch("nooverlap");
    // Never post a propagator in a failed space
    if (home.failed()) return;
    // Set up array of views for the coordinates
    ViewArray<IntView> vx(home,x);
    ViewArray<IntView> vy(home,y);
    // Set up arrays (allocated in home) for width and height and initialize
    int* wc = static_cast<Space&>(home).alloc<int>(x.size());
    int* hc = static_cast<Space&>(home).alloc<int>(y.size());
    for (int i=x.size(); i--; ) {
        wc[i]=w[i]; hc[i]=h[i];
    }
    // If posting failed, fail space
    if (NoOverlap::post(home,vx,wc,vy,hc) != ES_OK)
        home.fail();
}