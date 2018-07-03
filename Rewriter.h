#ifndef REDUCE_REWRITTER_H
#define REDUCE_REWRITTER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser.
class Rewriter
{
public:
  Rewriter(clang::Rewriter &R, std::unique_ptr<clang::ASTUnit> &_ASTs);

  void convert();

private:
  clang::Rewriter &TheRewriter;
  clang::ASTContext *ASTContext;
  std::unique_ptr<clang::ASTUnit> &ASTs;
};

#endif // REDUCE_REWRITTER_H
