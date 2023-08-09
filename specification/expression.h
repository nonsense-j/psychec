#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>
#include <string>
#include <map>

#include "ordered_map.h"
#include "type.h"

namespace z3 {
    class context;
    class expr;
    class solver;
}

namespace tool {
#define DECL_UNIQUE_PTR(T) using T##Ptr = std::unique_ptr<class T>;

    class Driver;
    class Solver;

    class Context;
    class DeclContext;
    using ExprPtr = std::unique_ptr<class Expr>;
    using ExprList = std::vector<ExprPtr>;
    using StmtPtr = std::unique_ptr<class Stmt>;
    using StmtList = std::vector<StmtPtr>;

    using NodePtr = std::unique_ptr<class Node>;
    using NodeList = std::vector<NodePtr>;
    class Node {
    public:
        virtual ~Node() = 0;
        virtual std::string className() const { return "$unknown$"; }
        // getChildren的调用者只对它们进行虚函数调用
        virtual std::vector<Node*> getChildren() const;
        virtual std::string getAST() const;
        virtual std::string getCode(int indent = 0) const;
        inline void printAST(std::ostream& os) const { os << getAST() << std::endl; }
        inline void printCode(std::ostream& os, int indent = 0) const { os << getCode() << std::endl; }

        //virtual void visit(tool::Solver& solver);
        //virtual StmtPtr cast2C1(DeclContext& ctx);    // 返回值真表示放弃原指针

        virtual bool isExpr() const;

        void skip(std::ostream& os, std::string prefix = "");
        void print(std::ostream& os, std::string prefix = "") const;
    };

    std::ostream& operator<<(std::ostream& os, Node& n);
    std::ostream& operator<<(std::ostream& os, Node* n);

    using DeclPtr = std::unique_ptr<class Decl>;
    using DeclList = std::vector<DeclPtr>;
    class Decl : public Node {
        friend class Context;
    public:
        enum class DeclKind {
            Field,
            Function,
            ParmVar,
            Struct,
            TranslationUnit,
            Var
        };
        static std::string DeclKind2Str(DeclKind K) {
            switch (K)
            {
            case Decl::DeclKind::Field:
                return "Field";
            case Decl::DeclKind::Function:
                return "Function";
            case Decl::DeclKind::ParmVar:
                return "ParmVar";
            case Decl::DeclKind::Struct:
                return "Struct";
            case Decl::DeclKind::TranslationUnit:
                return "TranslationUnit";
            case Decl::DeclKind::Var:
                return "Var";
            default:
                return "Undefined";
            }
        }
    protected:
        DeclKind _DK;
        // DeclContext& _Ctx;
        explicit Decl(DeclKind _DK, DeclContext&_Ctx);
    public:
        virtual std::string className() const { return "Decl"; }
        virtual std::string getCode(int indent = 0) const;

        inline DeclKind getDeclKind() const { return _DK; }
        bool isFunctionDecl() const { return _DK == DeclKind::Function; }
        bool isStructDecl() const { return _DK == DeclKind::Struct; }
        bool isVarDecl() const { return _DK == DeclKind::Var; }
        // DeclContext& getDeclContext() const;
    };

    using NamedDeclPtr = std::unique_ptr<class NamedDecl>;
    class NamedDecl : public Decl {
    protected:
        std::string name;
        explicit NamedDecl(DeclKind DeclKind, std::string name, DeclContext& Ctx);
    public:
        const std::string& getName() const { return name; }
    };

    class TypeDecl : public NamedDecl {
        friend class DeclContext;
    protected:
        mutable TypePtr TypeForDecl_;
        // TODO: 未完成
        TypeDecl(DeclKind DeclKind, std::string name, TypePtr type, DeclContext& Ctx);
    public:
        virtual bool isStruct() const;
    };

    using ValueDeclPtr = std::unique_ptr<class ValueDecl>;
    using ValueDeclList = std::vector<ValueDeclPtr>;
    class ValueDecl : public NamedDecl {
    protected:
        QualType _Type;
        ValueDecl(DeclContext& Ctx, DeclKind DeclKind, std::string name, QualType type);
    public: 
        std::string getCode(int indent = 0) const override;

        QualType getType() const;
    };

    
    using VarDeclPtr = std::unique_ptr<class VarDecl>;
    class VarDecl : public ValueDecl {
        friend class DeclStmt;
        StmtList cast2C1(DeclContext& ctx);
    protected:
        ExprPtr _Init;
        VarDecl(DeclContext& Ctx, std::string name, QualType qt, ExprPtr _Init);
    public:
        static VarDeclPtr create(DeclContext& Ctx, std::string name, QualType qt, ExprPtr _Init = ExprPtr());
        virtual std::string className() const { return "VarDecl"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const;
        virtual std::string getCode(int indent = 0) const;

        //virtual void visit(tool::Solver& solver) override;
        //virtual StmtPtr cast2C1(DeclContext& ctx) override;

        Expr* getInitExpr();
        inline std::vector<int> getDimensions() const { return _Type->getDimensions(); }
        inline int getLenth() const { return getType()->getLenth(); }
        inline int getSize() const { return getType()->getSize(); }
        // int getAvAddress() const;
    };

    using StmtPtr = std::unique_ptr<class Stmt>;
    using StmtList = std::vector<StmtPtr>;
    class Stmt : public Node {
    protected:
        enum class StmtKind {
            ArraySubscriptExpr,
            BinaryOperator,
            CallExpr,
            CharLiteral,
            CompoundStmt,
            DeclRefExpr,
            DeclStmt,
            FltLiteral,
            IfStmt,
            ImplicitCastExpr,
            IntLiteral,
            MemberExpr,
            ReturnStmt,
            SignStmt,
            StrLiteral,
            UnaryOperatorExpr,
            Empty
        };
        StmtKind _SK;
        explicit Stmt(StmtKind StmtKind);
    public:
        static StmtPtr createEmpty();

        virtual std::string className() const { return "Stmt"; }
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;

        virtual StmtPtr cast2C1(DeclContext& ctx);
        virtual void visit(tool::Solver& _Solver);
    };

    using CompoundStmtPtr = std::unique_ptr<class CompoundStmt>;
    class CompoundStmt : public Stmt {
        friend class TranslationUnitDecl;
    protected:
        std::vector<StmtPtr> _Stmts;
        CompoundStmt(std::vector<StmtPtr> _Stmts);
    public:
        static CompoundStmtPtr create(std::vector<StmtPtr> _Stmts);
        static CompoundStmtPtr createEmpty();

        const std::vector<StmtPtr>& getStmts() const;
        void addStmt(StmtPtr _Stmt);
        void addStmts(StmtList stmts);
    };

    using DeclStmtPtr = std::unique_ptr<class DeclStmt>;
    class DeclStmt : public Stmt {
    protected:
        std::vector<Decl*> Decls_;
        explicit DeclStmt(Decl* D);
        explicit DeclStmt(std::vector<Decl*> Decls);
    public:
        static DeclStmtPtr create(Decl* D);
        static DeclStmtPtr create(std::vector<Decl*> Decls);
        template<class _Ty>
        static DeclStmtPtr create(const std::vector<std::unique_ptr<_Ty>>& Decls)
        {
            std::vector<Decl*> decls;
            for (const auto& D : Decls)
                decls.emplace_back(D.get());
            return DeclStmt::create(move(decls));
        }

        virtual std::string className() const { return "DeclStmt"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getCode(int indent = 0) const override;

        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;
    };

    class ValueStmt : public Stmt {
    protected:
        using Stmt::Stmt;
    public:
        // virtual void exec(tool::Solver& solver);
    };

    using SignStmtPtr = std::unique_ptr<class SignStmt>;
    class SignStmt : public Stmt {
    protected:
        ExprPtr _Cond;
        explicit SignStmt(ExprPtr _Cond);
    public:
        static SignStmtPtr create(ExprPtr _Cond);

        virtual std::string className() const { return "SignStmt"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getCode(int indent = 0) const;

        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;
    };

    using ExprPtr = std::unique_ptr<class Expr>;
    using ExprList = std::vector<ExprPtr>;
    class Expr : public ValueStmt {
    public:
        enum class ExprValueKind {
            LValue,
            RValue,
            XValue
        };
        static std::string ExprValueKind2Str(ExprValueKind vk);
    protected:
        QualType _Type;
        ExprValueKind _VK;
#if USE_CACHE
        mutable int m_mem = 0;
#endif
        Expr(StmtKind _StmtKind, QualType _QT, ExprValueKind _EVK);
    public:
        virtual std::string className() const { return "Expr"; }
        virtual std::string getAST() const override;
        virtual int getMem() const; // 起点
        // virtual VarDeclPtr getVarDecl() const; // 起点
        ExprPtr castExpr2C1(DeclContext& ctx);
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;

        virtual bool isExpr() const override;
        inline bool isLvExpr() const { return _VK == ExprValueKind::LValue; }
        inline bool isRvExpr() const { return _VK == ExprValueKind::RValue; }
        // For ArraySubscriptExpr
        virtual bool isArrayExpr() const;   // isArraySubscriptExpr
        // virtual const Expr* getSubscript() const;
        
        virtual bool isArrayRef() const;
        virtual bool isArrayPointer() const;
        virtual bool isConstant() const;
        virtual int getConstantValue() const;
        virtual bool isInitList() const;
        virtual const Expr* getExprWithoutCast() const;

        QualType getType() const;

        // TODO: 因为智能指针的引用缺陷，用op=实现op的方案不可取
        // 并且返回值的类型不一定等于左操作数
        //friend ExprPtr& operator+=(ExprPtr&, const ExprPtr&);
        //friend ExprPtr& operator*=(ExprPtr&, const ExprPtr&);
    };

    ExprPtr createIntLiteral(int v);

#ifndef DEFINE_EXPR_OP_EXPR
#define DEFINE_EXPR_OP_EXPR(op) \
    ExprPtr operator op(ExprPtr lhs, ExprPtr rhs); \
    inline ExprPtr operator op(ExprPtr lhs, int rhs) { return move(lhs) + createIntLiteral(rhs); } \
    inline ExprPtr operator op(int lhs, ExprPtr rhs) { return move(rhs) + lhs; }
    //template<typename _Ty1, typename _Ty2, \ 
    //typename _Ptr1 = std::shared_ptr<_Ty1>, \
    //    typename _Ptr2 = std::shared_ptr<_Ty2>> \
    //    inline _Ptr1 & operator##op## = (_Ptr1 & lower, _Ptr2 & right);
    // inline ExprPtr& operator##op##=(ExprPtr&, ExprPtr);
#endif // !DEFINE_EXPR_OP_EXPR
    DEFINE_EXPR_OP_EXPR(+)
    DEFINE_EXPR_OP_EXPR(-)
    DEFINE_EXPR_OP_EXPR(*)
    DEFINE_EXPR_OP_EXPR(/)
#undef DEFINE_EXPR_OP_EXPR

    using ArraySubscriptExprPtr = std::unique_ptr<class ArraySubscriptExpr>;
    class ArraySubscriptExpr : public Expr {
    protected:
        friend class UnaryOperator;
        ExprPtr _Base, _Sub;
        // int lower_bound = 0, upper_bound = 0;
        explicit ArraySubscriptExpr(ExprPtr _BS, ExprPtr _SS);
    public:
        static ArraySubscriptExprPtr create(ExprPtr _BS, ExprPtr _SS);
        static ArraySubscriptExprPtr create(VarDecl* _BS, ExprPtr _SS);
        // ArraySubscriptExpr& operator+=(const Expr& rhs);

        virtual std::string className() const { return "ArraySubscriptExpr"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual int getMem() const override;
        // virtual ValueDecl* getValueDecl() const;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;
        // virtual z3::expr* cast2z3(tool::Solver& solver);

        // ArraySubscriptExprPtr increaseSubscriptBy(Expr* Delta, Context* Ctx);
        virtual bool isArrayExpr() const;
        //void setBound(int lower, int upper);
        //void moveBound(int offset);
        //const Expr* getBase() const;
        //const Expr* getSubscript() const override;
        ArraySubscriptExprPtr moveBy(int distance);
    };

    using BinaryOperatorPtr = std::unique_ptr<class BinaryOperator>;
    class BinaryOperator : public Expr {
    public:
        enum class Bop {
            Assign,
            Add,
            Sub,
            Mul,
            Div,
            Add_As,
            Sub_As,
            Mul_As,
            Div_As,
            Mod,
            Unequal,
            Equal,
            Greater,
            Less,
            GE,
            LE,
            LogicAnd,
            LogicOr,
            Undefined
        };
        static std::string bop2Str(Bop op);
        static Bop str2BOp(const std::string& s);
        static bool isLogicalBop(Bop op);
        static bool isAssignmentBop(Bop op);
    protected:
        Bop _Op;
        ExprPtr _Lhs, _Rhs;
        explicit BinaryOperator(ExprPtr _Lhs, ExprPtr _Rhs, Bop _Op, QualType _Type);
    public:
        static BinaryOperatorPtr create(ExprPtr lhs, ExprPtr rhs, Bop op);
        static BinaryOperatorPtr create(ExprPtr lhs, ExprPtr rhs, const std::string& op);
        static ExprPtr createAndReduce(ExprPtr lhs, ExprPtr rhs, Bop op);
        static ExprPtr createAndReduce(ExprPtr lhs, ExprPtr rhs, const std::string& op);

        virtual std::string className() const { return "BinaryOperator"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const;
        virtual std::string getCode(int indent = 0) const;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;
        // virtual z3::expr* cast2z3(tool::Solver& solver);
    };

    std::ostream& operator<<(std::ostream& os, BinaryOperator::Bop op);

    // TODO: 暂时就先这样
    using CallExprPtr = std::unique_ptr<class CallExpr>;
    class CallExpr : public Expr {
        std::string _Callee;
    protected:
        explicit CallExpr(std::string _Callee, QualType _Type, ExprValueKind _EVK);
    public:
        static CallExprPtr create(std::string _Callee, QualType _Type, ExprValueKind _EVK = ExprValueKind::RValue);

        virtual std::string className() const { return "CallExpr"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const;
        virtual std::string getCode(int indent = 0) const;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;
    };

    using CastExprPtr = std::unique_ptr<class CastExpr>;
    class CastExpr : public Expr {
    public:
        enum class CastKind {
            ArrayToPointerDecay,
            DecayedToPointer,
            IntegralToPointer,
            LValueToRValue,
            PointerToIntegral
        };
        static std::string ck2Str(CastKind _CK);
    protected:
        CastKind _CK;
        ExprPtr _Origin;
        explicit CastExpr(StmtKind _SK, CastKind _CK, ExprPtr _Origin, QualType _Type, ExprValueKind _EVK);
    public:
        virtual std::string className() const { return "CastExpr"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const;
        virtual std::string getCode(int indent = 0) const;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;

        virtual bool isArrayRef() const override;
        virtual bool isConstant() const override;
        virtual int getConstantValue() const override;
        virtual const Expr* getExprWithoutCast() const override;
    };

    using CharLiteralPtr = std::unique_ptr<class CharLiteral>;
    class CharLiteral : public Expr {
    protected:
        int _Value;
        explicit CharLiteral(int _Value, QualType _Type);
    public:
        static CharLiteralPtr create(int _Value, QualType _Type = BuildInType::getChar());
        virtual std::string className() const { return "CharLiteral"; }
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual void visit(tool::Solver& _Solver) override;

        int getValue() const;
        virtual bool isConstant() const override;
        virtual int getConstantValue() const override;
    };

    using DeclRefExprPtr = std::unique_ptr<class DeclRefExpr>;
    class DeclRefExpr : public Expr {
    protected:
        ValueDecl* ValueDecl_;
        explicit DeclRefExpr(ValueDecl* VD);
    public:
        static DeclRefExprPtr create(ValueDecl* VD);

        virtual std::string className() const { return "DeclRefExpr"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;

        VarDecl* getVarDecl() const { return dynamic_cast<VarDecl*>(ValueDecl_); }
        virtual bool isArrayRef() const override;
    };

    using ImplicitCastExprPtr = std::unique_ptr<class ImplicitCastExpr>;
    class ImplicitCastExpr : public CastExpr {
    protected:
        explicit ImplicitCastExpr(CastKind _CK, ExprPtr _Origin, QualType _Type, ExprValueKind _EVK);
    public:
        static ImplicitCastExprPtr create(CastKind _CK, ExprPtr _Origin, QualType _Type, ExprValueKind _EVK);
        virtual std::string className() const { return "ImplicitCastExpr"; }

        static ExprPtr castExpr2(ExprPtr expr, QualType target_type);
        static ExprPtr cast2ArrayPointer(ExprPtr expr);
        static ExprPtr cast2RvExpr(ExprPtr expr);
        static ExprPtr castIntegral2Pointer(ExprPtr expr, QualType pointer_type);
        static ExprPtr castPointer2Integral(ExprPtr expr, QualType int_type = BuildInType::getInt());
    };

    using InitListExprPtr = std::unique_ptr<class InitListExpr>;
    class InitListExpr : public Expr {
    protected:
        friend class VarDecl;
        std::vector<ExprPtr> _InitList;
        // TODO: 类型往往不可知
        InitListExpr(std::vector<ExprPtr> init_list, QualType type);
        void setType(QualType type);
    public:
        static InitListExprPtr create(std::vector<ExprPtr> init_list, QualType type);
        virtual std::string className() const { return "InitListExpr"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;

        int getLenth() const;
        // ExprPtr at(int i) const;
        virtual bool isInitList() const override;
    };

    using IntLiteralPtr = std::unique_ptr<class IntLiteral>;
    class IntLiteral : public Expr {
    protected:
        int _Value;
        explicit IntLiteral(int _V, QualType _Type);
    public:
        static IntLiteralPtr create(int V, QualType _Type = BuildInType::getInt());
        virtual std::string className() const { return "IntLiteral"; }
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual void visit(tool::Solver& _Solver);

        int getValue() const;
        virtual bool isConstant() const;
        virtual int getConstantValue() const;
    };

    using StrLiteralPtr = std::unique_ptr<class StrLiteral>;
    class StrLiteral : public Expr {
    protected:
        std::string _Value;
        explicit StrLiteral(std::string _V, QualType _Type);
    public:
        static StrLiteralPtr create(std::string _V, QualType _Elem = BuildInType::getChar());
        virtual std::string className() const { return "StrLiteral"; }
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual void visit(tool::Solver& _Solver) override;

        const std::string& getValue() const;
        virtual bool isConstant() const;
        virtual int getConstantValue() const;
    };

    using UnaryOperatorPtr = std::unique_ptr<class UnaryOperator>;
    class UnaryOperator : public Expr {
    public:
        enum class Uop {
            Deref,      // 解引用
            LogicNeg,   // 非
            Neg,        // 负
            Ref,        // 取地址
            Undefined
        };
        static std::string uop2Str(Uop op);
        static Uop str2Uop(const std::string& s);
    protected:
        Uop _Op;
        ExprPtr Expr_;
        explicit UnaryOperator(ExprPtr expr, Uop op, QualType type, ExprValueKind evk);
    public:
        static UnaryOperatorPtr create(ExprPtr expr, Uop op);
        static UnaryOperatorPtr create(ExprPtr expr, const std::string& op);
        bool isPointerOperation() const;

        virtual std::string className() const { return "UnaryOperator"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        virtual void visit(tool::Solver& _Solver) override;
    };

    std::ostream& operator<<(std::ostream& os, UnaryOperator::Uop op);

    using TranslationUnitDeclPtr = std::unique_ptr<class TranslationUnitDecl>;
    using ASTContextPtr = std::unique_ptr<class ASTContext>;
    class ASTContext {
    protected:
        StmtList Stmts_;
        DeclContext& Ctx_;
        explicit ASTContext(DeclContext &ctx);
    public:
        static ASTContextPtr create(DeclContext& ctx);

        void cast2C1();
        void printAST(std::ostream& os) const;
        void printCode(std::ostream& os) const;
        const StmtList& getStmts() const;
        void pushStmt(StmtPtr stmt);
        // TranslationUnitDecl* getTranslationUnitDecl();
    };

    // 声明上下文，存储了类型和声明（变量）
    class DeclContext {
    protected:
        // 上下文的独特id
        int uid;
        // shared_ptr存储类型，只有具名类型会被存储
        // 包含NamedType和BuildInType
        std::map<std::string, TypePtr> types;
        // unique_ptr存储声明，仅包括变量
        // 后续可能有函数。StructDecl由StructType管理
        my_std::ordered_map<std::string, ValueDeclPtr> decls;
        // std::map<std::string, ValueDeclPtr> decls;
        VarDeclPtr global_array_decl;
        // 缓存值地址，VarDecl才有地址
        std::map<VarDecl*, int> cache;
        // TODO: 使用一个全局的上下文，用引用代替？
        DeclContext* parent = nullptr;
        explicit DeclContext(int uid, DeclContext* parent = nullptr);
    public:
        virtual ~DeclContext() {}/*= 0;*/
        const int getUid() const;
        // StmtPtr cast2C1(StmtPtr stmt);
        CharLiteralPtr createChar(int V = '?');
        IntLiteralPtr createInt(int _Value = 0);
        StrLiteralPtr createStr(const std::string& str);
        // For Type
        void addType(TypePtr type_ptr, const std::string& name = "");
        bool existsType(const std::string& name) const;
        const TypePtr getType(const std::string& name) const;
        inline QualType getChrType() const { return getType("char"); }
        inline QualType getFltType() const { return getType("float"); }
        inline QualType getIntType() const { return getType("int"); }
        inline QualType getStrType(int _Lenth) const
        {
            return ArrayType::get(_Lenth, getChrType());
        }
        inline QualType getVoidType() const { return getType("void"); }
        // For VarDecl
        void addDecl(ValueDeclPtr vard_ptr);
        bool existsDecl(const std::string& name, bool recursively = true) const;
        ValueDecl* getDecl(const std::string& name, bool recursively = true) const;
        // For global array decl
        void createGaDecl(int lenth = 0);
        VarDecl* getGaDecl() const;
        ArraySubscriptExprPtr getGaExpr(VarDecl* vard, int offset = 0);
    };

    class TranslationUnitDecl : public Decl, public DeclContext {
        /*NodeList _Children;*/
        explicit TranslationUnitDecl(DeclContext* parent = nullptr);
    public:
        static TranslationUnitDeclPtr create(DeclContext* parent = nullptr);
        virtual std::string className() const { return "TranslationUnitDecl"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getCode(int indent = 0) const override;
        //virtual StmtPtr cast2C1(DeclContext& ctx) override;
        //void visit(tool::Solver& _Solver) override;

        void dumpInfo() const;
        // void push(NodePtr _Node);
    };

    using FieldDeclPtr = std::unique_ptr<class FieldDecl>;
    using StructDeclPtr = std::unique_ptr<class StructDecl>;
    class StructDecl : public TypeDecl, public DeclContext {
        friend class FieldDecl;
    protected:
        StructTypePtr getStructType() const;
        StructDecl(std::string name, int uid, DeclContext& ctx);
    public:
        static StructDeclPtr create(std::string name, int uid, DeclContext& ctx);
        virtual std::string className() const { return "StructDecl"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getCode(int indent = 0) const;
        // virtual void visit(tool::Solver& solver);

        virtual bool isStruct() const override;
        void addFieldDecl(FieldDeclPtr FD);
        void addFieldDecls(std::vector<FieldDeclPtr> FDs);
        // void complete();
        // bool isCompleted() const;
        std::vector<FieldDecl*> getFieldDecls() const;
        FieldDecl* getFieldDecl(const std::string& fname) const;
        FieldDecl* getFieldDecl(int offset) const;
        bool existsFieldDecl(const std::string& fname) const;
        int getSize() const;
    };

    class FieldDecl : public ValueDecl {
    protected:
        // this is a cache
        int offset = 0;
        StructDecl& _Struct;
        FieldDecl(StructDecl& _Struct, std::string name, QualType type);
    public:
        static FieldDeclPtr create(StructDecl& _Struct, std::string name, QualType QT);
        static FieldDeclPtr create(DeclContext& _Struct, std::string name, QualType QT);
        virtual std::string className() const { return "FieldDecl"; }
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;

        int getFieldOffset() const;
        StructDecl& getParent() const;
    };

    using MemberExprPtr = std::unique_ptr<class MemberExpr>;
    class MemberExpr : public Expr {
        bool _IsArrow;
        ExprPtr _Base;
        FieldDecl* _MemberDecl;
        MemberExpr(ExprPtr BS, FieldDecl* MD, QualType QT, bool isArrow);
    public:
        static MemberExprPtr create(ExprPtr BS, FieldDecl* MD, bool isArrow = false);
        virtual std::string className() const { return "MemberExpr"; }
        virtual std::vector<Node*> getChildren() const override;
        virtual std::string getAST() const override;
        virtual std::string getCode(int indent = 0) const override;
        virtual StmtPtr cast2C1(DeclContext& ctx) override;
        void visit(tool::Solver& _Solver) override;

        virtual bool isArrayRef() const;
    };

#undef DEFINE_PTR
#undef DECL_UNIQUE_PTR
}

#endif // !EXPRESSION_H