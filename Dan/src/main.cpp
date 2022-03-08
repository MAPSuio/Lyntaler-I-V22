#include "clang/AST/ASTContext.h"
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/PrecompiledPreamble.h"
#include "clang/Frontend/TextDiagnostic.h"
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm-10/llvm/ADT/StringRef.h>
#include <llvm-10/llvm/Support/SourceMgr.h>
#include <llvm-10/llvm/Support/raw_ostream.h>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"

#include <memory>
#include <sstream>

using namespace llvm;
using namespace clang;
using namespace clang::tooling;
using namespace clang;
using namespace clang::ast_matchers;

using namespace std;

class IfCallBack : public MatchFinder::MatchCallback {
 public:
  void run( const MatchFinder::MatchResult &r ) override {
    const IfStmt *sif = r.Nodes.getNodeAs<IfStmt>( "single_if" );  // Extract match node

    ASTContext *ctx = r.Context;  // Information about the AST translation not stored in the nodes

    FullSourceLoc fullLocation = ctx->getFullLoc( sif->getBeginLoc() );

    stringstream ss;

    if ( fullLocation.isValid() ) {
      ss << ":" << fullLocation.getSpellingLineNumber()
         << ":" << fullLocation.getSpellingColumnNumber()
         << ": note: \"single_if\" binds here";

      TextDiagnostic::printDiagnosticMessage( llvm::outs(), false, ss.str(), 0, 0, false );
    }
  }
};

class SingleIfVisitor : public RecursiveASTVisitor<SingleIfVisitor> {
 public:
  explicit SingleIfVisitor( ASTContext *Context ) : Context( Context ) {}

  bool VisitIfStmt( IfStmt *is ) {

    // Solution here: skip non single ifs
    if (is->hasElseStorage()) {
      Stmt else_stmt = is->getElse();
      if (isa<IfStmt>(else_stmt)) {
        elseifs.push_back(cast<IfStmt>(else_stmt));
      }
    }
    
    FullSourceLoc fullLocation = Context->getFullLoc( is->getBeginLoc() );

    stringstream ss;

    if ( fullLocation.isValid() ) {
      llvm::outs() << ":" << fullLocation.getSpellingLineNumber()
         << ":" << fullLocation.getSpellingColumnNumber()
         << ": note: \"single_if\" binds here\n";
    }

    return true;
  }

 private:
  vector<IfStmt *> elseifs;
  ASTContext *Context;
};

class SingleIfConsumer : public ASTConsumer {
 public:
  explicit SingleIfConsumer(ASTContext *Context) : Visitor(Context) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseTranslationUnitDecl(Context.getTranslationUnitDecl()); // Traverse the AST
  }
  
 private:
  SingleIfVisitor Visitor;
};

class SingleIfAction : public ASTFrontendAction {
 public:
  SingleIfAction() {}

  unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile) override {
    return make_unique<SingleIfConsumer>(&Compiler.getASTContext());
  }
};

static llvm::cl::OptionCategory SIF( "SingleIf" );

int main( int argc, const char **argv ) {
  CommonOptionsParser op( argc, argv, SIF );  // Command line parser

  ClangTool Tool( op.getCompilations(), op.getSourcePathList() );  // Initialize tool

  StatementMatcher if_stmt = ifStmt().bind( "single_if" );  // name of the match

  MatchFinder finder;
  IfCallBack if_cb;

  finder.addMatcher( if_stmt, &if_cb );

  if ( false ) { // AST Matcher
    Tool.run( newFrontendActionFactory( &finder ).get() );
  } else {  // Recursive AST Visitor
    Tool.run( newFrontendActionFactory<SingleIfAction>().get() );
  }

  return 0;
}
