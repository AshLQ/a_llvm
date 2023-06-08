#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/CommandLine.h"
#include "Utils.h"
#include "SplitBasicBlock.h"
#include <vector>

using namespace std;
using namespace llvm;

static cl::opt<int> fakeNum("fake_num", cl::init(1), cl::desc("fake_num"));

namespace {
    class FakeControl : public FunctionPass {
    public:
        static char ID;

        FakeControl() : FunctionPass(ID) {
            srand(time(NULL));
        }

        bool runOnFunction(Function &F);

        bool fake(BasicBlock *block);

        Value *createFakeCmp(BasicBlock *block);
    };
}


bool FakeControl::runOnFunction(Function &F) {
    INIT_CONTEXT(F);
//    FunctionPass *splitBlockPass = createSplitBasicBlockPass();
//    splitBlockPass->runOnFunction(F);

    for (int i = 0; i < fakeNum; i++) {
        vector<BasicBlock *> blockArr;
        for (BasicBlock &block: F) {
            blockArr.push_back(&block);
        }
        for (BasicBlock *block: blockArr) {
            fake(block);
        }
    }
    return true;
}

Value *FakeControl::createFakeCmp(BasicBlock *block) {
    //y>10||x*(x+1)%2==0
    Module *M = block->getModule();
    GlobalVariable *xPtr = new GlobalVariable(*M, TYPE_I32, false, GlobalValue::CommonLinkage, CONST_I32(0), "xPtr");
    GlobalVariable *yPtr = new GlobalVariable(*M, TYPE_I32, false, GlobalValue::CommonLinkage, CONST_I32(0), "yPtr");
    LoadInst *xValue = new LoadInst(TYPE_I32, xPtr, "xValue", block);
    LoadInst *yValue = new LoadInst(TYPE_I32, yPtr, "yValue", block);
    ICmpInst *cond1 = new ICmpInst(*block, CmpInst::ICMP_SLT, yValue, CONST_I32(10));
    BinaryOperator *op1 = BinaryOperator::CreateAdd(xValue, CONST_I32(1), "", block);
    BinaryOperator *op2 = BinaryOperator::CreateMul(op1, xValue, "", block);
    BinaryOperator *op3 = BinaryOperator::CreateURem(op2, CONST_I32(2), "", block);
    ICmpInst *cond2 = new ICmpInst(*block, CmpInst::ICMP_EQ, op3, CONST_I32(0));
    return BinaryOperator::CreateOr(cond1, cond2, "", block);
}

bool FakeControl::fake(BasicBlock *entryBlock) {
    //拆分基本块为3部分
    BasicBlock *bodyBlock = entryBlock->splitBasicBlock(entryBlock->getFirstNonPHI(), "body");
    BasicBlock *endBlock = bodyBlock->splitBasicBlock(bodyBlock->getTerminator(), "end");
    //clone中间基本块
    BasicBlock *cloneBodyBlock = createCloneBasicBlock(bodyBlock);

    //构造虚假跳转
    //去除原跳转
    entryBlock->getTerminator()->eraseFromParent();
    bodyBlock->getTerminator()->eraseFromParent();
    cloneBodyBlock->getTerminator()->eraseFromParent();

    Value *entryCond = createFakeCmp(entryBlock);
    Value *bodyCond = createFakeCmp(bodyBlock);

    BranchInst::Create(bodyBlock, cloneBodyBlock, entryCond, entryBlock);
    BranchInst::Create(endBlock, cloneBodyBlock, bodyCond, bodyBlock);
    BranchInst::Create(bodyBlock, cloneBodyBlock);
}


char FakeControl::ID = 0;
// 注册该 Demo Pass
static RegisterPass<FakeControl> X("fakecontrol", "fakecontrol");