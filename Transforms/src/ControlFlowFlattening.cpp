#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "Utils.h"
#include "SplitBasicBlock.h"
#include <vector>

using namespace std;
using namespace llvm;
namespace {
    class ControlFlowFlattening : public FunctionPass {
    public:
        static char ID;

        ControlFlowFlattening() : FunctionPass(ID) {}

        bool runOnFunction(Function &F);

        void flattening(Function &F);

    };
}


bool ControlFlowFlattening::runOnFunction(Function &F) {
    INIT_CONTEXT(F);
//    FunctionPass *slitBlockPass = createSplitBasicBlockPass();
//    slitBlockPass->runOnFunction(F);
    flattening(F);
    return true;
}


void ControlFlowFlattening::flattening(Function &F) {
    vector<BasicBlock *> blockArr;
    //记录原基本块
    for (BasicBlock &block: F) {
        blockArr.push_back(&block);
    }
    if (blockArr.empty()) {
        return;
    }
    blockArr.erase(blockArr.begin());
    //获取entryBlock块,判定是否需要分割,保证entryBlock末尾无条件跳转
    BasicBlock &entryBlock = F.getEntryBlock();
    BranchInst *branchInst = dyn_cast<BranchInst>(entryBlock.getTerminator());
    if (branchInst && branchInst->isConditional()) {
        blockArr.insert(blockArr.begin(), entryBlock.splitBasicBlock(branchInst));
    }

    //创建dispatchBlock和returnBlock,并建立跳转关系
    BasicBlock *dispatchBlock = BasicBlock::Create(*CONTEXT, "dispatchBlock", &F, &entryBlock);
    BasicBlock *returnBlock = BasicBlock::Create(*CONTEXT, "returnBlock", &F, &entryBlock);
    BranchInst::Create(dispatchBlock, returnBlock);
    //移动entryBlock到dispatchBlock之前
    entryBlock.moveBefore(dispatchBlock);
    //将entry块末尾跳转目标修改为dispatch块
    entryBlock.getTerminator()->eraseFromParent();
    BranchInst::Create(dispatchBlock, &entryBlock);


    //在entry块中初始化一个随机的switch变量
    int randNumCase = rand();
    AllocaInst *swVarPtr = new AllocaInst(TYPE_I32, 0, "swVarPtr", entryBlock.getTerminator());
    new StoreInst(CONST_I32(randNumCase), swVarPtr, entryBlock.getTerminator());
    //dispatch块读取这个变量,创建switch默认分支,默认跳转到return块
    LoadInst *swVar = new LoadInst(TYPE_I32, swVarPtr, "swVar", false, dispatchBlock);
    BasicBlock *swDefaultBlock = BasicBlock::Create(*CONTEXT, "swDefault", &F, returnBlock);
    BranchInst::Create(returnBlock, swDefaultBlock);
    SwitchInst *swInst = SwitchInst::Create(swVar, swDefaultBlock, 0, dispatchBlock);
    //switch指令添加对每个基本块跳转的case分支
    for (BasicBlock *block: blockArr) {
        block->moveBefore(returnBlock);
        swInst->addCase(CONST_I32(randNumCase), block);
        randNumCase = rand();
    }


    //对基本块执行流程进行处理
    for (BasicBlock *block: blockArr) {
        if (block->getTerminator()->getNumSuccessors() == 0) {
            continue;
        } else if (block->getTerminator()->getNumSuccessors() == 1) {
            ConstantInt *numCase = swInst->findCaseDest(block->getTerminator()->getSuccessor(0));
            new StoreInst(numCase, swVarPtr, block->getTerminator());
            block->getTerminator()->eraseFromParent();
            BranchInst::Create(returnBlock, block);
        } else if (block->getTerminator()->getNumSuccessors() == 2) {
            ConstantInt *numCase1 = swInst->findCaseDest(block->getTerminator()->getSuccessor(0));
            ConstantInt *numCase2 = swInst->findCaseDest(block->getTerminator()->getSuccessor(1));
            BranchInst *br = cast<BranchInst>(block->getTerminator());
            SelectInst *sel = SelectInst::Create(br->getCondition(), numCase1, numCase2, "", block->getTerminator());
            new StoreInst(sel, swVarPtr, block->getTerminator());
            block->getTerminator()->eraseFromParent();
            BranchInst::Create(returnBlock, block);
        }
    }

    //修复phi和逃逸指令
    fixStack(F);
}


FunctionPass *createSplitBasicBlockPass() {
    return new ControlFlowFlattening();
}

char ControlFlowFlattening::ID = 0;


static RegisterPass<ControlFlowFlattening> X("flattening", "flowflattening llvm pass");