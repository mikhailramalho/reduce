#include <clang/Basic/Version.inc>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Options.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Option/ArgList.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <iostream>

#include "AddDefinitions.h"
#include "DumpHeaders.h"
#include "Rewriter.h"

std::string create_tmp_header_dir()
{
  // TODO: use boost filesystem
  return "/tmp/";
}

int main(int argc, char *argv[])
{
  // Create virtual file system to add clang's headers
  llvm::IntrusiveRefCntPtr<clang::vfs::OverlayFileSystem> OverlayFileSystem(
    new clang::vfs::OverlayFileSystem(clang::vfs::getRealFileSystem()));

  llvm::IntrusiveRefCntPtr<clang::vfs::InMemoryFileSystem> InMemoryFileSystem(
    new clang::vfs::InMemoryFileSystem);
  OverlayFileSystem->pushOverlay(InMemoryFileSystem);

  llvm::IntrusiveRefCntPtr<clang::FileManager> Files(
    new clang::FileManager(clang::FileSystemOptions(), OverlayFileSystem));

  // Create everything needed to create a CompilerInvocation,
  // copied from ToolInvocation::run
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts =
    new clang::DiagnosticOptions();

  std::unique_ptr<llvm::opt::OptTable> Opts(
    clang::driver::createDriverOptTable());

  std::vector<const char *> Argv;
  for (int i = 0; i < argc; i++)
    Argv.push_back(argv[i]);

  // Add syntax-only so the linker option is not called
  Argv.push_back("-fsyntax-only");

  // Dump the headers to a tmp dir and add to the arguments
  std::string dir = create_tmp_header_dir();
  dump_clang_headers(dir);
  Argv.push_back(std::string("-I" + dir).c_str());

  unsigned MissingArgIndex, MissingArgCount;
  llvm::opt::InputArgList ParsedArgs = Opts->ParseArgs(
    llvm::ArrayRef<const char *>(Argv).slice(1),
    MissingArgIndex,
    MissingArgCount);

  clang::ParseDiagnosticArgs(*DiagOpts, ParsedArgs);

  clang::TextDiagnosticPrinter *DiagnosticPrinter =
    new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  clang::DiagnosticsEngine *Diagnostics =
    new clang::DiagnosticsEngine(
      llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs>(
        new clang::DiagnosticIDs()),
      &*DiagOpts,
      DiagnosticPrinter,
      false);

  const std::unique_ptr<clang::driver::Driver> Driver(
    new clang::driver::Driver(
      "clang-tool",
      llvm::sys::getDefaultTargetTriple(),
      *Diagnostics,
      std::move(Files->getVirtualFileSystem())));
  Driver->setTitle("clang_based_tool");

  // Since the input might only be virtual, don't check whether it exists.
  Driver->setCheckInputsExist(false);
  const std::unique_ptr<clang::driver::Compilation> Compilation(
    Driver->BuildCompilation(llvm::makeArrayRef(Argv)));

  const clang::driver::JobList &Jobs = Compilation->getJobs();
  assert(Jobs.size() == 1);

  const llvm::opt::ArgStringList *const CC1Args = &Jobs.begin()->getArguments();

#if (CLANG_VERSION_MAJOR >= 4)
  std::shared_ptr<clang::CompilerInvocation> Invocation(
    clang::tooling::newInvocation(Diagnostics, *CC1Args));
#else
  auto Invocation = clang::tooling::newInvocation(Diagnostics, *CC1Args);
#endif

  // Show the invocation, with -v.
  if (Invocation->getHeaderSearchOpts().Verbose)
  {
    llvm::errs() << "clang Invocation:\n";
    Compilation->getJobs().Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  // Create our custom action
  auto action = new AddDefinitions();

  // Create ASTUnit
  std::unique_ptr<clang::ASTUnit> unit(
    clang::ASTUnit::LoadFromCompilerInvocationAction(
      Invocation,
      std::make_shared<clang::PCHContainerOperations>(),
      Diagnostics,
      action));
  assert(unit);

  // Run the rewritter
  clang::Rewriter TheRewriter;
  TheRewriter.setSourceMgr(Diagnostics->getSourceManager(),
                           *Invocation->LangOpts.get());

  // Create an AST consumer instance which is going to get called by
  // ParseAST.
  Rewriter r(TheRewriter, unit);
  r.convert();

  // Query the rewriter for all the files it has rewritten, dumping their new
  // contents to stdout.
  for (clang::Rewriter::buffer_iterator I = TheRewriter.buffer_begin(),
         E = TheRewriter.buffer_end();
       I != E; ++I)
  {
    const clang::FileEntry *Entry = Diagnostics->getSourceManager().getFileEntryForID(
      I->first);
    I->second.write(llvm::outs());
  }

  return 0;
}