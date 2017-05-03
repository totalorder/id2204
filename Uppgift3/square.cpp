#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

using namespace Gecode;

class Square : public Script {
protected:
    //the size of the surrounding square
    IntVar sizeOfSquare;
    //the x-coordinates for the packed squares
    IntVarArray xCoords;
    //the y-coordinates for the packed squares
    IntVarArray yCoords;
public:

    //Hardcoded number of squares
    const int n = 30;

    int size(int i) {
        return n-i;
    }
    //Constructor
    Square(const SizeOptions& opt)
            : Script(opt),
         sizeOfSquare(*this, std::floor(std::sqrt((n*(n-1))*(2*n+1)/6)), (n*(n+1)/2)),
         xCoords(*this, n ,0, sizeOfSquare.max() - 1),
         yCoords(*this, n ,0, sizeOfSquare.max() - 1) {

        //Constraint for "lower-right corner" to make sure the squares fit.
        for (int i = 0; i < n; i++) {
            rel(*this, xCoords[i], IRT_LQ, sizeOfSquare.max() - (n-i));
            rel(*this, yCoords[i], IRT_LQ, sizeOfSquare.max() - (n-i));
        }
//TODO control that these constraints are correct
        //Constraint for non-overlapping squares.
        for (int square = 0; square < n; square++) {
            for (int otherSquare = square+1; otherSquare <= n; otherSquare++) {
                BoolVar b(*this, 0, 1);
                rel(*this, (xCoords[square] + (n-square) <= xCoords[otherSquare]) == b);
                rel(*this, (xCoords[otherSquare] + (n-otherSquare) <= xCoords[square] == b));
                rel(*this, (yCoords[square] + (n-square) <= yCoords[otherSquare]) == b);
                rel(*this, (yCoords[otherSquare] + (n-otherSquare) <= yCoords[square] == b));
            }
        }


//TODO fix the column and row reified propagators described in #3 in instructions
        //Constraint for columns of x coordinates
        for (int cx = 0; cx < sizeOfSquare.max(); cx++) {
            BoolVarArgs bx(*this,sizeOfSquare.max(),0,1);
            for (int i=0; i<sizeOfSquare.max(); i++)
                //xCoords[i] must be less than cx-(n-i)+1 and bigger than cx
                dom(*this, xCoords[i], cx-(n-i)+1, cx, bx[i]);
            linear(*this, n, bx, IRT_EQ, sizeOfSquare);
        }

//TODO write a good branching heuristic
        branch(*this, xCoords, INT_VAR_MIN_MIN(), INT_VAL_MIN());
        branch(*this, yCoords, INT_VAR_MIN_MIN(), INT_VAL_MIN());

        /// Constructor

//TODO figure out why the compiler is whining about this, it should be correct.
    // Copy constructor
    Square(bool share, Square& s) : Script(share, s) {
        xCoords.update(*this, share, s.xCoords);
        yCoords.update(*this, share, s.yCoords);
        }

    // Copy method
    virtual Space* copy(bool share) {
        return new Square(share, *this);
    }

    /// Print solution


int main(int argc, char* argv[]) {
    SizeOptions opt("Square");
    opt.size(0);
    opt.ipl(IPL_DOM);
    opt.solutions(0);
    opt.parse(argc,argv);
    if (opt.size() < 1) {
        std::cerr << "Error: number of squares to place must be at least 1!"
                  << std::endl;
        return 1;
    }

    Script::run<Square,DFS,SizeOptions>(opt);
}
