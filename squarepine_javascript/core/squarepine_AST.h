//==============================================================================
#undef SP_LEXER_CREATE_INLINE_IDENTIFIER

/** */
#define SP_LEXER_CREATE_INLINE_IDENTIFIER(name) \
    static const Identifier name##Id = #name;

//==============================================================================
namespace tokenIds
{
    // Details:
    SP_LEXER_CREATE_INLINE_IDENTIFIER (expression)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (value)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (length)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (line)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (column)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (uniqueId)

    // Genericised Types:
    SP_LEXER_CREATE_INLINE_IDENTIFIER (type)

    SP_LEXER_CREATE_INLINE_IDENTIFIER (identifier)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (keyword)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (separator)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (operator)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (literal)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (comment)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (eof)
}

enum class TokenType
{
    identifierToken,
    keywordToken,
    separatorToken,
    operatorToken,
    literalToken,
    commentToken,
    eofToken
};

inline const Identifier& toIdentifier (TokenType type) noexcept
{
    switch (type)
    {
        case TokenType::identifierToken:    break;
        case TokenType::keywordToken:       return tokenIds::keywordId;
        case TokenType::separatorToken:     return tokenIds::separatorId;
        case TokenType::operatorToken:      return tokenIds::operatorId;
        case TokenType::literalToken:       return tokenIds::literalId;
        case TokenType::commentToken:       return tokenIds::commentId;
        case TokenType::eofToken:           return tokenIds::eofId;

        default:
            jassertfalse;
        break;
    };

    return tokenIds::identifierId;
}

inline String toString (TokenType type) noexcept
{
    return toIdentifier (type).toString();
}

//==============================================================================
namespace parserIds
{
    // Sub-Types:
    SP_LEXER_CREATE_INLINE_IDENTIFIER (number)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (function)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (object)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (string)

    // Even Subber-Types:
    SP_LEXER_CREATE_INLINE_IDENTIFIER (integral)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (unsigned)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (int8)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (int16)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (int32)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (int64)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (int128)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (int256)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (float)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (float16)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (float32)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (float64)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (float128)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (float256)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (infinity)
    SP_LEXER_CREATE_INLINE_IDENTIFIER (NaN)
}

//==============================================================================
inline bool isHexDigit (juce_wchar c) noexcept
{
    return (c >= '0' && c <= '9')
        || (c >= 'a' && c <= 'f')
        || (c >= 'A' && c <= 'F');
}

inline bool isOctalDigit (juce_wchar c) noexcept
{
    return c >= '0' && c <= '7';
}

inline bool isDecimalDigit (juce_wchar c) noexcept
{
    return c >= '0' && c <= '9';
}
