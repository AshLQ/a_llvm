#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
namespace {
    class Demo : public FunctionPass {
    public:
        static char ID;

        Demo() : FunctionPass(ID) {}

        bool runOnFunction(Function &F);
    };
}

// runOnFunction 函数实现
bool Demo::runOnFunction(Function &F) {
    outs() << "Function: " << F.getName() << "\n";
    return true;
}

char Demo::ID = 0;
// 注册该 Demo Pass
static RegisterPass<Demo> X("hlw", "my first line of llvm pass");