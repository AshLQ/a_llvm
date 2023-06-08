#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "SplitBasicBlock.h"
#include <vector>

using namespace std;
using namespace llvm;


static cl::opt<int> splitNum("split_num", cl::init(2), cl::desc("spilit <split_num time(s) each BB>"));

bool SplitBasicBlock::runOnFunction(Function &F) {
    vector<BasicBlock *> bbVec;
    for (BasicBlock &BB: F) {
        bbVec.push_back(&BB);
    }
    for (BasicBlock *BB: bbVec) {
        if (!containsPHI(BB)) {
            split(BB);
        }
    }

    return true;
}

bool SplitBasicBlock::containsPHI(BasicBlock *BB) {
    for (Instruction &I: *BB) {
        if (isa<PHINode>(&I)) {
            return true;
        }
    }
    return false;
}

void SplitBasicBlock::split(BasicBlock *BB) {
    //向上取整
    int splitSize = (BB->size() + splitNum - 1) / splitNum;
    //分裂次数为splitNum-1次 2(1) 3(2)
    for (int i = 0; i < splitNum - 1; i++) {
        BasicBlock *curBB = BB;
        //当前指令大小
        int curSize = 0;
        for (Instruction &I: *curBB) {
            if (curSize++ == splitSize) {
                curBB->splitBasicBlock(&I);
                break;
            }
        }
    }
}

FunctionPass *llvm::createSplitBasicBlockPass() {
    return new SplitBasicBlock();
}

char SplitBasicBlock::ID = 0;
static RegisterPass<SplitBasicBlock> X("split", "split basicblock llvm pass");


