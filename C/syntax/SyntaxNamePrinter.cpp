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

#include "SyntaxNamePrinter.h"

#include <algorithm>
#include <iostream>
#include <unordered_map>

#include "SyntaxNode.h"

using namespace psy;
using namespace C;

namespace {

int CUR_LEVEL;

std::string formatSnippet(std::string& snippet, bool maxLimit = true) {
    std::replace_if(
        snippet.begin(), snippet.end(), [](char c) { return c == '\n' || c == '\t'; }, ' ');

    while (true) {
        auto pos = snippet.find("  ");
        if (pos == std::string::npos) break;
        snippet = snippet.replace(pos, 2, " ");
    }

    if (maxLimit) {
        static const auto MAX_LEN = 30;
        if (snippet.length() > MAX_LEN) {
            snippet = snippet.substr(0, MAX_LEN);
            snippet += "...";
        }
    }

    return snippet;
}

}  // namespace

void SyntaxNamePrinter::print(const SyntaxNode* node, Style style) { print(node, style, std::cout); }

void SyntaxNamePrinter::print(const SyntaxNode* node, Style style, std::ostream& os) {
    CUR_LEVEL = 0;

    nonterminal(node);

    auto source = node->syntaxTree()->text().rawText();

    os << std::endl;
    for (auto i = 0U; i < dump_.size(); ++i) {
        auto node = std::get<0>(dump_[i]);
        auto nodeLevel = std::get<1>(dump_[i]);

        if (style == Style::Plain) {
            os << std::string(nodeLevel * 4, ' ');
            os << to_string(node->kind()) << std::endl;
            continue;
        }

        auto levelCnt = 0;
        while (nodeLevel > levelCnt) {
            if (nodeLevel == levelCnt + 1) {
                os << '|';
                os << std::string(2, '-');
            } else {
                int nextLevelBelow;
                for (auto j = i + 1; j < dump_.size(); ++j) {
                    nextLevelBelow = std::get<1>(dump_[j]);
                    if (nextLevelBelow <= levelCnt + 1) break;
                }
                if (nextLevelBelow == levelCnt + 1)
                    os << '|';
                else
                    os << ' ';
                os << std::string(2, ' ');
            }
            ++levelCnt;
        }

        os << to_string(node->kind()) << " [" << nodeLevel << "]  ";

        if (node->kind() == TranslationUnit) {
            os << std::endl;
            continue;
        }

        os << " <";
        auto firstTk = node->firstToken();
        auto lastTk = node->lastToken();
        if (firstTk.isValid()) os << firstTk.location().lineSpan().span().start();
        os << "..";
        if (lastTk.isValid()) os << lastTk.location().lineSpan().span().end();
        os << "> ";

        if (firstTk.isValid() && lastTk.isValid()) {
            auto firstTkStart = source.c_str() + firstTk.span().start();
            auto lastTkEnd = source.c_str() + lastTk.span().end();
            std::string snippet(firstTkStart, lastTkEnd - firstTkStart);
            os << " `" << formatSnippet(snippet) << "`";
        }

        os << std::endl;
    }
}

void SyntaxNamePrinter::nonterminal(const SyntaxNode* node) {
    if (!node) return;

    dump_.push_back(std::make_tuple(node, CUR_LEVEL));

    ++CUR_LEVEL;
    visit(node);
    --CUR_LEVEL;
}

// CFGNode related
// 获取第一个括号相匹配的内容
std::string getMatchContent(std::string input) {
    int index = input.find("(");
    int count;
    count = (index == -1) ? 0 : 1;
    index++;
    while (count > 0 && index < (int)input.size()) {
        if (input[index] == '(') count += 1;
        if (input[index] == ')') count -= 1;
        index++;
    }
    if (count == 0) return input.substr(0, index);
    return "";
}

void CFGNode::setCode(std::string codeSnippet) {
    // compound -> simple
    if (isCondition) {
        cfgCode_ = getMatchContent(codeSnippet);
        if (cfgCode_.empty()) {
            std::cout << "Invalid Conditional Statement" << std::endl;
        }
    } else
        cfgCode_ = codeSnippet;
}
std::string CFGNode::getCode() {
    if (cfgCode_.empty())
        return "Code has not been set yet";
    else
        return cfgCode_;
}

void CFGNode::setNextNode(std::shared_ptr<CFGNode> nextNode) { nextNode_ = nextNode; }
std::shared_ptr<CFGNode> CFGNode::getNextNode() { return nextNode_; }

void CFGNode::setNextFalseNode(std::shared_ptr<CFGNode> nextFalseNode) { nextFalseNode_ = nextFalseNode; }
std::shared_ptr<CFGNode> CFGNode::getNextFalseNode() { return nextFalseNode_; }

void CFGNode::setSyntaxNode(const SyntaxNode* syntaxNode) { syntaxNode_ = syntaxNode; }
const SyntaxNode* CFGNode::getSyntaxNode() { return syntaxNode_; }

void SyntaxNamePrinter::getCFG(const SyntaxNode* node) {
    CUR_LEVEL = 0;
    nonterminal(node);
    auto source = node->syntaxTree()->text().rawText();
    funcDefStack_.clear();
    bool elseCompFlag = false, callExprFlag = false;

    for (auto i = 0U; i < dump_.size(); ++i) {
        auto* node = std::get<0>(dump_[i]);
        auto nodeLevel = std::get<1>(dump_[i]);
        auto firstTk = node->firstToken();
        auto lastTk = node->lastToken();

        if (firstTk.isValid() && lastTk.isValid()) {
            auto firstTkStart = source.c_str() + firstTk.span().start();
            auto lastTkEnd = source.c_str() + lastTk.span().end();
            std::string snippet(firstTkStart, lastTkEnd - firstTkStart);

            // 对于函数定义的语句，先放到funcDefStack里面存起来有利于后续进行输出
            if (node->kind() == SyntaxKind::FunctionDefinition) {
                std::shared_ptr<CFGNode> funcDefNode = std::make_shared<CFGNode>();
                funcDefNode->isFuncDef = true;
                funcDefNode->setSyntaxNode(node);
                funcDefNode->nodeLevel = nodeLevel;
                funcDefNode->setCode(formatSnippet(snippet, false));
                funcDefStack_.push_back(funcDefNode);
                // [DEBUG]
                // std::cout << "[FUNCDEF] ----- \t" << funcDefNode->nodeLevel << "\t----- \t" << funcDefNode->getCode()
                //           << std::endl;
            }

            // 处理除了compound stmt以外的所有stmt
            if (node->kind() > SyntaxKind::CompoundStatement && node->kind() < SyntaxKind::TypeName) {
                std::shared_ptr<CFGNode> stmtNode = std::make_shared<CFGNode>();
                stmtNode->setSyntaxNode(node);
                stmtNode->nodeLevel = nodeLevel;

                // 标记函数调用
                if (callExprFlag) {
                    lastNode->hasCallExpr = true;
                    callExprFlag = false;
                }

                // 添加默认跳转：顺序跳转
                if (!funcDefStack_.back()->getNextNode()) {
                    funcDefStack_.back()->setNextNode(stmtNode);
                } else {
                    if (lastNode->getSyntaxNode()->kind() != SyntaxKind::ReturnStatement &&
                        lastNode->getSyntaxNode()->kind() != SyntaxKind::ContinueStatement) {
                        lastNode->setNextNode(stmtNode);
                    }
                }

                /* 处理 if 条件语句 */
                // 添加跳转：if有两种
                if (!ifPairStack_.empty()) {
                    // 当If层级计数归零的时候添加else跳转：If语句的false跳转
                    // std::cout << "[curIfLevelMark]\t ----- \t" << ifPairStack_.back().second << std::endl;
                    if ((ifPairStack_.back().second == 0 && nodeLevel <= ifPairStack_.back().first->nodeLevel + 1) ||
                        elseCompFlag) {
                        // [DEBUG]
                        // std::cout << "[Next False STMT for If]\t ----- \t" << to_string(node->kind()) << std::endl;
                        ifPairStack_.back().first->setNextFalseNode(stmtNode);
                        ifEndStmtStack_.push_back(lastNode);
                        ifPairStack_.back().second = -1;
                        if (elseCompFlag) elseCompFlag = false;
                    }
                    // 当遇到和If语句level一样的语句（else之后第一条）时，if之后else之前最后一条语句->else之后第一条
                    if (!ifEndStmtStack_.empty() && nodeLevel <= ifPairStack_.back().first->nodeLevel) {
                        ifEndStmtStack_.back()->setNextNode(stmtNode);
                        ifPairStack_.pop_back();
                        ifEndStmtStack_.pop_back();
                    }
                }
                // 消除 if 语句自身的对于添加跳转部分的影响，所以先跳转再识别
                if (node->kind() == SyntaxKind::IfStatement) {
                    // 跟IfStmt相比，第三个（相同层级或者小一级的stmt）就是else的跳转语句
                    // 跟IfStmt相比，相同层级的下一个Stmt就是If语句块的跳转语句
                    stmtNode->isCondition = true;
                    ifPairStack_.push_back(std::make_pair(stmtNode, 3));
                }

                /* 处理 while for 循环语句 */
                if (node->kind() == SyntaxKind::BreakStatement) {
                    loopPairStack_.back().second = true;
                    breakStmtsStack_.back().push_back(stmtNode);
                }
                // 添加跳转：continue
                if (node->kind() == SyntaxKind::ContinueStatement) {
                    if (!loopPairStack_.empty()) {
                        stmtNode->setNextNode(loopPairStack_.back().first);
                    } else
                        std::cout << "No Loop for this BreakStatement." << std::endl;
                }
                // 添加跳转：while/for/break
                if (!loopPairStack_.empty()) {
                    // 找到循环结束后第一条语句
                    while (!loopPairStack_.empty() && nodeLevel <= loopPairStack_.back().first->nodeLevel) {
                        // [DEBUG]
                        // std::cout << "[Next False STMT for Loop]\t ----- \t" << to_string(node->kind()) << std::endl;
                        loopPairStack_.back().first->setNextFalseNode(stmtNode);
                        lastNode->setNextNode(loopPairStack_.back().first);
                        if (loopPairStack_.back().second) {
                            for (std::shared_ptr<CFGNode> curBreakStmt : breakStmtsStack_.back()) {
                                curBreakStmt->setNextNode(stmtNode);
                            }
                            breakStmtsStack_.pop_back();
                        }
                        loopPairStack_.pop_back();
                    }
                }
                // 消除while/for语句自身的对于添加跳转部分的影响，所以先跳转再识别
                if (node->kind() == SyntaxKind::WhileStatement || node->kind() == SyntaxKind::ForStatement) {
                    stmtNode->isCondition = true;
                    stmtNode->isLoop = true;
                    loopPairStack_.push_back(std::make_pair(stmtNode, false));
                    breakStmtsStack_.push_back(std::vector<std::shared_ptr<CFGNode>>{});
                }
                // 获取code
                // 对于declareStatement, code不完全，需要获取到下一个语句从而得到完整的code
                if (node->kind() == SyntaxKind::DeclarationStatement) {
                    auto* nextNode = std::get<0>(dump_[i + 1]);
                    auto nextFirstTk = nextNode->firstToken();
                    auto nextLastTk = nextNode->lastToken();
                    auto nextFirstTkStart = source.c_str() + nextFirstTk.span().start();
                    auto nextLastTkEnd = source.c_str() + nextLastTk.span().end();
                    std::string nextSnippet(nextFirstTkStart, nextLastTkEnd - nextFirstTkStart);
                    stmtNode->setCode(formatSnippet(nextSnippet, false));
                } else
                    stmtNode->setCode(formatSnippet(snippet, false));
                // [DEBUG]
                // std::cout << "[STMT]\t ----- \t" << stmtNode->getCode() << std::endl;

                lastNode = stmtNode;
            }
        }
        // 更新当前If层级计数(ifPairStack_.back().second > 0时ifPairStack_一定不为空)
        if (!ifPairStack_.empty()) {
            // 处理当else分支存在compound stmt时，需要获取到下一个语句
            if (ifPairStack_.back().second == 0 && nodeLevel <= ifPairStack_.back().first->nodeLevel + 1) {
                elseCompFlag = true;
                // std::cout << "\t[Else Compound]" << std::endl;
            }
            if (ifPairStack_.back().second > 0 && nodeLevel <= ifPairStack_.back().first->nodeLevel + 1) {
                ifPairStack_.back().second -= 1;
                // std::cout << "\t[Enter -1]" << std::endl;
            }
        }
        // 遇到函数调用保存下来
        if (node->kind() == SyntaxKind::CallExpression) {
            callExprFlag = true;
        }
    }
}

void SyntaxNamePrinter::printCFG() {
    for (auto i = 0U; i < funcDefStack_.size(); ++i) {
        std::shared_ptr<CFGNode> node = funcDefStack_[i];
        std::cout << "========[PRINTCFG]  FunctionDefinition #" << i << "  [PRINTCFG]========" << std::endl;
        while (node) {
            if (node->hasCallExpr) std::cout << "**";

            std::cout << "\t" << to_string(node->getSyntaxNode()->kind()) << "\t" << node->nodeLevel << "\t ----- \t"
                      << node->getCode() << std::endl;
            if (node->isCondition)
                node = node->getNextFalseNode();
            else
                node = node->getNextNode();
        }
        std::cout << std::endl;
    }
}
