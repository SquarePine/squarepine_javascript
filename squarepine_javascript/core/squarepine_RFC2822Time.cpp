//==============================================================================
static int parseFixedSizeIntAndSkip (String::CharPointerType& t, int numChars, char charToSkip) noexcept
{
    int n = 0;

    for (int i = numChars; --i >= 0;)
    {
        const auto digit = (int) (*t - '0');

        if (! isPositiveAndBelow (digit, 10))
            return -1;

        ++t;
        n = n * 10 + digit;
    }

    if (charToSkip != 0 && *t == (juce_wchar) charToSkip)
        ++t;

    return n;
}

//==============================================================================
Result RFC2422TimeParser::parseString (Time& result, StringRef timeString)
{
    ignoreUnused (timeString);
    result = Time();
    return Result::ok();
}

String RFC2422TimeParser::toString (const Time&)
{
    return {};
}

//==============================================================================
Result ISO8601TimeParser::parseString (Time& result, StringRef timeString)
{
    const auto failResult = Result::fail (String());

    auto t = timeString.text;
    auto year = parseFixedSizeIntAndSkip (t, 4, '-');

    if (year < 0)
        return failResult;

    auto month = parseFixedSizeIntAndSkip (t, 2, '-');

    if (month < 0)
        return failResult;

    auto day = parseFixedSizeIntAndSkip (t, 2, 0);

    if (day < 0)
        return failResult;

    int hours = 0, minutes = 0, milliseconds = 0;

    if (*t == 'T')
    {
        ++t;
        hours = parseFixedSizeIntAndSkip (t, 2, ':');

        if (hours < 0)
            return failResult;

        minutes = parseFixedSizeIntAndSkip (t, 2, ':');

        if (minutes < 0)
            return failResult;

        const auto seconds = parseFixedSizeIntAndSkip (t, 2, 0);

        if (seconds < 0)
            return failResult;

        if (*t == '.' || *t == ',')
        {
            ++t;
            milliseconds = parseFixedSizeIntAndSkip (t, 3, 0);

            if (milliseconds < 0)
                return failResult;
        }

        milliseconds += 1000 * seconds;
    }

    const auto nextChar = t.getAndAdvance();

    if (nextChar == '-' || nextChar == '+')
    {
        const auto offsetHours = parseFixedSizeIntAndSkip (t, 2, ':');

        if (offsetHours < 0)
            return failResult;

        const auto offsetMinutes = parseFixedSizeIntAndSkip (t, 2, 0);

        if (offsetMinutes < 0)
            return failResult;

        const auto offsetMs = (offsetHours * 60 + offsetMinutes) * 60 * 1000;
        milliseconds += nextChar == '-' ? offsetMs : -offsetMs; // NB: this seems backwards but is correct!
    }
    else if (nextChar != 0 && nextChar != 'Z')
    {
        return failResult;
    }

    result = Time (year, month - 1, day, hours, minutes, 0, milliseconds, false);
    return Result::ok();
}

String ISO8601TimeParser::toString (const Time& t, bool includeDividerCharacters)
{
    return String::formatted (includeDividerCharacters ? "%04d-%02d-%02dT%02d:%02d:%06.03f"
                                                       : "%04d%02d%02dT%02d%02d%06.03f",
                              t.getYear(),
                              t.getMonth() + 1,
                              t.getDayOfMonth(),
                              t.getHours(),
                              t.getMinutes(),
                              t.getSeconds() + t.getMilliseconds() / 1000.0)
            + t.getUTCOffsetString (includeDividerCharacters);
}
