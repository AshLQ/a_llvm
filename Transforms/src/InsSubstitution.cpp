#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/Support/CommandLine.h"
#include "Utils.h"
#include <vector>


using namespace std;
using namespace llvm;

#define NUM_ADD_SUBST 4
#define NUM_SUB_SUBST 3

static cl::opt<int> subst_num("subst_num", cl::init(1), cl::desc("fake_num"));
namespace {
    class InsSubstitution : public FunctionPass {
    public:
        static char ID;

        InsSubstitution() : FunctionPass(ID) {
            srand(time(NULL));
        }

        bool runOnFunction(Function &F);

        void substInsControl(BinaryOperator *bi);

        void substAdd(BinaryOperator *bi);

        void addNeg(BinaryOperator *bi);

        void addDoubleNeg(BinaryOperator *bi);

        void addRand(BinaryOperator *bi);

        void addRand2(BinaryOperator *bi);

    };
}

bool InsSubstitution::runOnFunction(Function &F) {
    for (int i = 0; i < subst_num; i++) {
        for (BasicBlock &block: F) {
            vector<Instruction *> insArr;
            for (Instruction &inst: block) {
                insArr.push_back(&inst);
            }
            for (Instruction *inst: insArr) {
                if (isa<BinaryOperator>(inst)) {
                    substInsControl(cast<BinaryOperator>(inst));
                }
            }
        }
    }

    return true;
}

void InsSubstitution::substInsControl(BinaryOperator *bi) {
    //判断指令类型
    switch (bi->getOpcode()) {
        case BinaryOperator::Add:
            substAdd(bi);
            break;
        case BinaryOperator::Sub:
            break;

        default:
            break;
    }
}

void InsSubstitution::substAdd(BinaryOperator *bi) {
    int choice = rand() % NUM_ADD_SUBST;
    switch (choice) {
        case 0:
            addNeg(bi);
            break;
        case 1:
            addDoubleNeg(bi);
            break;
        case 2:
            addRand(bi);
            break;
        case 3:
            addRand2(bi);
            break;
    }
}

void InsSubstitution::addNeg(BinaryOperator *bi) {
    //a=b+c->a=b-(-c)
    BinaryOperator *opNeg = BinaryOperator::CreateNeg(bi->getOperand(1), "", bi);
    BinaryOperator *opSub = BinaryOperator::CreateSub(bi->getOperand(0), opNeg, "", bi);
    bi->replaceAllUsesWith(opSub);
    bi->eraseFromParent();
}

void InsSubstitution::addDoubleNeg(BinaryOperator *bi) {
    //a=b+c->a=-(-b+(-c));
    BinaryOperator *op1 = BinaryOperator::CreateNeg(bi->getOperand(0), "", bi);
    BinaryOperator *op2 = BinaryOperator::CreateNeg(bi->getOperand(1), "", bi);
    BinaryOperator *op3 = BinaryOperator::CreateAdd(op1, op2, "", bi);
    BinaryOperator *op4 = BinaryOperator::CreateNeg(op3, "", bi);
    bi->replaceAllUsesWith(op4);
    bi->eraseFromParent();
}

void InsSubstitution::addRand(BinaryOperator *bi) {
    //a=b+c->r=rand();a=b+r;a=a+c;a=a-r;
    ConstantInt *r = (ConstantInt *) CONST(bi->getType(), rand());
    BinaryOperator *op1 = BinaryOperator::CreateAdd(bi->getOperand(0), r, "", bi);
    BinaryOperator *op2 = BinaryOperator::CreateAdd(bi->getOperand(1), op1, "", bi);
    BinaryOperator *op3 = BinaryOperator::CreateSub(op2, r, "", bi);
    bi->replaceAllUsesWith(op3);
    bi->eraseFromParent();
}

void InsSubstitution::addRand2(BinaryOperator *bi) {
    //a=b+c->r=rand();a=b-r;a=a+b;a=a+r
    ConstantInt *r = (ConstantInt *) CONST(bi->getType(), rand());
    BinaryOperator *op1 = BinaryOperator::CreateSub(bi->getOperand(0), r, "", bi);
    BinaryOperator *op2 = BinaryOperator::CreateAdd(bi->getOperand(1), op1, "", bi);
    BinaryOperator *op3 = BinaryOperator::CreateAdd(op2, r, "", bi);
    bi->replaceAllUsesWith(op3);
    bi->eraseFromParent();
}


char InsSubstitution::ID = 0;
static RegisterPass<InsSubstitution> X("inssubst", "inssubst llvm pass");