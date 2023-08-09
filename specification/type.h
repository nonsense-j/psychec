#ifndef TOOL_TYPE_H
#define TOOL_TYPE_H

#include <ostream>
#include <string>
#include <vector>

// Type -> NamedType, ArrayType, PointerType
// BaseType -> BuildInType, StructType, ...
// 仅有NamedType会被Context记录，其余用shared_ptr管理其生命周期
// TODO: 生成Context记录外的NamedType如int，应该被允许吗？

namespace tool {
#define DECL_SHARED_PTR(T) using T##Ptr = std::shared_ptr<class T>;

    DECL_SHARED_PTR(Type)

    class QualType {
        TypePtr _TypePtr;
    public:
        // 无参、复制、移动构造函数以及复制、移动赋值函数均为默认
        QualType() = default;
        QualType(TypePtr _Type);
        QualType(const QualType& other) = default;
        QualType(QualType&& other) noexcept = default;
        ~QualType() = default;
        QualType& operator=(const QualType& other) = default;
        QualType& operator=(QualType&& other) = default;
        bool operator==(const QualType& other) const;
        explicit operator bool() const noexcept { return _TypePtr.operator bool(); }
        Type const* const operator->() const noexcept;
        Type const& operator*() const noexcept;
        friend std::ostream& operator<<(std::ostream& os, const QualType& T)
        {
            return os << T.getAsString();
        }

        std::string getAsString() const;
        std::string getAsString(const std::string& _ValueName) const;
        const QualType getBaseType() const;
        Type const* const get() const { return _TypePtr.get(); }
        const class StructType* getAsStructType() const;
    };

    class Type {
    protected:
        // 子类可以改写
        int _Bytes = 1;                 // 现版本下，char和int都是一字节 
        explicit Type(int _Bytes = 1);  // 保护的默认构造函数
        bool operator==(const Type& other) const;
        virtual const QualType getBaseType() const;
    public:
        Type(const Type& other) = delete;
        Type(Type&& other) = delete;
        virtual ~Type() = 0;
        Type& operator=(const Type& other) = delete;
        Type& operator=(Type&& other) = delete;

        // virtual const std::string getAsString() const;
        virtual const std::vector<int> getDimensions() const;
        int getElementSize() const;
        virtual const QualType getElementType() const;
        virtual int getLenth() const;
        virtual int getLevel() const;
        virtual const std::string getName() const;
        int getPointeeSize() const;
        virtual const QualType getPointeeType() const;
        virtual const QualType getPointerDecay() const;
        virtual int getSize() const;
        virtual int getTotalLevel() const;

        // 谓词
        virtual bool isArray() const;
        virtual bool isChar() const;
        virtual bool isDecayed() const;
        virtual bool isFloat() const;
        virtual bool isInt() const;
        virtual bool isNamed() const;
        virtual bool isPointer() const;
        virtual bool isStruct() const;

        friend class QualType;
    };

    DECL_SHARED_PTR(BuildInType)
    class BuildInType : public Type {
        protected:
            enum class Kind {
                Char,
                Float,
                Int,
                Undefined
            };
            constexpr static auto kind_num = (unsigned long long)Kind::Undefined;
            constexpr static int kind_size[kind_num]
                = { 1, 1, 1 };
            constexpr static std::string_view kind_name[kind_num]
                = { "char", "float", "int" };
            Kind _Kind;
            BuildInType(int _Size, Kind _Kind);
            virtual const QualType getBaseType() const;
        public:
            static const TypePtr getChar();
            static const TypePtr getFloat();
            static const TypePtr getInt();
            bool operator==(const BuildInType& other) const;

            virtual const std::string getName() const;

            virtual bool isChar() const;
            virtual bool isFloat() const;
            virtual bool isInt() const;
            virtual bool isNamed() const;
    };

    DECL_SHARED_PTR(ArrayType)
    class ArrayType : public Type {
    protected:
        QualType Elem_;
        explicit ArrayType(int _Lenth, QualType _Elem);
        const int lenth() const { return _Bytes; }
        int& lenth() { return _Bytes; }
        virtual const QualType getBaseType() const;
    public:
        static QualType get(int _Lenth, QualType _Elem);
        static QualType get(const std::vector<int>& Dim, QualType _Elem);
        static QualType getStr(int _Lenth, QualType _Elem = BuildInType::getChar());
        bool operator==(const ArrayType& other) const;

        // virtual const std::string getAsString() const;
        virtual const std::vector<int> getDimensions() const;
        virtual const QualType getElementType() const;
        virtual int getLenth() const;
        virtual const QualType getPointerDecay() const;
        virtual int getSize() const;
        virtual int getTotalLevel() const;

        virtual bool isArray() const;
    };

    // 具名类型，会存在Context中，包含StructType等
    DECL_SHARED_PTR(NamedType)
    class NamedType : public Type {
    protected:
        std::string _Name;
        NamedType(std::string _Name, int _Size);
        virtual const QualType getBaseType() const;
    public:
        static const QualType get(std::string _Name, int _Size);
        bool operator==(const NamedType& other) const;

        virtual const std::string getName() const;

        virtual bool isNamed() const;
    };

    DECL_SHARED_PTR(PointerType)
    class PointerType : public Type {
    protected:
        constexpr static int pointer_size = 1;
        QualType _Pointee;
        explicit PointerType(QualType _Pointee);
        virtual const QualType getBaseType() const;
    public:
        static const QualType get(QualType Pointee, int level = 1);
        bool operator==(const PointerType& other) const;

        // virtual const std::string getAsString() const;
        virtual const std::vector<int> getDimensions() const;
        virtual int getLevel() const;
        virtual const QualType getPointeeType() const;
        virtual int getTotalLevel() const;

        virtual bool isPointer() const;
    };

    DECL_SHARED_PTR(DecayedType)
    class DecayedType : public PointerType {
    protected:
        int _Lenth = 1;
        explicit DecayedType(QualType _Array);
        explicit DecayedType(const ArrayType* _AT);
        // virtual const QualType getBaseType() const;
    public:
        static const QualType get(QualType _Array);
        static const QualType get(const ArrayType* _AT);
        bool operator==(const DecayedType& other) const;

        // virtual const std::string getAsString() const;
        virtual int getLenth() const;

        virtual bool isDecayed() const;
    };

    DECL_SHARED_PTR(StructType)
    class StructType : public NamedType {
    protected:
        class StructDecl* Decl_;
        explicit StructType(StructDecl* SD);
        // virtual const Type* getBaseType() const;
    public:
        static TypePtr get(StructDecl* SD);
        bool operator==(const StructType& other) const;
        StructDecl* getStructDecl() const;

        // virtual const std::string getAsString() const;
        virtual int getSize() const override;

        virtual bool isStruct() const override;
    };

#undef DECL_SHARED_PTR
}

#endif // !TOOL_TYPE_H