//==============================================================================
/** */
class RFC2422TimeParser final
{
public:
    /** */
    static Result parseString (Time& result, StringRef timeString);

    /** */
    static String toString (const Time& t);

private:
    //==============================================================================
    RFC2422TimeParser() = delete;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RFC2422TimeParser)
};

//==============================================================================
/** */
class ISO8601TimeParser final
{
public:
    /** */
    static Result parseString (Time& result, StringRef timeString);

    /** */
    static String toString (const Time& t, bool includeDividerCharacters = true);

private:
    //==============================================================================
    ISO8601TimeParser() = delete;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ISO8601TimeParser)
};
