//==============================================================================
#define SP_JS_OPERATORS(X) \
    X(semicolon,            ";")   X(dot,               ".")   X(comma,        ",") \
    X(openParen,            "(")   X(closeParen,        ")")   X(openBrace,    "{")    X(closeBrace, "}") \
    X(openBracket,          "[")   X(closeBracket,      "]")   X(colon,        ":")    X(question,   "?") \
    X(typeEquals,           "===") X(equals,            "==")  X(assign,       "=") \
    X(typeNotEquals,        "!==") X(notEquals,         "!=")  X(logicalNot,   "!") \
    X(plusEquals,           "+=")  X(plusplus,          "++")  X(plus,         "+") \
    X(minusEquals,          "-=")  X(minusminus,        "--")  X(minus,        "-") \
    X(timesEquals,          "*=")  X(times,             "*")   X(divideEquals, "/=")   X(divide,     "/") \
    X(moduloEquals,         "%=")  X(modulo,            "%")   X(xorEquals,    "^=")   X(bitwiseXor, "^") \
    X(andEquals,            "&=")  X(logicalAnd,        "&&")  X(bitwiseAnd,   "&") \
    X(orEquals,             "|=")  X(logicalOr,         "||")  X(bitwiseOr,    "|") \
    X(leftShiftEquals,      "<<=") X(lessThanOrEqual,   "<=")  X(leftShift,    "<<")   X(lessThan,   "<") \
    X(rightShiftUnsigned,   ">>>") X(rightShiftEquals,  ">>=") X(rightShift,   ">>")   X(greaterThanOrEqual, ">=")  X(greaterThan,  ">")

#define SP_JS_KEYWORDS(X) \
    X(var,          "var")          X(if_,          "if")           X(else_,        "else")         X(do_,          "do")           X(null_,        "null") \
    X(while_,       "while")        X(for_,         "for")          X(break_,       "break")        X(continue_,    "continue")     X(undefined,    "undefined") \
    X(function,     "function")     X(return_,      "return")       X(true_,        "true")         X(false_,       "false")        X(new_,         "new") \
    X(typeof_,      "typeof")       X(instanceof_,  "instanceof")   X(infinity_,    "Infinity")     X(try_,         "try")          X(catch_,       "catch") \
    X(throw_,       "throw")        X(const_,       "const")        X(let_,         "let")          X(import_,      "import")       X(await_,       "await") \
    X(debugger_,    "debugger")     X(switch_,      "switch")       X(default_,     "default")      X(with_,        "with")         X(yield_,       "yield") \
    X(in_,          "in")           X(extends_,     "extends")      X(export_,      "export")       X(finally_,     "finally")      X(super_,       "super") \
    X(this_,        "this")         X(enum_,        "enum")         X(implements_,  "implements")   X(interface_,   "interface")    X(package_,     "package") \
    X(private_,     "private")      X(protected_,   "protected")    X(public_,      "public")       X(static_,      "static")       X(volatile_,    "volatile") \
    X(abstract_,    "abstract")     X(float_,       "float")        X(synchronized_,"synchronized") X(boolean_,     "boolean")      X(goto_,        "goto") \
    X(int_,         "int")          X(long_,        "long")         X(native_,      "native")       X(short_,       "short")        X(transient_,   "transient")

namespace TokenTypes
{
    #undef SP_DECLARE_JS_TOKEN
    #define SP_DECLARE_JS_TOKEN(name, str) \
        static const char* const name = str;

    SP_JS_KEYWORDS  (SP_DECLARE_JS_TOKEN)
    SP_JS_OPERATORS (SP_DECLARE_JS_TOKEN)
    SP_DECLARE_JS_TOKEN (eof,        "$eof")
    SP_DECLARE_JS_TOKEN (literal,    "$literal")
    SP_DECLARE_JS_TOKEN (identifier, "$identifier")

    #undef SP_DECLARE_JS_TOKEN
}

//==============================================================================
using Args = const var::NativeFunctionArgs&;
using TokenType = const char*;

//==============================================================================
static String getTokenName (TokenType t)                                        { return t[0] == '$' ? String (t + 1) : ("'" + String (t) + "'"); }
static bool isNumeric (const var& v) noexcept                                   { return v.isInt() || v.isDouble() || v.isInt64() || v.isBool(); }
static bool isNumericOrUndefined (const var& v) noexcept                        { return isNumeric (v) || v.isUndefined(); }
static int64 getOctalValue (const String& s)                                    { BigInteger b; b.parseString (s.initialSectionContainingOnly ("01234567"), 8); return b.toInt64(); }
static Identifier getPrototypeIdentifier()                                      { static const Identifier i ("prototype"); return i; }
static var* getPropertyPointer (DynamicObject& o, const Identifier& i) noexcept { return o.getProperties().getVarPointer (i); }

bool isFunction (const var& v) noexcept;

static bool areTypeEqual (const var& a, const var& b)
{
    if (auto* ado = dynamic_cast<JavascriptClass*> (a.getDynamicObject()))
        return ado->areSameValue (b);

    if (auto* bdo = dynamic_cast<JavascriptClass*> (b.getDynamicObject()))
        return bdo->areSameValue (a);

    return a.hasSameTypeAs (b)
        && isFunction (a) == isFunction (b)
        && (((a.isUndefined() || a.isVoid()) && (b.isUndefined() || b.isVoid())) || a == b);
}

//==============================================================================
static var get (Args a, int index) noexcept                 { return isPositiveAndBelow (index, a.numArguments) ? a.arguments[index] : var(); }
static bool isInt (Args a, int index) noexcept              { return get (a, index).isInt() || get (a, index).isInt64(); }
static int getInt (Args a, int index) noexcept              { return static_cast<int> (get (a, index)); }
static double getDouble (Args a, int index) noexcept        { return static_cast<double> (get (a, index)); }
static bool isString (Args a, int index) noexcept           { return get (a, index).isString(); }
static String getString (Args a, int index) noexcept        { return get (a, index).toString(); }
static var trace (Args a)                                   { Logger::outputDebugString (JSON::toString (a.thisObject)); return var::undefined(); }
static var charToInt (Args a)                               { return (int) getString (a, 0)[0]; }

static var encodeURI (Args a)
{
    if (! isString (a, 0) || a.numArguments > 1)
        return var::undefined();

    return URL::addEscapeChars (getString (a, 0), false);
}

static var encodeURIComponent (Args a)
{
    if (! isString (a, 0) || a.numArguments > 1)
        return var::undefined();

    return URL::addEscapeChars (getString (a, 0), true);
}

static var decodeURI (Args a)
{
    if (! isString (a, 0) || a.numArguments > 1)
        return var::undefined();

    return URL::removeEscapeChars (getString (a, 0));
}

static var decodeURIComponent (Args a)
{
    return decodeURI (a);
}

//==============================================================================
static var typeof_internal (Args a)
{
    const auto v = get (a, 0);

    if (v.isVoid())                      return "void";
    if (v.isString())                    return "string";
    if (v.isBool())                      return "boolean";
    if (isNumeric (v))                   return "number";
    if (isFunction (v) || v.isMethod())  return "function";
    if (v.isObject())                    return "object";

    return "undefined";
}

static var instanceof_internal (Args a)
{
    const auto v = get (a, 0);

    if (v.isVoid())                      return "void";
    if (v.isString())                    return "string";
    if (v.isBool())                      return "boolean";
    if (isNumeric (v))                   return "number";
    if (isFunction (v) || v.isMethod())  return "function";
    if (v.isObject())                    return "object";

    return "undefined";
}

static var exec (Args a)
{
    if (auto* root = dynamic_cast<RootObject*> (a.thisObject.getObject()))
        root->execute (getString (a, 0));

    return var::undefined();
}

static var eval (Args a)
{
    if (auto* root = dynamic_cast<RootObject*> (a.thisObject.getObject()))
        return root->evaluate (getString (a, 0));

    return exec (a);
}

static var setTimeout (Args a)
{
    if (auto* root = dynamic_cast<RootObject*> (a.thisObject.getObject()))
        root->setTimeoutInternal (getString (a, 0));

    return var::undefined();
}

//==============================================================================
struct CodeLocation final
{
    CodeLocation() = default;
    ~CodeLocation() = default;

    CodeLocation (const String& code) noexcept :
        program (code),
        location (program.getCharPointer())
    {
    }

    CodeLocation (const CodeLocation& other) noexcept :
        program (other.program),
        location (other.location)
    {
    }

    CodeLocation (CodeLocation&& other) noexcept :
        program (std::move (other.program)),
        location (std::move (other.location))
    {
    }

    CodeLocation& operator= (const CodeLocation& other) noexcept
    {
        if (this != &other)
        {
            program = other.program;
            location = other.location;
        }

        return *this;
    }

    CodeLocation& operator= (CodeLocation&& other) noexcept
    {
        if (this != &other)
        {
            program = std::move (other.program);
            location = std::move (other.location);
        }

        return *this;
    }

    void throwError (const String& message) const
    {
        int col = 1, line = 1;

        for (auto i = program.getCharPointer(); i < location && ! i.isEmpty(); ++i)
        {
            ++col;

            if (*i == '\n')
            {
                col = 1;
                ++line;
            }
        }

        throw "Line " + String (line) + ", column " + String (col) + " : " + message;
    }

    String program;
    String::CharPointerType location;
};

//==============================================================================
struct Scope final
{
    Scope (const Scope* p, ReferenceCountedObjectPtr<RootObject> rt, DynamicObject::Ptr scp) noexcept :
        parent (p),
        root (std::move (rt)),
        scope (std::move (scp))
    {
    }

    const Scope* const parent;
    ReferenceCountedObjectPtr<RootObject> root;
    DynamicObject::Ptr scope;

    var findFunctionCall (const CodeLocation& location, const var& targetObject, const Identifier& functionName) const;
    var* findRootClassProperty (const Identifier& className, const Identifier& propName) const;
    var findSymbolInParentScopes (const Identifier& name) const;
    bool findAndInvokeMethod (const Identifier& function, const var::NativeFunctionArgs& args, var& result) const;
    bool invokeMethod (const var& m, const var::NativeFunctionArgs& args, var& result) const;
    void checkTimeOut (const CodeLocation& location) const;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Scope)
};

//==============================================================================
struct Statement
{
    Statement (const CodeLocation& l) noexcept : location (l) {}
    virtual ~Statement() {}

    enum class ResultCode
    {
        ok = 0,
        returnWasHit,
        breakWasHit,
        continueWasHit
    };

    virtual ResultCode perform (const Scope&, var*) const  { return ResultCode::ok; }

    CodeLocation location;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Statement)
};

//==============================================================================
struct Expression : public Statement
{
    Expression (const CodeLocation& l) noexcept : Statement (l) {}

    virtual var getResult (const Scope&) const                  { return var::undefined(); }
    virtual void assign (const Scope&, const var&) const        { location.throwError ("Cannot assign to this expression!"); }
    ResultCode perform (const Scope& s, var*) const override    { getResult (s); return ResultCode::ok; }
};

using ExpPtr = std::unique_ptr<Expression>;

//==============================================================================
struct BlockStatement final : public Statement
{
    BlockStatement (const CodeLocation& l) noexcept : Statement (l) {}

    ResultCode perform (const Scope& s, var* returnedValue) const override
    {
        for (auto* statement : statements)
        {
            const auto r = statement->perform (s, returnedValue);
            if (r != ResultCode::ok)
                return r;
        }

        return ResultCode::ok;
    }

    OwnedArray<Statement> statements;
};

struct IfStatement final : public Statement
{
    IfStatement (const CodeLocation& l) noexcept : Statement (l) {}

    ResultCode perform (const Scope& s, var* returnedValue) const override
    {
        return (condition->getResult(s) ? trueBranch : falseBranch)->perform (s, returnedValue);
    }

    ExpPtr condition;
    std::unique_ptr<Statement> trueBranch, falseBranch;
};

struct VarStatement final : public Statement
{
    VarStatement (const CodeLocation& l) noexcept : Statement (l) {}

    ResultCode perform (const Scope& s, var*) const override
    {
        s.scope->setProperty (name, initialiser->getResult (s));
        return ResultCode::ok;
    }

    Identifier name;
    ExpPtr initialiser;
};

struct LoopStatement final : public Statement
{
    LoopStatement (const CodeLocation& l, bool isDo) noexcept : Statement (l), isDoLoop (isDo) {}

    ResultCode perform (const Scope& s, var* returnedValue) const override
    {
        initialiser->perform (s, nullptr);

        while (isDoLoop || condition->getResult (s))
        {
            s.checkTimeOut (location);
            auto r = body->perform (s, returnedValue);

            if (r == ResultCode::returnWasHit)      return r;
            else if (r == ResultCode::breakWasHit)  break;

            iterator->perform (s, nullptr);

            if (isDoLoop && r != ResultCode::continueWasHit && ! condition->getResult (s))
                break;
        }

        return ResultCode::ok;
    }

    std::unique_ptr<Statement> initialiser, iterator, body;
    ExpPtr condition;
    bool isDoLoop;
};

struct ReturnStatement final : public Statement
{
    ReturnStatement (const CodeLocation& l, Expression* v) noexcept : Statement (l), returnValue (v) {}

    ResultCode perform (const Scope& s, var* ret) const override
    {
        if (ret != nullptr)  *ret = returnValue->getResult (s);
        return ResultCode::returnWasHit;
    }

    ExpPtr returnValue;
};

struct BreakStatement final : public Statement
{
    BreakStatement (const CodeLocation& l) noexcept : Statement (l) {}
    ResultCode perform (const Scope&, var*) const override  { return ResultCode::breakWasHit; }
};

struct ContinueStatement final : public Statement
{
    ContinueStatement (const CodeLocation& l) noexcept : Statement (l) {}
    ResultCode perform (const Scope&, var*) const override  { return ResultCode::continueWasHit; }
};

struct LiteralValue final : public Expression
{
    LiteralValue (const CodeLocation& l, const var& v) noexcept : Expression (l), value (v) {}
    var getResult (const Scope&) const override   { return value; }
    var value;
};

struct UnqualifiedName final : public Expression
{
    UnqualifiedName (const CodeLocation& l, const Identifier& n) noexcept : Expression (l), name (n) {}

    var getResult (const Scope& s) const override  { return s.findSymbolInParentScopes (name); }

    void assign (const Scope& s, const var& newValue) const override
    {
        if (auto* v = getPropertyPointer (*s.scope, name))
            *v = newValue;
        else
            s.root->setProperty (name, newValue);
    }

    Identifier name;
};

//==============================================================================
struct DotOperator final : public Expression
{
    DotOperator (const CodeLocation& l, ExpPtr& p, const Identifier& c) noexcept : Expression (l), parent (p.release()), child (c) {}

    var getResult (const Scope& s) const override
    {
        auto p = parent->getResult (s);
        static const Identifier lengthID ("length");

        if (child == lengthID)
        {
            if (auto* array = p.getArray())   return array->size();
            if (p.isString())                 return p.toString().length();
        }

        if (auto* o = p.getDynamicObject())
            if (auto* v = getPropertyPointer (*o, child))
                return *v;

        return var::undefined();
    }

    void assign (const Scope& s, const var& newValue) const override
    {
        if (auto* o = parent->getResult (s).getDynamicObject())
            o->setProperty (child, newValue);
        else
            Expression::assign (s, newValue);
    }

    ExpPtr parent;
    Identifier child;
};

//==============================================================================
struct ArraySubscript final : public Expression
{
    ArraySubscript (const CodeLocation& l) noexcept : Expression (l) {}

    var getResult (const Scope& s) const override
    {
        auto arrayVar = object->getResult (s); // must stay alive for the scope of this method
        auto key = index->getResult (s);

        if (const auto* array = arrayVar.getArray())
            if (key.isInt() || key.isInt64() || key.isDouble())
                return (*array) [static_cast<int> (key)];

        if (auto* o = arrayVar.getDynamicObject())
            if (key.isString())
                if (auto* v = getPropertyPointer (*o, Identifier (key)))
                    return *v;

        return var::undefined();
    }

    void assign (const Scope& s, const var& newValue) const override
    {
        auto arrayVar = object->getResult (s); // must stay alive for the scope of this method
        auto key = index->getResult (s);

        if (auto* array = arrayVar.getArray())
        {
            if (key.isInt() || key.isInt64() || key.isDouble())
            {
                const auto i = static_cast<int64> (key);
                while (static_cast<int64> (array->size()) < i)
                    array->add (var::undefined());

                array->set (static_cast<int> (i), newValue);
                return;
            }
        }

        if (auto* o = arrayVar.getDynamicObject())
        {
            if (key.isString())
            {
                o->setProperty (key.toString(), newValue);
                return;
            }
        }

        Expression::assign (s, newValue);
    }

    ExpPtr object, index;
};

//==============================================================================
struct BinaryOperatorBase : public Expression
{
    BinaryOperatorBase (const CodeLocation& l, ExpPtr& a, ExpPtr& b, TokenType op) noexcept
        : Expression (l), lhs (a.release()), rhs (b.release()), operation (op) {}

    ExpPtr lhs, rhs;
    TokenType operation;
};

struct BinaryOperator : public BinaryOperatorBase
{
    BinaryOperator (const CodeLocation& l, ExpPtr& a, ExpPtr& b, TokenType op) noexcept
        : BinaryOperatorBase (l, a, b, op) {}

    virtual var getWithUndefinedArg() const                           { return var::undefined(); }
    virtual var getWithDoubles (double, double) const                 { return throwError ("Double"); }
    virtual var getWithInts (int64, int64) const                      { return throwError ("Integer"); }
    virtual var getWithArrayOrObject (const var& a, const var&) const { return throwError (a.isArray() ? "Array" : "Object"); }
    virtual var getWithStrings (const String&, const String&) const   { return throwError ("String"); }

    var getResult (const Scope& s) const override
    {
        var a (lhs->getResult (s)), b (rhs->getResult (s));

        if ((a.isUndefined() || a.isVoid()) && (b.isUndefined() || b.isVoid()))
            return getWithUndefinedArg();

        if (isNumericOrUndefined (a) && isNumericOrUndefined (b))
            return (a.isDouble() || b.isDouble()) ? getWithDoubles (a, b) : getWithInts (a, b);

        if (a.isArray() || a.isObject())
            return getWithArrayOrObject (a, b);

        return getWithStrings (a.toString(), b.toString());
    }

    var throwError (const char* typeName) const
    {
        location.throwError (getTokenName (operation) + " is not allowed on the " + typeName + " type");
        return {};
    }
};

//==============================================================================
struct EqualsOp final : public BinaryOperator
{
    EqualsOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::equals) {}
    var getWithUndefinedArg() const override                               { return true; }
    var getWithDoubles (double a, double b) const override                 { return a == b; }
    var getWithInts (int64 a, int64 b) const override                      { return a == b; }
    var getWithStrings (const String& a, const String& b) const override   { return a == b; }
    var getWithArrayOrObject (const var& a, const var& b) const override   { return a == b; }
};

struct NotEqualsOp final : public BinaryOperator
{
    NotEqualsOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::notEquals) {}
    var getWithUndefinedArg() const override                               { return false; }
    var getWithDoubles (double a, double b) const override                 { return a != b; }
    var getWithInts (int64 a, int64 b) const override                      { return a != b; }
    var getWithStrings (const String& a, const String& b) const override   { return a != b; }
    var getWithArrayOrObject (const var& a, const var& b) const override   { return a != b; }
};

#undef JUCE_JAVASCRIPT_DEFINE_OP
#define JUCE_JAVASCRIPT_DEFINE_OP(name, operationToPerform, tokenTypeToUse) \
    struct name final : public BinaryOperator \
    { \
        name (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::tokenTypeToUse) {} \
        var getWithDoubles (double a, double b) const override                 { return a operationToPerform b; } \
        var getWithInts (int64 a, int64 b) const override                      { return a operationToPerform b; } \
        var getWithStrings (const String& a, const String& b) const override   { return a operationToPerform b; } \
    };

JUCE_JAVASCRIPT_DEFINE_OP (LessThanOp, <, lessThan)
JUCE_JAVASCRIPT_DEFINE_OP (LessThanOrEqualOp, <=, lessThanOrEqual)
JUCE_JAVASCRIPT_DEFINE_OP (GreaterThanOp, >, greaterThan)
JUCE_JAVASCRIPT_DEFINE_OP (GreaterThanOrEqualOp, >=, greaterThanOrEqual)
JUCE_JAVASCRIPT_DEFINE_OP (AdditionOp, +, plus)

#undef JUCE_JAVASCRIPT_DEFINE_OP

struct SubtractionOp final : public BinaryOperator
{
    SubtractionOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::minus) {}
    var getWithDoubles (double a, double b) const override { return a - b; }
    var getWithInts (int64 a, int64 b) const override      { return a - b; }
};

struct MultiplyOp final : public BinaryOperator
{
    MultiplyOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::times) {}
    var getWithDoubles (double a, double b) const override { return a * b; }
    var getWithInts (int64 a, int64 b) const override      { return a * b; }
};

struct DivideOp final : public BinaryOperator
{
    DivideOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::divide) {}
    var getWithDoubles (double a, double b) const override  { return b != 0 ? a / b : std::numeric_limits<double>::infinity(); }
    var getWithInts (int64 a, int64 b) const override       { return b != 0 ? var ((double) a / (double) b) : var (std::numeric_limits<double>::infinity()); }
};

struct ModuloOp final : public BinaryOperator
{
    ModuloOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::modulo) {}
    var getWithDoubles (double a, double b) const override  { return b != 0 ? std::fmod (a, b) : std::numeric_limits<double>::infinity(); }
    var getWithInts (int64 a, int64 b) const override       { return b != 0 ? var (a % b) : var (std::numeric_limits<double>::infinity()); }
};

struct BitwiseOrOp final : public BinaryOperator
{
    BitwiseOrOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::bitwiseOr) {}
    var getWithInts (int64 a, int64 b) const override   { return a | b; }
};

struct BitwiseAndOp final : public BinaryOperator
{
    BitwiseAndOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::bitwiseAnd) {}
    var getWithInts (int64 a, int64 b) const override   { return a & b; }
};

struct BitwiseXorOp final : public BinaryOperator
{
    BitwiseXorOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::bitwiseXor) {}
    var getWithInts (int64 a, int64 b) const override   { return a ^ b; }
};

struct LeftShiftOp final : public BinaryOperator
{
    LeftShiftOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::leftShift) {}
    var getWithInts (int64 a, int64 b) const override   { return a << b; }
};

struct RightShiftOp final : public BinaryOperator
{
    RightShiftOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::rightShift) {}
    var getWithInts (int64 a, int64 b) const override   { return a >> b; }
};

struct RightShiftUnsignedOp final : public BinaryOperator
{
    RightShiftUnsignedOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperator (l, a, b, TokenTypes::rightShiftUnsigned) {}
    var getWithInts (int64 a, int64 b) const override   { return (int) (((uint32) a) >> (int) b); }
};

struct LogicalAndOp final : public BinaryOperatorBase
{
    LogicalAndOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperatorBase (l, a, b, TokenTypes::logicalAnd) {}
    var getResult (const Scope& s) const override       { return lhs->getResult (s) && rhs->getResult (s); }
};

struct LogicalOrOp final : public BinaryOperatorBase
{
    LogicalOrOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperatorBase (l, a, b, TokenTypes::logicalOr) {}
    var getResult (const Scope& s) const override       { return lhs->getResult (s) || rhs->getResult (s); }
};

//==============================================================================
struct TypeEqualsOp final : public BinaryOperatorBase
{
    TypeEqualsOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperatorBase (l, a, b, TokenTypes::typeEquals) {}
    var getResult (const Scope& s) const override       { return areTypeEqual (lhs->getResult (s), rhs->getResult (s)); }
};

struct TypeNotEqualsOp final : public BinaryOperatorBase
{
    TypeNotEqualsOp (const CodeLocation& l, ExpPtr& a, ExpPtr& b) noexcept : BinaryOperatorBase (l, a, b, TokenTypes::typeNotEquals) {}
    var getResult (const Scope& s) const override       { return ! areTypeEqual (lhs->getResult (s), rhs->getResult (s)); }
};

//==============================================================================
struct ConditionalOp final : public Expression
{
    ConditionalOp (const CodeLocation& l) noexcept : Expression (l) {}

    var getResult (const Scope& s) const override              { return (condition->getResult (s) ? trueBranch : falseBranch)->getResult (s); }
    void assign (const Scope& s, const var& v) const override  { (condition->getResult (s) ? trueBranch : falseBranch)->assign (s, v); }

    ExpPtr condition, trueBranch, falseBranch;
};

//==============================================================================
struct Assignment final : public Expression
{
    Assignment (const CodeLocation& l, ExpPtr& dest, ExpPtr& source) noexcept : Expression (l), target (dest.release()), newValue (source.release()) {}

    var getResult (const Scope& s) const override
    {
        const auto value = newValue->getResult (s);
        target->assign (s, value);
        return value;
    }

    ExpPtr target, newValue;
};

//==============================================================================
struct SelfAssignment : public Expression
{
    SelfAssignment (const CodeLocation& l, Expression* dest, Expression* source) noexcept
        : Expression (l), target (dest), newValue (source) {}

    var getResult (const Scope& s) const override
    {
        auto value = newValue->getResult (s);
        target->assign (s, value);
        return value;
    }

    Expression* target; // Careful! this pointer aliases a sub-term of newValue!
    ExpPtr newValue;
    TokenType op;
};

//==============================================================================
struct PostAssignment final : public SelfAssignment
{
    PostAssignment (const CodeLocation& l, Expression* dest, Expression* source) noexcept : SelfAssignment (l, dest, source) {}

    var getResult (const Scope& s) const override
    {
        auto oldValue = target->getResult (s);
        target->assign (s, newValue->getResult (s));
        return oldValue;
    }
};

//==============================================================================
struct FunctionCall : public Expression
{
    FunctionCall (const CodeLocation& l) noexcept : Expression (l) {}

    var getResult (const Scope& s) const override
    {
        if (auto* dot = dynamic_cast<DotOperator*> (object.get()))
        {
            const auto thisObject = dot->parent->getResult (s);
            return invokeFunction (s, s.findFunctionCall (location, thisObject, dot->child), thisObject);
        }

        const auto function = object->getResult (s);
        return invokeFunction (s, function, var (s.scope.get()));
    }

    var invokeFunction (const Scope& s, const var& function, const var& thisObject) const;

    ExpPtr object;
    OwnedArray<Expression> arguments;
};

//==============================================================================
struct NewOperator final : public FunctionCall
{
    NewOperator (const CodeLocation& l, const Identifier& cID) noexcept : FunctionCall (l), classId (cID) {}

    var getResult (const Scope& s) const override;

    const Identifier classId;
};

//==============================================================================
struct ObjectDeclaration final : public Expression
{
    ObjectDeclaration (const CodeLocation& l) noexcept : Expression (l) {}

    var getResult (const Scope& s) const override
    {
        DynamicObject::Ptr newObject (new DynamicObject());

        for (int i = 0; i < names.size(); ++i)
            newObject->setProperty (names.getUnchecked (i), initialisers.getUnchecked (i)->getResult (s));

        return newObject.get();
    }

    Array<Identifier> names;
    OwnedArray<Expression> initialisers;
};

//==============================================================================
struct ArrayDeclaration final : public Expression
{
    ArrayDeclaration (const CodeLocation& l) noexcept : Expression (l) {}

    var getResult (const Scope& s) const override
    {
        Array<var> a;

        for (int i = 0; i < values.size(); ++i)
            a.add (values.getUnchecked(i)->getResult (s));

        // std::move() needed here for older compilers
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wredundant-move")
        return std::move (a);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    OwnedArray<Expression> values;
};

//==============================================================================
struct FunctionObject final : public DynamicObject
{
    FunctionObject() noexcept {}
    FunctionObject (const FunctionObject& other);

    DynamicObject::Ptr clone() override { return *new FunctionObject (*this); }

    void writeAsJSON (OutputStream& out, int /*indentLevel*/, bool /*allOnOneLine*/, int /*maximumDecimalPlaces*/) override
    {
        out << "function " << functionCode;
    }

    var invoke (const Scope& s, const var::NativeFunctionArgs& args) const
    {
        DynamicObject::Ptr functionRoot (new DynamicObject());

        static const Identifier thisIdent ("this");
        functionRoot->setProperty (thisIdent, args.thisObject);

        for (int i = 0; i < parameters.size(); ++i)
            functionRoot->setProperty (parameters.getReference(i),
                                        i < args.numArguments ? args.arguments[i] : var::undefined());

        var result;
        body->perform (Scope (&s, s.root, functionRoot), &result);
        return result;
    }

    String functionCode;
    Array<Identifier> parameters;
    std::unique_ptr<Statement> body;
};

bool isFunction (const var& v) noexcept
{
    return dynamic_cast<FunctionObject*> (v.getObject()) != nullptr;
}

//==============================================================================
struct TokenIterator
{
    TokenIterator (const String& code) : location (code), p (code.getCharPointer()) { skip(); }
    virtual ~TokenIterator() {}

    void skip()
    {
        skipWhitespaceAndComments();
        location.location = p;
        currentType = matchNextToken();
    }

    void match (TokenType expected)
    {
        if (currentType != expected)
            location.throwError ("Found " + getTokenName (currentType) + " when expecting " + getTokenName (expected));

        skip();
    }

    bool matchIf (TokenType expected)                                 { if (currentType == expected) { skip(); return true; } return false; }
    bool matchesAny (TokenType t1, TokenType t2) const                { return currentType == t1 || currentType == t2; }
    bool matchesAny (TokenType t1, TokenType t2, TokenType t3) const  { return matchesAny (t1, t2) || currentType == t3; }

    CodeLocation location;
    TokenType currentType;
    var currentValue;

private:
    String::CharPointerType p;

    static bool isIdentifierStart (juce_wchar c) noexcept   { return CharacterFunctions::isLetter (c) || c == '_'; }
    static bool isIdentifierBody (juce_wchar c) noexcept    { return CharacterFunctions::isLetterOrDigit (c) || c == '_'; }

    TokenType matchNextToken()
    {
        if (isIdentifierStart (*p))
        {
            auto end = p;
            while (isIdentifierBody (*++end)) {}

            auto len = (size_t) (end - p);

            #undef SP_JS_COMPARE_KEYWORD
            #define SP_JS_COMPARE_KEYWORD(name, str) \
                if (len == sizeof (str) - 1 && matchToken (TokenTypes::name, len)) \
                    return TokenTypes::name;

            SP_JS_KEYWORDS (SP_JS_COMPARE_KEYWORD)
            #undef SP_JS_COMPARE_KEYWORD

            currentValue = String (p, end); p = end;
            return TokenTypes::identifier;
        }

        if (p.isDigit())
        {
            if (parseHexLiteral() || parseFloatLiteral() || parseOctalLiteral() || parseDecimalLiteral())
                return TokenTypes::literal;

            location.throwError ("Syntax error in numeric constant");
        }

        if (parseStringLiteral (*p) || (*p == '.' && parseFloatLiteral()))
            return TokenTypes::literal;

        #undef SP_JS_COMPARE_OPERATOR
        #define SP_JS_COMPARE_OPERATOR(name, str) \
            if (matchToken (TokenTypes::name, sizeof (str) - 1)) \
                return TokenTypes::name;

        SP_JS_OPERATORS (SP_JS_COMPARE_OPERATOR)
        #undef SP_JS_COMPARE_OPERATOR

        if (! p.isEmpty())
            location.throwError ("Unexpected character '" + String::charToString (*p) + "' in source");

        return TokenTypes::eof;
    }

    bool matchToken (TokenType name, size_t len) noexcept
    {
        if (p.compareUpTo (CharPointer_ASCII (name), (int) len) != 0)
            return false;

        p += (int) len;
        return true;
    }

    void skipWhitespaceAndComments()
    {
        for (;;)
        {
            p = p.findEndOfWhitespace();

            if (*p == '/')
            {
                auto c2 = p[1];

                if (c2 == '/')
                {
                    p = CharacterFunctions::find (p, (juce_wchar) '\n');
                    continue;
                }

                if (c2 == '*')
                {
                    location.location = p;
                    p = CharacterFunctions::find (p + 2, CharPointer_ASCII ("*/"));

                    if (p.isEmpty())
                        location.throwError ("Unterminated '/*' comment");

                    p += 2;
                    continue;
                }
            }

            break;
        }
    }

    bool parseStringLiteral (juce_wchar quoteType)
    {
        if (quoteType != '"' && quoteType != '\'')
            return false;

        auto r = JSON::parseQuotedString (p, currentValue);
        if (r.failed())
            location.throwError (r.getErrorMessage());

        return true;
    }

    bool parseHexLiteral()
    {
        if (*p != '0' || (p[1] != 'x' && p[1] != 'X'))
            return false;

        auto t = ++p;
        auto v = (int64) CharacterFunctions::getHexDigitValue (*++t);
        if (v < 0)
            return false;

        for (;;)
        {
            const auto digit = CharacterFunctions::getHexDigitValue (*++t);
            if (digit < 0)
                break;

            v = v * 16 + (int64) digit;
        }

        currentValue = v;
        p = t;
        return true;
    }

    bool parseFloatLiteral()
    {
        int64 numDigits = 0;
        auto t = p;
        while (t.isDigit())
        {
            ++t;
            ++numDigits;
        }

        const bool hasPoint = *t == '.';

        if (hasPoint)
            while ((++t).isDigit())
                ++numDigits;

        if (numDigits == 0)
            return false;

        auto c = *t;
        const bool hasExponent = c == 'e' || c == 'E';

        if (hasExponent)
        {
            c = *++t;

            if (c == '+' || c == '-')
                ++t;

            if (! t.isDigit())
                return false;

            while ((++t).isDigit()) {}
        }

        if (! (hasExponent || hasPoint))
            return false;

        currentValue = CharacterFunctions::getDoubleValue (p);
        p = t;
        return true;
    }

    bool parseOctalLiteral()
    {
        auto t = p;
        auto v = static_cast<int64> (*t - '0');
        if (v != 0)
            return false;  // first digit of octal must be 0

        for (;;)
        {
            const auto digit = (int64) (*++t - '0');
            if (isPositiveAndBelow (digit, 8))        v = v * 8 + digit;
            else if (isPositiveAndBelow (digit, 10))  location.throwError ("Decimal digit in octal constant");
            else break;
        }

        currentValue = v;
        p = t;
        return true;
    }

    bool parseDecimalLiteral()
    {
        int64 v = 0;

        for (;; ++p)
        {
            auto digit = (int64) (*p - '0');

            if (isPositiveAndBelow (digit, 10))
                v = v * 10 + digit;
            else
                break;
        }

        currentValue = v;
        return true;
    }
};

//==============================================================================
struct ExpressionTreeBuilder final : private TokenIterator
{
    ExpressionTreeBuilder (const String code) : TokenIterator (code) {}

    BlockStatement* parseStatementList()
    {
        auto* b = new BlockStatement (location);

        while (currentType != TokenTypes::closeBrace && currentType != TokenTypes::eof)
            b->statements.add (parseStatement());

        return b;
    }

    void parseFunctionParamsAndBody (FunctionObject& fo)
    {
        match (TokenTypes::openParen);

        while (currentType != TokenTypes::closeParen)
        {
            auto paramName = currentValue.toString();
            match (TokenTypes::identifier);
            fo.parameters.add (paramName);

            if (currentType != TokenTypes::closeParen)
                match (TokenTypes::comma);
        }

        match (TokenTypes::closeParen);
        fo.body.reset (parseBlock());
    }

    Expression* parseExpression()
    {
        ExpPtr lhs (parseLogicOperator());

        if (matchIf (TokenTypes::question))          return parseTernaryOperator (lhs);
        if (matchIf (TokenTypes::assign))            { ExpPtr rhs (parseExpression()); return new Assignment (location, lhs, rhs); }
        if (matchIf (TokenTypes::plusEquals))        return parseInPlaceOpExpression<AdditionOp> (lhs);
        if (matchIf (TokenTypes::minusEquals))       return parseInPlaceOpExpression<SubtractionOp> (lhs);
        if (matchIf (TokenTypes::timesEquals))       return parseInPlaceOpExpression<MultiplyOp> (lhs);
        if (matchIf (TokenTypes::divideEquals))      return parseInPlaceOpExpression<DivideOp> (lhs);
        if (matchIf (TokenTypes::moduloEquals))      return parseInPlaceOpExpression<ModuloOp> (lhs);
        if (matchIf (TokenTypes::leftShiftEquals))   return parseInPlaceOpExpression<LeftShiftOp> (lhs);
        if (matchIf (TokenTypes::rightShiftEquals))  return parseInPlaceOpExpression<RightShiftOp> (lhs);

        return lhs.release();
    }

private:
    void throwError (const String& err) const { location.throwError (err); }

    template<typename OpType>
    Expression* parseInPlaceOpExpression (ExpPtr& lhs)
    {
        ExpPtr rhs (parseExpression());
        auto* bareLHS = lhs.get(); // careful - bare pointer is deliberately aliased
        return new SelfAssignment (location, bareLHS, new OpType (location, lhs, rhs));
    }

    BlockStatement* parseBlock()
    {
        match (TokenTypes::openBrace);
        auto* b = parseStatementList();
        match (TokenTypes::closeBrace);
        return b;
    }

    Statement* parseStatement()
    {
        if (currentType == TokenTypes::openBrace)                       return parseBlock();
        if (matchIf (TokenTypes::var) || matchIf (TokenTypes::let_))    return parseVar();
        if (matchIf (TokenTypes::if_))                                  return parseIf();
        if (matchIf (TokenTypes::while_))                               return parseDoOrWhileLoop (false);
        if (matchIf (TokenTypes::do_))                                  return parseDoOrWhileLoop (true);
        if (matchIf (TokenTypes::for_))                                 return parseForLoop();
        if (matchIf (TokenTypes::return_))                              return parseReturn();
        if (matchIf (TokenTypes::break_))                               return new BreakStatement (location);
        if (matchIf (TokenTypes::continue_))                            return new ContinueStatement (location);
        if (matchIf (TokenTypes::function))                             return parseFunction();
        if (matchIf (TokenTypes::semicolon))                            return new Statement (location);
        if (matchIf (TokenTypes::plusplus))                             return parsePreIncDec<AdditionOp>();
        if (matchIf (TokenTypes::minusminus))                           return parsePreIncDec<SubtractionOp>();

        if (matchesAny (TokenTypes::openParen, TokenTypes::openBracket))
            return matchEndOfStatement (parseFactor());

        if (matchesAny (TokenTypes::identifier, TokenTypes::literal, TokenTypes::minus))
            return matchEndOfStatement (parseExpression());

        throwError ("Found " + getTokenName (currentType) + " when expecting a statement");
        return nullptr;
    }

    /**
        @see http://inimino.org/~inimino/blog/javascript_semicolons
    */
    Expression* matchEndOfStatement (Expression* ex)
    {
        ExpPtr e (ex);
        if (currentType != TokenTypes::eof)
            match (TokenTypes::semicolon);

        return e.release();
    }

    Expression* matchCloseParen (Expression* ex)
    {
        ExpPtr e (ex);
        match (TokenTypes::closeParen);
        return e.release();
    }

    Statement* parseIf()
    {
        auto* s = new IfStatement (location);
        match (TokenTypes::openParen);
        s->condition.reset (parseExpression());
        match (TokenTypes::closeParen);
        s->trueBranch.reset (parseStatement());
        s->falseBranch.reset (matchIf (TokenTypes::else_) ? parseStatement() : new Statement (location));
        return s;
    }

    Statement* parseReturn()
    {
        if (matchIf (TokenTypes::semicolon))
            return new ReturnStatement (location, new Expression (location));

        auto* r = new ReturnStatement (location, parseExpression());
        matchIf (TokenTypes::semicolon);
        return r;
    }

    Statement* parseVar()
    {
        auto* s = new VarStatement (location);
        s->name = parseIdentifier();
        s->initialiser.reset (matchIf (TokenTypes::assign) ? parseExpression() : new Expression (location));

        if (matchIf (TokenTypes::comma))
        {
            auto* block = new BlockStatement (location);
            block->statements.add (std::move (s));
            block->statements.add (parseVar());
            return block;
        }

        match (TokenTypes::semicolon);
        return s;
    }

    Statement* parseFunction()
    {
        Identifier name;
        auto fn = parseFunctionDefinition (name);

        if (name.isNull())
            throwError ("Functions defined at statement-level must have a name");

        ExpPtr nm (new UnqualifiedName (location, name)), value (new LiteralValue (location, fn));
        return new Assignment (location, nm, value);
    }

    Statement* parseForLoop()
    {
        auto* s = new LoopStatement (location, false);
        match (TokenTypes::openParen);
        s->initialiser.reset (parseStatement());

        if (matchIf (TokenTypes::semicolon))
        {
            s->condition.reset (new LiteralValue (location, true));
        }
        else
        {
            s->condition.reset (parseExpression());
            match (TokenTypes::semicolon);
        }

        if (matchIf (TokenTypes::closeParen))
        {
            s->iterator.reset (new Statement (location));
        }
        else
        {
            s->iterator.reset (parseExpression());
            match (TokenTypes::closeParen);
        }

        s->body.reset (parseStatement());
        return s;
    }

    Statement* parseDoOrWhileLoop (bool isDoLoop)
    {
        auto* s = new LoopStatement (location, isDoLoop);
        s->initialiser.reset (new Statement (location));
        s->iterator.reset (new Statement (location));

        if (isDoLoop)
        {
            s->body.reset (parseBlock());
            match (TokenTypes::while_);
        }

        match (TokenTypes::openParen);
        s->condition.reset (parseExpression());
        match (TokenTypes::closeParen);

        if (! isDoLoop)
            s->body.reset (parseStatement());

        return s;
    }

    Identifier parseIdentifier()
    {
        Identifier i;
        if (currentType == TokenTypes::identifier)
            i = currentValue.toString();

        match (TokenTypes::identifier);
        return i;
    }

    var parseFunctionDefinition (Identifier& functionName)
    {
        auto functionStart = location.location;

        if (currentType == TokenTypes::identifier)
            functionName = parseIdentifier();

        auto fo = std::make_unique<FunctionObject>();
        parseFunctionParamsAndBody (*fo);
        fo->functionCode = String (functionStart, location.location);
        return var (fo.release());
    }

    Expression* parseFunctionCall (FunctionCall* call, ExpPtr& function)
    {
        call->object.reset (function.release());
        match (TokenTypes::openParen);

        while (currentType != TokenTypes::closeParen)
        {
            call->arguments.add (parseExpression());
            if (currentType != TokenTypes::closeParen)
                match (TokenTypes::comma);
        }

        return matchCloseParen (call);
    }

    Expression* parseSuffixes (Expression* e)
    {
        ExpPtr input (e);

        if (matchIf (TokenTypes::dot))
            return parseSuffixes (new DotOperator (location, input, parseIdentifier()));

        if (currentType == TokenTypes::openParen)
            return parseSuffixes (parseFunctionCall (new FunctionCall (location), input));

        if (matchIf (TokenTypes::openBracket))
        {
            auto* s = new ArraySubscript (location);
            s->object.reset (input.release());
            s->index.reset (parseExpression());
            match (TokenTypes::closeBracket);
            return parseSuffixes (s);
        }

        if (matchIf (TokenTypes::plusplus))   return parsePostIncDec<AdditionOp> (input);
        if (matchIf (TokenTypes::minusminus)) return parsePostIncDec<SubtractionOp> (input);

        return input.release();
    }

    Expression* parseFactor()
    {
        if (currentType == TokenTypes::identifier)  return parseSuffixes (new UnqualifiedName (location, parseIdentifier()));
        if (matchIf (TokenTypes::openParen))        return parseSuffixes (matchCloseParen (parseExpression()));
        if (matchIf (TokenTypes::true_))            return parseSuffixes (new LiteralValue (location, (int) 1));
        if (matchIf (TokenTypes::false_))           return parseSuffixes (new LiteralValue (location, (int) 0));
        if (matchIf (TokenTypes::null_))            return parseSuffixes (new LiteralValue (location, var()));
        if (matchIf (TokenTypes::undefined))        return parseSuffixes (new Expression (location));

        if (currentType == TokenTypes::literal)
        {
            auto v = currentValue;
            skip();

            return parseSuffixes (new LiteralValue (location, v));
        }

        if (matchIf (TokenTypes::openBrace))
        {
            auto e = std::make_unique<ObjectDeclaration> (location);

            while (currentType != TokenTypes::closeBrace)
            {
                auto memberName = currentValue.toString();
                match (currentType == TokenTypes::literal && currentValue.isString()
                        ? TokenTypes::literal : TokenTypes::identifier);
                match (TokenTypes::colon);

                e->names.add (memberName);
                e->initialisers.add (parseExpression());

                if (currentType != TokenTypes::closeBrace)
                    match (TokenTypes::comma);
            }

            match (TokenTypes::closeBrace);
            return parseSuffixes (e.release());
        }

        if (matchIf (TokenTypes::openBracket))
        {
            auto e = std::make_unique<ArrayDeclaration> (location);

            while (currentType != TokenTypes::closeBracket)
            {
                e->values.add (parseExpression());

                if (currentType != TokenTypes::closeBracket)
                    match (TokenTypes::comma);
            }

            match (TokenTypes::closeBracket);
            return parseSuffixes (e.release());
        }

        if (matchIf (TokenTypes::function))
        {
            Identifier name;
            auto fn = parseFunctionDefinition (name);

            if (name.isValid())
                throwError ("Inline functions definitions cannot have a name");

            return new LiteralValue (location, fn);
        }

        if (matchIf (TokenTypes::new_))
        {
            const auto classId = parseIdentifier();
            ExpPtr name (new UnqualifiedName (location, classId));

            while (matchIf (TokenTypes::dot))
                name.reset (new DotOperator (location, name, parseIdentifier()));

            return parseFunctionCall (new NewOperator (location, classId), name);
        }

        throwError ("Found " + getTokenName (currentType) + " when expecting an expression.");
        return nullptr;
    }

    template<typename OpType>
    Expression* parsePreIncDec()
    {
        auto* e = parseFactor(); // careful - bare pointer is deliberately aliased
        ExpPtr lhs (e), one (new LiteralValue (location, (int) 1));
        return new SelfAssignment (location, e, new OpType (location, lhs, one));
    }

    template<typename OpType>
    Expression* parsePostIncDec (ExpPtr& lhs)
    {
        auto* e = lhs.release(); // careful - bare pointer is deliberately aliased
        ExpPtr lhs2 (e), one (new LiteralValue (location, (int) 1));
        return new PostAssignment (location, e, new OpType (location, lhs2, one));
    }

    Expression* parseTypeof()
    {
        auto f = std::make_unique<FunctionCall> (location);
        f->object.reset (new UnqualifiedName (location, "typeof"));
        f->arguments.add (parseUnary());
        return f.release();
    }

    Expression* parseUnary()
    {
        if (matchIf (TokenTypes::minus))       { ExpPtr a (new LiteralValue (location, (int) 0)), b (parseUnary()); return new SubtractionOp   (location, a, b); }
        if (matchIf (TokenTypes::logicalNot))  { ExpPtr a (new LiteralValue (location, (int) 0)), b (parseUnary()); return new EqualsOp        (location, a, b); }
        if (matchIf (TokenTypes::plusplus))    return parsePreIncDec<AdditionOp>();
        if (matchIf (TokenTypes::minusminus))  return parsePreIncDec<SubtractionOp>();
        if (matchIf (TokenTypes::typeof_))     return parseTypeof();

        return parseFactor();
    }

    Expression* parseMultiplyDivide()
    {
        ExpPtr a (parseUnary());

        for (;;)
        {
            if (matchIf (TokenTypes::times))        { ExpPtr b (parseUnary()); a.reset (new MultiplyOp (location, a, b)); }
            else if (matchIf (TokenTypes::divide))  { ExpPtr b (parseUnary()); a.reset (new DivideOp   (location, a, b)); }
            else if (matchIf (TokenTypes::modulo))  { ExpPtr b (parseUnary()); a.reset (new ModuloOp   (location, a, b)); }
            else break;
        }

        return a.release();
    }

    Expression* parseAdditionSubtraction()
    {
        ExpPtr a (parseMultiplyDivide());

        for (;;)
        {
            if (matchIf (TokenTypes::plus))            { ExpPtr b (parseMultiplyDivide()); a.reset (new AdditionOp    (location, a, b)); }
            else if (matchIf (TokenTypes::minus))      { ExpPtr b (parseMultiplyDivide()); a.reset (new SubtractionOp (location, a, b)); }
            else break;
        }

        return a.release();
    }

    Expression* parseShiftOperator()
    {
        ExpPtr a (parseAdditionSubtraction());

        for (;;)
        {
            if (matchIf (TokenTypes::leftShift))                { ExpPtr b (parseExpression()); a.reset (new LeftShiftOp          (location, a, b)); }
            else if (matchIf (TokenTypes::rightShift))          { ExpPtr b (parseExpression()); a.reset (new RightShiftOp         (location, a, b)); }
            else if (matchIf (TokenTypes::rightShiftUnsigned))  { ExpPtr b (parseExpression()); a.reset (new RightShiftUnsignedOp (location, a, b)); }
            else break;
        }

        return a.release();
    }

    Expression* parseComparator()
    {
        ExpPtr a (parseShiftOperator());

        for (;;)
        {
            if (matchIf (TokenTypes::equals))                  { ExpPtr b (parseShiftOperator()); a.reset (new EqualsOp             (location, a, b)); }
            else if (matchIf (TokenTypes::notEquals))          { ExpPtr b (parseShiftOperator()); a.reset (new NotEqualsOp          (location, a, b)); }
            else if (matchIf (TokenTypes::typeEquals))         { ExpPtr b (parseShiftOperator()); a.reset (new TypeEqualsOp         (location, a, b)); }
            else if (matchIf (TokenTypes::typeNotEquals))      { ExpPtr b (parseShiftOperator()); a.reset (new TypeNotEqualsOp      (location, a, b)); }
            else if (matchIf (TokenTypes::lessThan))           { ExpPtr b (parseShiftOperator()); a.reset (new LessThanOp           (location, a, b)); }
            else if (matchIf (TokenTypes::lessThanOrEqual))    { ExpPtr b (parseShiftOperator()); a.reset (new LessThanOrEqualOp    (location, a, b)); }
            else if (matchIf (TokenTypes::greaterThan))        { ExpPtr b (parseShiftOperator()); a.reset (new GreaterThanOp        (location, a, b)); }
            else if (matchIf (TokenTypes::greaterThanOrEqual)) { ExpPtr b (parseShiftOperator()); a.reset (new GreaterThanOrEqualOp (location, a, b)); }
            else break;
        }

        return a.release();
    }

    Expression* parseLogicOperator()
    {
        ExpPtr a (parseComparator());

        for (;;)
        {
            if (matchIf (TokenTypes::logicalAnd))       { ExpPtr b (parseComparator()); a.reset (new LogicalAndOp (location, a, b)); }
            else if (matchIf (TokenTypes::logicalOr))   { ExpPtr b (parseComparator()); a.reset (new LogicalOrOp  (location, a, b)); }
            else if (matchIf (TokenTypes::bitwiseAnd))  { ExpPtr b (parseComparator()); a.reset (new BitwiseAndOp (location, a, b)); }
            else if (matchIf (TokenTypes::bitwiseOr))   { ExpPtr b (parseComparator()); a.reset (new BitwiseOrOp  (location, a, b)); }
            else if (matchIf (TokenTypes::bitwiseXor))  { ExpPtr b (parseComparator()); a.reset (new BitwiseXorOp (location, a, b)); }
            else break;
        }

        return a.release();
    }

    Expression* parseTernaryOperator (ExpPtr& condition)
    {
        auto e = std::make_unique<ConditionalOp> (location);
        e->condition.reset (condition.release());
        e->trueBranch.reset (parseExpression());
        match (TokenTypes::colon);
        e->falseBranch.reset (parseExpression());
        return e.release();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExpressionTreeBuilder)
};

//==============================================================================
FunctionObject::FunctionObject (const FunctionObject& other) :
    functionCode (other.functionCode)
{
    ExpressionTreeBuilder tb (functionCode);
    tb.parseFunctionParamsAndBody (*this);
}

//==============================================================================
var FunctionCall::invokeFunction (const Scope& s, const var& function, const var& thisObject) const
{
    s.checkTimeOut (location);
    Array<var> argVars;

    for (auto* a : arguments)
        argVars.add (a->getResult (s));

    const var::NativeFunctionArgs args (thisObject, argVars.begin(), argVars.size());

    if (auto nativeFunction = function.getNativeFunction())
        return nativeFunction (args);

    if (auto* fo = dynamic_cast<FunctionObject*> (function.getObject()))
        return fo->invoke (s, args);

    if (auto* dot = dynamic_cast<DotOperator*> (object.get()))
        if (auto* o = thisObject.getDynamicObject())
            if (o->hasMethod (dot->child)) // allow an overridden DynamicObject::invokeMethod to accept a method call.
                return o->invokeMethod (dot->child, args);

    location.throwError ("This expression is not a function!");
    return var::undefined();
}

var* Scope::findRootClassProperty (const Identifier& className, const Identifier& propName) const
{
    if (auto* cls = root->getProperty (className).getDynamicObject())
        return getPropertyPointer (*cls, propName);

    return nullptr;
}

var Scope::findSymbolInParentScopes (const Identifier& name) const
{
    if (auto v = getPropertyPointer (*scope, name))
        return *v;

    return parent != nullptr
            ? parent->findSymbolInParentScopes (name)
            : var::undefined();
}

bool Scope::findAndInvokeMethod (const Identifier& function, const var::NativeFunctionArgs& args, var& result) const
{
    auto* target = args.thisObject.getDynamicObject();

    if (target == nullptr || target == scope.get())
    {
        if (auto* m = getPropertyPointer (*scope, function))
        {
            if (auto fo = dynamic_cast<FunctionObject*> (m->getObject()))
            {
                result = fo->invoke (*this, args);
                return true;
            }
        }
    }

    const auto& props = scope->getProperties();

    for (int i = 0; i < props.size(); ++i)
        if (auto* o = props.getValueAt (i).getDynamicObject())
            if (Scope (this, *root, *o).findAndInvokeMethod (function, args, result))
                return true;

    return false;
}

bool Scope::invokeMethod (const var& m, const var::NativeFunctionArgs& args, var& result) const
{
    if (isFunction (m))
    {
        auto* target = args.thisObject.getDynamicObject();

        if (target == nullptr || target == scope.get())
        {
            if (auto fo = dynamic_cast<FunctionObject*> (m.getObject()))
            {
                result = fo->invoke (*this, args);
                return true;
            }
        }
    }

    return false;
}

void Scope::checkTimeOut (const CodeLocation& location) const
{
    if (Time::getCurrentTime() > root->timeout)
        location.throwError (root->timeout == Time() ? "Interrupted" : "Execution timed-out");
}

//==============================================================================
