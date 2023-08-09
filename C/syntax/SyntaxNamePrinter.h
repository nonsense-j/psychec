// Copyright (c) 2020/21/22 Leandro T. C. Melo <ltcmelo@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef PSYCHE_C_SYNTAX_NAME_PRINTER_H__
#define PSYCHE_C_SYNTAX_NAME_PRINTER_H__

#include <ostream>
#include <tuple>
#include <vector>

#include "API.h"
#include "SyntaxDumper.h"

namespace psy {
namespace C {

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
class PSY_C_API SyntaxNamePrinter final : public SyntaxDumper {
   public:
    using SyntaxDumper::SyntaxDumper;

    enum class Style : char { Plain, Decorated };

    void print(const SyntaxNode* node, Style style);
    void print(const SyntaxNode* node, Style style, std::ostream& os);
    void getCFG(const SyntaxNode* node);
    void printCFG();

   private:
    virtual void nonterminal(const SyntaxNode* node) override;

    std::vector<std::tuple<const SyntaxNode*, int>> dump_;  // psychec原来就有，存放所有抽象语法树结点
    std::vector<std::shared_ptr<CFGNode>> funcDefStack_;    // 存放所有函数定义，也是出书CFGNode的入口
    std::vector<std::pair<std::shared_ptr<CFGNode>, int>> ifPairStack_;  // 存放if语句及其层级计数，添加flase跳转
    std::vector<std::shared_ptr<CFGNode>> ifEndStmtStack_;  // 存放if语句段else之前的最后一个命令，添加跳出if语句的跳转
    std::vector<std::pair<std::shared_ptr<CFGNode>, bool>> loopPairStack_;  // 存放while/for语句和hasBreak判断
    std::vector<std::vector<std::shared_ptr<CFGNode>>> breakStmtsStack_;    // 存放break语句的栈

    std::shared_ptr<CFGNode> lastNode = std::make_shared<CFGNode>();  // 上一条语句，添加顺序跳转
};

}  // namespace C
}  // namespace psy

#endif
