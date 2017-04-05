/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  Last modified:
 *     $Date: 2016-04-19 17:19:45 +0200 (Tue, 19 Apr 2016) $ by $Author: schulte $
 *     $Revision: 14967 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#endif

using namespace Gecode;

/**
 * \brief %Example: n-%Queens puzzle
 *
 * Place n queens on an n times n chessboard such that they do not
 * attack each other.
 *
 * \ingroup Example
 *
 */
class Queens : public Script {
public:
  int size;
  IntVarArray matrixData;
  /// Propagation to use for model
  enum {
    PROP_BINARY,  ///< Use only binary disequality constraints
    PROP_MIXED,   ///< Use single distinct and binary disequality constraints
    PROP_DISTINCT ///< Use three distinct constraints
  };

  Queens(const SizeOptions& opt)
    : Script(opt),
      // Initialize to size 9*9, one for each cell in on the board,
      // with values in range 0-1
      matrixData(*this, opt.size()*opt.size(), 0, 1),
      size(opt.size()) {
    const int size = opt.size();

    // Create a matrix representing the board, backed by matrixData
    // of size*size
    Matrix<IntVarArray> matrix(matrixData, size, size);

    // Create constraints saying the sum of all values in each
    // row and column should be 1
    for (int rowIdx = 0; rowIdx < size; rowIdx++) {
      rel(*this, sum(matrix.row(rowIdx)) == 1);
      rel(*this, sum(matrix.col(rowIdx)) == 1);
    }

    // Create constraints saying the sum of all values in each diagonal
    // should be at most 1
    for (int diagonalIndex = -size+1; diagonalIndex < size; diagonalIndex++) {
      // First half the start point moves up along the 0th column
      int colIdx = std::max(diagonalIndex, 0);
      int rowIdx = std::max(-diagonalIndex, 0);
      int steps = size - 1 - std::abs(diagonalIndex);

      // Create linear expressions which are the sum of all cells
      // in each diagonal
      LinIntExpr sumOfDownDiagonal;
      LinIntExpr sumOfUpDiagonal;
      for (int idx = 0; idx <= steps; idx++) {
        // Going right-down of the start point
        IntVar downCellValue = matrix(colIdx + idx, rowIdx + idx);
        sumOfDownDiagonal = idx != 0 ? sumOfDownDiagonal + downCellValue : downCellValue;

        // Going right-up of the start point with inverted row
        IntVar upCellValue = matrix(colIdx + idx, size - 1 - (rowIdx + idx));
        sumOfUpDiagonal = idx != 0 ? sumOfUpDiagonal + upCellValue : upCellValue;
      }

      // Each diagonal can have a sum of at most 1
      rel(*this, sumOfDownDiagonal <= 1);
      rel(*this, sumOfUpDiagonal <= 1);
    }

    branch(*this, matrixData, INT_VAR_SIZE_MAX(), INT_VAL_MAX());
  }

  /// Constructor for cloning \a sS
  Queens(bool share, Queens& s, int size) : Script(share,s), size(size) {
    matrixData.update(*this, share, s.matrixData);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Queens(share, *this, size);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int rowIdx = 0; rowIdx < size; rowIdx++) {
      for (int colIdx = 0; colIdx < size; colIdx++) {
        IntVar cellValue = matrixData[colIdx + rowIdx * size];
        os << cellValue << " ";
      }
      os << std::endl;
    }
  }
};

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
/// Inspector showing queens on a chess board
class QueensInspector : public Gist::Inspector {
protected:
  /// The graphics scene displaying the board
  QGraphicsScene* scene;
  /// The window containing the graphics scene
  QMainWindow* mw;
  /// The size of a field on the board
  static const int unit = 20;
public:
  /// Constructor
  QueensInspector(void) : scene(NULL), mw(NULL) {}
  /// Inspect space \a s
  virtual void inspect(const Space& s) {
    const Queens& q = static_cast<const Queens&>(s);

    if (!scene)
      initialize();
    QList <QGraphicsItem*> itemList = scene->items();
    foreach (QGraphicsItem* i, scene->items()) {
      scene->removeItem(i);
      delete i;
    }

    for (int i=0; i<q.matrixData.size(); i++) {
      for (int j=0; j<q.matrixData.size(); j++) {
        scene->addRect(i*unit,j*unit,unit,unit);
      }
      QBrush b(q.matrixData[i].assigned() ? Qt::black : Qt::red);
      QPen p(q.matrixData[i].assigned() ? Qt::black : Qt::white);
      for (IntVarValues xv(q.matrixData[i]); xv(); ++xv) {
        scene->addEllipse(QRectF(i*unit+unit/4,xv.val()*unit+unit/4,
                                 unit/2,unit/2), p, b);
      }
    }
    mw->show();
  }

  /// Set up main window
  void initialize(void) {
    mw = new QMainWindow();
    scene = new QGraphicsScene();
    QGraphicsView* view = new QGraphicsView(scene);
    view->setRenderHints(QPainter::Antialiasing);
    mw->setCentralWidget(view);
    mw->setAttribute(Qt::WA_QuitOnClose, false);
    mw->setAttribute(Qt::WA_DeleteOnClose, false);
    QAction* closeWindow = new QAction("Close window", mw);
    closeWindow->setShortcut(QKeySequence("Ctrl+W"));
    mw->connect(closeWindow, SIGNAL(triggered()),
                mw, SLOT(close()));
    mw->addAction(closeWindow);
  }

  /// Name of the inspector
  virtual std::string name(void) { return "Board"; }
  /// Finalize inspector
  virtual void finalize(void) {
    delete mw;
    mw = NULL;
  }
};

#endif /* GECODE_HAS_GIST */

/** \brief Main-function
 *  \relates Queens
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Queens");
  opt.iterations(500);
  opt.size(9);
  opt.propagation(Queens::PROP_DISTINCT);
  opt.propagation(Queens::PROP_BINARY, "binary",
                      "only binary disequality constraints");
  opt.propagation(Queens::PROP_MIXED, "mixed",
                      "single distinct and binary disequality constraints");
  opt.propagation(Queens::PROP_DISTINCT, "distinct",
                      "three distinct constraints");

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)

  QueensInspector ki;
  opt.inspect.click(&ki);
#endif

  opt.parse(argc,argv);
  Script::run<Queens,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

