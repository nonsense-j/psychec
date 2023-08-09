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
    bool isCondition;
    bool isLoop;
    bool isFuncDef;
    bool hasCallExpr;
    int nodeLevel;

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
    std::string cfgCode_;
    std::shared_ptr<CFGNode> nextNode_;       // true branch
    std::shared_ptr<CFGNode> nextFalseNode_;  // false branch both for if and loop
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

    std::vector<std::tuple<const SyntaxNode*, int>> dump_;
    std::vector<std::shared_ptr<CFGNode>> funcDefStack_;
    std::vector<std::pair<std::shared_ptr<CFGNode>, int>> ifPairStack_;  // 存放if语句及其层级计数，添加flase跳转
    std::vector<std::shared_ptr<CFGNode>> ifEndStmtStack_;  // 存放if语句段else之前的最后一个命令，添加跳出if语句的跳转
    std::vector<std::pair<std::shared_ptr<CFGNode>, bool>> loopPairStack_;  // 存放while/for语句和hasBreak判断
    std::vector<std::vector<std::shared_ptr<CFGNode>>> breakStmtsStack_;    // 存放break语句的栈

    std::shared_ptr<CFGNode> lastNode = std::make_shared<CFGNode>();  // 上一条语句，添加顺序跳转
};

}  // namespace C
}  // namespace psy

#endif
