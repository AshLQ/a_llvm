cd ./build
cmake ../Transforms
make
cd ../Test
clang -S -emit-llvm TestProgram.cpp -o IR/TestProgram.ll
opt -load ../build/LLVMObfuscator.so -hlw -S IR/TestProgram.ll -o IR/TestProgram_hlw.ll
clang IR/TestProgram_hlw.ll -o bin/TestProgram_hlw
./bin/TestProgram_hlw flag{s1mpl3_11vm_d3m0}
echo "===================split basic block==========================="
opt -load ../build/LLVMObfuscator.so -split -S IR/TestProgram.ll -o IR/TestProgram_split.ll
clang IR/TestProgram_split.ll -o bin/TestProgram_split
./bin/TestProgram_split flag{s1mpl3_11vm_d3m0}

echo "===================cfg flattening==========================="
opt -lowerswitch -S IR/TestProgram.ll -o IR/TestProgram_lowerswitch.ll
opt -load ../build/LLVMObfuscator.so -flattening -S IR/TestProgram_lowerswitch.ll -o IR/TestProgram_cfgFlattening.ll
clang IR/TestProgram_cfgFlattening.ll -o bin/TestProgram_cfgFlattening
./bin/TestProgram_cfgFlattening flag{s1mpl3_11vm_d3m0}

echo "===================fake control==========================="
opt -load ../build/LLVMObfuscator.so -fakecontrol -S IR/TestProgram.ll -o IR/TestProgram_fakeControl.ll
clang IR/TestProgram_fakeControl.ll -o bin/TestProgram_fakeControl
./bin/TestProgram_fakeControl flag{s1mpl3_11vm_d3m0}