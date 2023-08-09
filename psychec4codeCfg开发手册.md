# psychec4codeCfg开发手册
### 前言
- 主要修改的文件是`C/syntax`中的`SyntaxNamePrinter.h`和`SyntaxNamePrinter.cpp`
- 原来的语法树中有两个重要的基本信息：一个是语句类别（所有类别存放在`C/syntax/SyntaxKind.h`文件中），另一个是语句层级（nodelevel，例如 if 语句是 nodelevel=2 ，那么其中第一个语句 nodelevel=3）

### 简单使用

项目目录下有一个编译好的`cnip`程序可以直接用，测试用例在`testCase`目录下。
或者参考`psychec`的编译命令进行编译：
```sh
# cmake CMakeLists.txt && make -j 4
```
在项目目录下可以得到`.cnip`文件，使用`-c`参数就可以获取当前文件的CodeCFG，例如（这个显示的结果是只考虑了false跳转）：
```sh
# ./cnip -c CodeEx/testCase3.c 
CFG is ready to dump ~
========[PRINTCFG]  FunctionDefinition #0  [PRINTCFG]========
        FunctionDefinition      1        -----  int f(int p) {
        IfStatement     3        -----  if (p > 2)
        ExpressionStatement     4        -----  p = p - 10;
        ExpressionStatement     3        -----  p *= 2;
        ReturnStatement 3        -----  return p;

========[PRINTCFG]  FunctionDefinition #1  [PRINTCFG]========
        FunctionDefinition      1        -----  int g(int p) {
        IfStatement     3        -----  if (p > 2)
**      ExpressionStatement     3        -----  p = f(p);
        ReturnStatement 3        -----  return p;
```
本项目中实现的所有跳转类型（尤其注意循环跳转）：
- **顺序跳转**：也就是正常顺序执行跳转到下一条语句
- **If 条件语句部分**：`node->isCondition=true`
    - **False 跳转**： If 语句除了顺序跳转还会跳转到 else 语句（或者跳出 If 结构体）
    - **If 段跳出**： If 中最后一句（ Else 之前）跳出If结构体
- **Loop 循环语句部分**：`node->isCondition=true && node->isLoop=true`
    - **False 跳转**：   while ( for ) 语句除了顺序跳转还会跳出当前循环结构体
    - **Break 跳转**：   break 语句会跳出当前循环结构体
    - **Continue 跳转**：   continue 语句默认会跳转到当前 while 语句
    - **循环跳转**：   while 结构体中最后一句默认会跳转到 while 语句

如果后续要获取到所有的路径序列（尤其要注意循环跳转，默认会死循环，可以自行设置循环跳转次数），参考`SyntaxNamePrinter.cpp`中的`SyntaxNamePrinter::printCFG`实现就行，具体的代码内容解释可以看下面两个章节。

### 进一步开发 -- `SyntaxNamePrinter.h`详细解释
> CFGNode声明在`C/syntax/SyntaxNamePrinter.h`文件中，具体实现代码在`SyntaxNamePrinter.cpp`中。在我的实现中，每一个CFGNode对应一个语句。
```cpp
class CFGNode {
   public:
    bool isCondition;  // 标识 If 和 While(For) 语句
    bool isLoop;       // 标识循环语句 while 和 for
    bool isFuncDef;    // 标识函数定义语句
    bool hasCallExpr;  // 标识存在函数调用
    int nodeLevel;     // 标识当前语句层级

    CFGNode()
        : isCondition(false),
          isLoop(false),
          isFuncDef(false),
          hasCallExpr(false),
          nodeLevel(-1),
          cfgCode_(""),
          nextNode_(nullptr),
          nextFalseNode_(nullptr),
          syntaxNode_(nullptr) {}

    void setCode(std::string codeSnippet);
    std::string getCode();

    void setNextNode(std::shared_ptr<CFGNode> nextNode);
    std::shared_ptr<CFGNode> getNextNode();

    void setNextFalseNode(std::shared_ptr<CFGNode> nextFalseNode);
    std::shared_ptr<CFGNode> getNextFalseNode();

    void setSyntaxNode(const SyntaxNode* syntaxNode);
    const SyntaxNode* getSyntaxNode();

   private:
    std::string cfgCode_;                     // 存放当前cfgNode对应的代码字符串
    std::shared_ptr<CFGNode> nextNode_;       // 下一个cfgNode，也是true跳转
    std::shared_ptr<CFGNode> nextFalseNode_;  // False跳转，包括if和while(for)
    const SyntaxNode* syntaxNode_;
};
```
为了实现各种情况下的codeCFGNode跳转，还在`SyntaxDumper`中定义了一些共享数据段：
```cpp
std::vector<std::tuple<const SyntaxNode*, int>> dump_;  // psychec原来就有，存放所有抽象语法树结点
    std::vector<std::shared_ptr<CFGNode>> funcDefStack_;    // 存放所有函数定义，也是出书CFGNode的入口
    std::vector<std::pair<std::shared_ptr<CFGNode>, int>> ifPairStack_;  // 存放if语句及其层级计数，添加flase跳转
    std::vector<std::shared_ptr<CFGNode>> ifEndStmtStack_;  // 存放if语句段else之前的最后一个命令，添加跳出if语句的跳转
    std::vector<std::pair<std::shared_ptr<CFGNode>, bool>> loopPairStack_;  // 存放while/for语句和hasBreak判断
    std::vector<std::vector<std::shared_ptr<CFGNode>>> breakStmtsStack_;    // 存放break语句的栈

    std::shared_ptr<CFGNode> lastNode = std::make_shared<CFGNode>();  // 上一条语句，添加顺序跳转
```
### 进一步开发 -- `SyntaxNamePrinter.cpp`详细解释
核心内容就是其中的`SyntaxNamePrinter::getCFG`函数，也就是根据不同的语句类型进行处理，创建补全CFGNode内容并且添加跳转，可以根据代码注释简单看一下。
