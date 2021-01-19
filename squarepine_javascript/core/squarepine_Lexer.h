//==============================================================================
template<typename Iterator>
static bool skipNumberSuffix (Iterator& source)
{
    auto c = source.peekNextChar();

    if (c == 'l' || c == 'L' || c == 'u' || c == 'U')
        source.skip();

    return ! CharacterFunctions::isLetterOrDigit (source.peekNextChar());
}

template<typename Iterator>
static bool parseHexLiteral (Iterator& source) noexcept
{
    if (source.peekNextChar() == '-')
        source.skip();

    if (source.nextChar() != '0')
        return false;

    auto c = source.nextChar();

    if (c != 'x' && c != 'X')
        return false;

    int64 numDigits = 0;

    while (isHexDigit (source.peekNextChar()))
    {
        ++numDigits;
        source.skip();
    }

    if (numDigits == 0)
        return false;

    return skipNumberSuffix (source);
}

template<typename Iterator>
static bool parseOctalLiteral (Iterator& source) noexcept
{
    if (source.peekNextChar() == '-')
        source.skip();

    if (source.nextChar() != '0')
        return false;

    if (! isOctalDigit (source.nextChar()))
        return false;

    while (isOctalDigit (source.peekNextChar()))
        source.skip();

    return skipNumberSuffix (source);
}

template<typename Iterator>
static bool parseDecimalLiteral (Iterator& source) noexcept
{
    if (source.peekNextChar() == '-')
        source.skip();

    int numChars = 0;
    while (isDecimalDigit (source.peekNextChar()))
    {
        ++numChars;
        source.skip();
    }

    if (numChars == 0)
        return false;

    return skipNumberSuffix (source);
}

template<typename Iterator>
static bool parseFloatLiteral (Iterator& source) noexcept
{
    if (source.peekNextChar() == '-')
        source.skip();

    int64 numDigits = 0;

    while (isDecimalDigit (source.peekNextChar()))
    {
        source.skip();
        ++numDigits;
    }

    const bool hasPoint = source.peekNextChar() == '.';

    if (hasPoint)
    {
        source.skip();

        while (isDecimalDigit (source.peekNextChar()))
        {
            source.skip();
            ++numDigits;
        }
    }

    if (numDigits == 0)
        return false;

    auto c = source.peekNextChar();
    const bool hasExponent = c == 'e' || c == 'E';

    if (hasExponent)
    {
        source.skip();
        c = source.peekNextChar();

        if (c == '+' || c == '-')
            source.skip();

        int numExpDigits = 0;

        while (isDecimalDigit (source.peekNextChar()))
        {
            source.skip();
            ++numExpDigits;
        }

        if (numExpDigits == 0)
            return false;
    }

    c = source.peekNextChar();

    if (c == 'f' || c == 'F')
        source.skip();
    else if (! (hasExponent || hasPoint))
        return false;

    return true;
}

template<typename Iterator>
static void skipQuotedString (Iterator& source) noexcept
{
    auto quote = source.nextChar();

    for (;;)
    {
        auto c = source.nextChar();

        if (c == quote || c == 0)
            break;

        if (c == '\\')
            source.skip();
    }
}

template<typename Iterator>
static void skipIfNextCharMatches (Iterator& source, juce_wchar c) noexcept
{
    if (source.peekNextChar() == c)
        source.skip();
}

template<typename Iterator>
static void skipIfNextCharMatches (Iterator& source, juce_wchar c1, juce_wchar c2) noexcept
{
    auto c = source.peekNextChar();

    if (c == c1 || c == c2)
        source.skip();
}

//==============================================================================
/** A class that can be passed to the GenericLexer in order to parse a String. */
struct StringIterator
{
    StringIterator (const String& s) noexcept            : t (s.getCharPointer()) {}
    StringIterator (String::CharPointerType s) noexcept  : t (s) {}

    juce_wchar nextChar() noexcept      { if (isEOF()) return 0; ++numChars; return t.getAndAdvance(); }
    juce_wchar peekNextChar() noexcept  { return *t; }
    void skip() noexcept                { if (! isEOF()) { ++t; ++numChars; } }
    void skipWhitespace() noexcept      { while (t.isWhitespace()) skip(); }
    void skipToEndOfLine() noexcept     { while (*t != '\r' && *t != '\n' && *t != 0) skip(); }
    bool isEOF() const noexcept         { return t.isEmpty(); }

    String::CharPointerType t;
    int numChars = 0;
};

//==============================================================================
/** Takes a UTF8 string and writes it to a stream using standard C++ escape sequences for any
    non-ascii bytes.

    Although not strictly a tokenising function, this is still a function that often comes in
    handy when working with C++ code!

    Note that addEscapeChars() is easier to use than this function if you're working with Strings.

    @see addEscapeChars
*/
static void writeEscapeChars (OutputStream& out, const char* utf8, int numBytesToRead,
                                int maxCharsOnLine, bool breakAtNewLines,
                                bool replaceSingleQuotes, bool allowStringBreaks)
{
    int charsOnLine = 0;
    bool lastWasHexEscapeCode = false;
    bool trigraphDetected = false;

    for (int i = 0; i < numBytesToRead || numBytesToRead < 0; ++i)
    {
        auto c = (unsigned char) utf8[i];
        bool startNewLine = false;

        switch (c)
        {
            case '\t':  out << "\\t";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
            case '\r':  out << "\\r";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
            case '\n':  out << "\\n";  trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; startNewLine = breakAtNewLines; break;
            case '\\':  out << "\\\\"; trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;
            case '\"':  out << "\\\""; trigraphDetected = false; lastWasHexEscapeCode = false; charsOnLine += 2; break;

            case '?':
                if (trigraphDetected)
                {
                    out << "\\?";
                    charsOnLine++;
                    trigraphDetected = false;
                }
                else
                {
                    out << "?";
                    trigraphDetected = true;
                }

                lastWasHexEscapeCode = false;
                charsOnLine++;
                break;

            case 0:
                if (numBytesToRead < 0)
                    return;

                out << "\\0";
                lastWasHexEscapeCode = true;
                trigraphDetected = false;
                charsOnLine += 2;
                break;

            case '\'':
                if (replaceSingleQuotes)
                {
                    out << "\\\'";
                    lastWasHexEscapeCode = false;
                    trigraphDetected = false;
                    charsOnLine += 2;
                    break;
                }
                // deliberate fall-through...
                JUCE_FALLTHROUGH

            default:
                if (c >= 32 && c < 127 && ! (lastWasHexEscapeCode  // (have to avoid following a hex escape sequence with a valid hex digit)
                                                && CharacterFunctions::getHexDigitValue (c) >= 0))
                {
                    out << (char) c;
                    lastWasHexEscapeCode = false;
                    trigraphDetected = false;
                    ++charsOnLine;
                }
                else if (allowStringBreaks && lastWasHexEscapeCode && c >= 32 && c < 127)
                {
                    out << "\"\"" << (char) c;
                    lastWasHexEscapeCode = false;
                    trigraphDetected = false;
                    charsOnLine += 3;
                }
                else
                {
                    out << (c < 16 ? "\\x0" : "\\x") << String::toHexString ((int) c);
                    lastWasHexEscapeCode = true;
                    trigraphDetected = false;
                    charsOnLine += 4;
                }

                break;
        }

        if ((startNewLine || (maxCharsOnLine > 0 && charsOnLine >= maxCharsOnLine))
                && (numBytesToRead < 0 || i < numBytesToRead - 1))
        {
            charsOnLine = 0;
            out << "\"" << newLine << "\"";
            lastWasHexEscapeCode = false;
        }
    }
}

/** Takes a string and returns a version of it where standard C++ escape sequences have been
    used to replace any non-ascii bytes.

    Although not strictly a tokenising function, this is still a function that often comes in
    handy when working with C++ code!

    @see writeEscapeChars
*/
static String addEscapeChars (const String& s)
{
    MemoryOutputStream mo;
    writeEscapeChars (mo, s.toRawUTF8(), -1, -1, false, true, true);
    return mo.toString();
}

//==============================================================================
/** */
template<typename IteratorType>
class GenericLexer
{
public:
    //==============================================================================
    /** */
    using Iterator = IteratorType;

    //==============================================================================
    /** */
    GenericLexer() = default;
    /** */
    virtual ~GenericLexer() = default;

    //==============================================================================
    virtual bool isIdentifierStart (juce_wchar c) const noexcept
    {
        return CharacterFunctions::isLetter (c) || c == '_' || c == '@';
    }

    virtual bool isIdentifierBody (juce_wchar c) const noexcept
    {
        return CharacterFunctions::isLetterOrDigit (c) || c == '_' || c == '@';
    }

    virtual bool isReservedKeyword (String::CharPointerType token, int64 tokenLength) const noexcept
    {
        ignoreUnused (token, tokenLength);
        return false;
    }

    virtual int64 getMinimumKeywordLength() const noexcept { return 2; }
    virtual int64 getMaximumKeywordLength() const noexcept { return 30; }

    virtual ValueTree parseIdentifier (Iterator& source) noexcept
    {
        int64 tokenLength = 0;
        String::CharPointerType::CharType possibleIdentifier[100];
        String::CharPointerType possible (possibleIdentifier);

        while (isIdentifierBody (source.peekNextChar()))
        {
            const auto c = source.nextChar();

            if (tokenLength < getMaximumKeywordLength())
                possible.write (c);

            ++tokenLength;
        }

        if (tokenLength >= getMinimumKeywordLength() && tokenLength <= getMaximumKeywordLength())
        {
            possible.writeNull();

            if (isReservedKeyword (String::CharPointerType (possibleIdentifier), tokenLength))
                return TokenType::keywordToken;
        }

        return TokenType::identifierToken;
    }

    virtual ValueTree parseNumber (Iterator& source) const
    {
        const Iterator original (source);

        if (parseFloatLiteral (source))
            return createToken ("", 0, 0, 0, TokenType::literalToken);

        source = original;
        if (parseHexLiteral (source))
            return createToken ("", 0, 0, 0, TokenType::literalToken);

        source = original;
        if (parseOctalLiteral (source))
            return createToken ("", 0, 0, 0, TokenType::literalToken);

        source = original;
        if (parseDecimalLiteral (source))
            return createToken ("", 0, 0, 0, TokenType::literalToken);

        source = original;
        return {};
    }

    virtual void skipComment (Iterator& source) noexcept
    {
        bool lastWasStar = false;

        for (;;)
        {
            auto c = source.nextChar();

            if (c == 0 || (c == '/' && lastWasStar))
                break;

            lastWasStar = c == '*';
        }
    }

    virtual void skipPreprocessorLine (Iterator& source) noexcept
    {
        bool lastWasBackslash = false;

        for (;;)
        {
            auto c = source.peekNextChar();

            if (c == '"')
            {
                skipQuotedString (source);
                continue;
            }

            if (c == '/')
            {
                Iterator next (source);
                next.skip();
                auto c2 = next.peekNextChar();

                if (c2 == '/' || c2 == '*')
                    return;
            }

            if (c == 0)
                break;

            if (c == '\n' || c == '\r')
            {
                source.skipToEndOfLine();

                if (lastWasBackslash)
                    skipPreprocessorLine (source);

                break;
            }

            lastWasBackslash = (c == '\\');
            source.skip();
        }
    }

    //==============================================================================
    virtual ValueTree readNextToken (CodeDocument::Iterator& source)
    {
        source.skipWhitespace();
        const auto firstChar = source.peekNextChar();

        switch (firstChar)
        {
            case 0: //EOF?
                return {}; //CPlusPlusCodeTokeniser::tokenType_punctuation;

            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case '.':
            {
                auto result = parseNumber (source);

                if (! result.isValid())
                {
                    source.skip();

                    if (firstChar == '.')
                        return {}; //CPlusPlusCodeTokeniser::tokenType_punctuation;
                }

                return result;
            }

            case ',':
            case ';':
            case ':':
            {
                auto e = createToken (String (firstChar), 1, 0, 0, TokenType::separatorToken);
                source.skip();
                return e;
            }

            case '(': case ')':
            case '{': case '}':
            case '[': case ']':
                source.skip();
                return {}; //CPlusPlusCodeTokeniser::tokenType_bracket;

            case '"':
            case '\'':
                skipQuotedString (source);
                return {}; //CPlusPlusCodeTokeniser::tokenType_string;

            case '+':
                source.skip();
                skipIfNextCharMatches (source, '+', '=');
                return {}; //CPlusPlusCodeTokeniser::tokenType_operator;

            case '-':
            {
                source.skip();
                auto result = parseNumber (source);

                if (result == CPlusPlusCodeTokeniser::tokenType_error)
                {
                    skipIfNextCharMatches (source, '-', '=');
                    return {}; //return CPlusPlusCodeTokeniser::tokenType_operator;
                }

                return result;
            }

            case '*': case '%':
            case '=': case '!':
                source.skip();
                skipIfNextCharMatches (source, '=');
                return {}; //return CPlusPlusCodeTokeniser::tokenType_operator;

            case '/':
            {
                source.skip();
                auto nextChar = source.peekNextChar();

                if (nextChar == '/')
                {
                    source.skipToEndOfLine();
                    return {}; //return CPlusPlusCodeTokeniser::tokenType_comment;
                }

                if (nextChar == '*')
                {
                    source.skip();
                    skipComment (source);
                    return {}; //return CPlusPlusCodeTokeniser::tokenType_comment;
                }

                if (nextChar == '=')
                    source.skip();

                return {}; //return CPlusPlusCodeTokeniser::tokenType_operator;
            }

            case '?':
            case '~':
                source.skip();
                return {}; //return CPlusPlusCodeTokeniser::tokenType_operator;

            case '<': case '>':
            case '|': case '&': case '^':
                source.skip();
                skipIfNextCharMatches (source, firstChar);
                skipIfNextCharMatches (source, '=');
                return {}; //return CPlusPlusCodeTokeniser::tokenType_operator;

            case '#':
                skipPreprocessorLine (source);
                return {}; // return CPlusPlusCodeTokeniser::tokenType_preprocessor;

            default:
                if (isIdentifierStart (firstChar))
                    return parseIdentifier (source);

                source.skip();
            break;
        }

        return {};
    }

    //==============================================================================
    /** */
    virtual ValueTree parse (const String& sourceCode) const
    {
        CodeDocument cd;
        cd.replaceAllContent (s);

        CodeDocument::Iterator source (cd);
        CodeDocument::Iterator lastIterator (source);
        const auto lineLength = lineText.length();

        auto root = createExpression ("root", 0, 0, 0);

        for (;;)
        {
            auto tokenType = readNextToken (source);
            if (! tokenType.isValid())
            {
                jassertfalse;
                return {};
            }

            auto tokenStart = lastIterator.getPosition();
            auto tokenEnd = source.getPosition();

            if (tokenEnd <= tokenStart)
                break;

            tokenEnd -= startPosition;

            if (tokenEnd > 0)
            {
                tokenStart -= startPosition;
                /*const auto start = jmax (0, tokenStart);

                root.appendChild (tokenType);

                addToken (newTokens,
                        lineText.substring (start, tokenEnd),
                        tokenEnd - start,
                        tokenType);*/

                if (tokenEnd >= lineLength)
                    break;
            }

            lastIterator = source;
        }

        return root;
    }

    /** */
    ValueTree parse (const File& sourceCodeFile) const
    {
        return parse (sourceCodeFile.loadFileAsString());
    }

    //==============================================================================
    /** */
    static ValueTree createToken (const String& value,
                                  int64 length,
                                  int64 line,
                                  int64 column,
                                  TokenType tokenType)
    {
        return createToken (value, length, line, column, toIdentifier (tokenType));
    }

    /** */
    static ValueTree createToken (const String& value,
                                  int64 length,
                                  int64 line,
                                  int64 column,
                                  const Identifier& tokenType)
    {
        return createExpression (value, length, line, column)
                .setProperty (tokenIds::typeId, tokenType.toString(), nullptr);
    }

    /** */
    static ValueTree createExpression (const String& value,
                                       int64 length,
                                       int64 line,
                                       int64 column)
    {
        return ValueTree (tokenIds::expressionId)
                .setProperty (tokenIds::valueId, value, nullptr)
                .setProperty (tokenIds::lengthId, length, nullptr)
                .setProperty (tokenIds::lineId, line, nullptr)
                .setProperty (tokenIds::columnId, column, nullptr);
    }

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericLexer)
};
