#include "Utils.h"

#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"

using namespace std;
using namespace llvm;

LLVMContext *CONTEXT = nullptr;

void llvm::fixStack(Function &F) {
    vector < PHINode * > phiArr;
    vector < Instruction * > escapeVarArr;
    BasicBlock &entryBlock = F.getEntryBlock();
    for (BasicBlock &block: F) {
        for (Instruction &inst: block) {
            if (PHINode * pn = dyn_cast<PHINode>(&inst)) {
                phiArr.push_back(pn);
            } else if (!(isa<AllocaInst>(&inst) && inst.getParent() == &entryBlock) &&
                       isa<AllocaInst>(&inst) && inst.isUsedOutsideOfBlock(&block)) {
                outs() << "Instruction: " << inst.getName() << "\n";
                escapeVarArr.push_back(&inst);
            }
        }
    }

    for (PHINode *phi: phiArr) {
        DemotePHIToStack(phi, entryBlock.getTerminator());
    }

    for (Instruction *inst: escapeVarArr) {
        DemoteRegToStack(*inst, entryBlock.getTerminator());
    }

}

BasicBlock *llvm::createCloneBasicBlock(BasicBlock *block) {
    ValueToValueMapTy valueMap;
    BasicBlock *cloneBlock = CloneBasicBlock(block, valueMap, "cloneBlock", block->getParent());
    for (Instruction &inst: *cloneBlock) {
        for (int i = 0; i < inst.getNumOperands(); i++) {
            Value *value = MapValue(inst.getOperand(i), valueMap);
            if (value) {
                inst.setOperand(i, value);
            }
        }
    }
    return cloneBlock;
}