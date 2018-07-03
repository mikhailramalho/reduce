#include "Rewriter.h"

#include <iostream>
#include <sstream>

static std::string get_decl_name(const clang::NamedDecl *decl)
{
  if (const clang::IdentifierInfo *identifier = decl->getIdentifier())
    return identifier->getName().str();

  std::string name;
  llvm::raw_string_ostream rso(name);
  decl->printName(rso);
  return rso.str();
}

Rewriter::Rewriter(clang::Rewriter &R, std::unique_ptr<clang::ASTUnit> &_ASTs)
  : TheRewriter(R), ASTContext(nullptr), ASTs(_ASTs)
{
}

void Rewriter::convert()
{
  // Update ASTContext as it changes for each source file
  ASTContext = &(*ASTs).getASTContext();

  auto tu = *ASTContext->getTranslationUnitDecl();
  for (auto const &decl : tu.decls())
  {
    if(!decl->getLocation().isValid())
      continue;

    if (auto *f = llvm::dyn_cast<clang::FunctionDecl>(decl))
    {
      if(!f->isUsed() && get_decl_name(f) != "ED25519_verify")
        TheRewriter.RemoveText(f->getSourceRange());
    }
    else if (auto *e = llvm::dyn_cast<clang::EmptyDecl>(decl))
    {
      TheRewriter.RemoveText(e->getSourceRange());
    }
    else if (auto *t = llvm::dyn_cast<clang::TypedefNameDecl>(decl))
    {
      /* Doesn't work yet, see https://reviews.llvm.org/D46190
      if(!t->isUsed())
        TheRewriter.RemoveText(t->getSourceRange());
      */
    }
    else if (auto *t = llvm::dyn_cast<clang::TagDecl>(decl))
    {
      /* Doesn't work yet, see https://reviews.llvm.org/D46190
      if(!t->isUsed())
        TheRewriter.RemoveText(t->getSourceRange());
      */
    }
    else if (auto *v = llvm::dyn_cast<clang::ValueDecl>(decl))
    {
      if(!v->isUsed())
        TheRewriter.RemoveText(v->getSourceRange());

    }
  }
}
