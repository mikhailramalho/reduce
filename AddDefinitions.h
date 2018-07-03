#ifndef REDUCE_ADDDEFINITIONS_H
#define REDUCE_ADDDEFINITIONS_H

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Lex/Preprocessor.h>
#include <string>

class AddDefinitions : public clang::ASTFrontendAction
{
public:
  AddDefinitions() = default;

  bool BeginSourceFileAction(
    clang::CompilerInstance &CI) override
  {
    std::string intrinsics = R"(  )";

    clang::Preprocessor &PP = CI.getPreprocessor();

    std::string s = PP.getPredefines();
    s += intrinsics;
    PP.setPredefines(s);

    return true;
  }

  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
    clang::CompilerInstance &CI,
    StringRef InFile) override
  {
    return llvm::make_unique<clang::ASTConsumer>();
  }
};


#endif // REDUCE_ADDDEFINITIONS_H
