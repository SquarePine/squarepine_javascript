//==============================================================================
#define SP_JS_CREATE_METHOD(methodName) \
    setMethod (JUCE_STRINGIFY (methodName), methodName);

#define SP_JS_IDENTIFY_CLASS(className) \
    static Identifier getClassName() { static const Identifier i (className); return i; }

//==============================================================================
inline bool isNumber (const String& s)
{
    CodeDocument cd;
    cd.replaceAllContent (s);

    CodeDocument::Iterator iter (cd);
    return CppTokeniserFunctions::parseNumber (iter) != CPlusPlusCodeTokeniser::tokenType_error;
}

//==============================================================================
/** A simple helper class to allow debugging and testing of
    copied and pasted Javascript code from official examples online.
*/
struct ConsoleClass final : public JavascriptClass
{
    ConsoleClass()
    {
        setMethod ("log", logMethod);
        setMethod ("print", logMethod);
    }

    SP_JS_IDENTIFY_CLASS ("console")

    static var logMethod (Args a)
    {
        MemoryOutputStream mo (1024);

        for (int i = 0; i < a.numArguments; ++i)
            mo << a.arguments[i].toString() << newLine;

        const auto result = mo.toString();
        Logger::writeToLog (result);
        return result;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleClass)
};

//==============================================================================
/** Allows fetching an assortment of information about the JUCE framework
    we compiled with from within a Javascript script.
*/
struct JUCEClass final : public JavascriptClass
{
    JUCEClass()
    {
        setMethod ("getOperatingSystemType",    [](Args) { return static_cast<int> (SystemStats::getOperatingSystemType()); });
        setMethod ("getJUCEVersionMajor",       [](Args) { return JUCE_MAJOR_VERSION; });
        setMethod ("getJUCEVersionMinor",       [](Args) { return JUCE_MINOR_VERSION; });
        setMethod ("getJUCEVersionBuildNumber", [](Args) { return JUCE_BUILDNUMBER; });
        setMethod ("getCompilationDate",        [](Args) { return String (__DATE__ __TIME__); });

        #define JUCE_CLASS_METHODS(X) \
            X (getJUCEVersion)              X (getOperatingSystemName)  X (isOperatingSystem64Bit)  X (getLogonName) \
            X (getFullUserName)             X (getComputerName)         X (getUserLanguage)         X (getUserRegion) \
            X (getDisplayLanguage)          X (getDeviceDescription)    X (getDeviceManufacturer)   X (getNumCpus) \
            X (getNumPhysicalCpus)          X (getCpuSpeedInMegahertz)  X (getCpuVendor)            X (getCpuModel) \
            X (getMemorySizeInMegabytes)    X (getPageSize)             X (getStackBacktrace)       X (isRunningInAppExtensionSandbox)

        #define CREATE_JUCE_CLASS_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), [](Args) { return SystemStats:: ## methodName ## (); });

        JUCE_CLASS_METHODS (CREATE_JUCE_CLASS_METHOD)

        #undef OBJECT_CLASS_METHODS
    }

    SP_JS_IDENTIFY_CLASS ("JUCE")

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JUCEClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object
*/
struct ObjectClass final : public JavascriptClass
{
    ObjectClass()
    {
        #define OBJECT_CLASS_METHODS(X) \
            X (assign)                      X (create)                      X (defineProperties)    X (defineProperty) \
            X (dump)                        X (entries)                     X (freeze)              X (fromEntires) \
            X (getOwnPropertyDescriptor)    X (getOwnPropertyDescriptors)   X (getOwnPropertyNames) X (getOwnPropertySymbols) \
            X (getPrototypeOf)              X (is)                          X (isExtensible)        X (isFrozen) \
            X (isSealed)                    X (keys)                        X (preventExtensions)   X (seal) \
            X (setProtypeOf)                X (values)

        OBJECT_CLASS_METHODS (SP_JS_CREATE_METHOD)

        #undef OBJECT_CLASS_METHODS

        setMethod ("clone",  cloneFn);
    }

    SP_JS_IDENTIFY_CLASS ("Object")

    //NB: These are non-standard methods - just helpers...
    static var dump (Args a)                        { DBG (JSON::toString (a.thisObject)); ignoreUnused (a); return var::undefined(); }
    static var cloneFn (Args a)                     { return a.thisObject.clone(); }

    static var assign (Args a)                      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var create (Args a)                      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var defineProperties (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var defineProperty (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var entries (Args a)                     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var freeze (Args a)                      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var fromEntires (Args a)                 { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getOwnPropertyDescriptor (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getOwnPropertyDescriptors (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getOwnPropertyNames (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getOwnPropertySymbols (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getPrototypeOf (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var is (Args a)                          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var isExtensible (Args a)                { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var isFrozen (Args a)                    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var isSealed (Args a)                    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var keys (Args a)                        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var preventExtensions (Args a)           { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var seal (Args a)                        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setProtypeOf (Args a)                { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var values (Args a)                      { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ObjectClass)
};

//==============================================================================
/*
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array
*/
struct ArrayClass final : public JavascriptClass
{
    ArrayClass()
    {
        #define ARRAY_CLASS_METHODS(X) \
            X (concat)      X (contains)    X (copyWithin)      X (entries) \
            X (every)       X (fill)        X (filter)          X (find) \
            X (findIndex)   X (forEach)     X (from)            X (includes) \
            X (indexOf)     X (isArray)     X (join)            X (keys) \
            X (lastIndexOf) X (map)         X (observe)         X (of) \
            X (pop)         X (push)        X (reduce)          X (reduceRight) \
            X (reverse)     X (shift)       X (slice)           X (some) \
            X (sort)        X (splice)      X (toLocaleString)  X (toSource) \
            X (toString)    X (unshift)     X (values)

        ARRAY_CLASS_METHODS (SP_JS_CREATE_METHOD)

        #undef ARRAY_CLASS_METHODS
    }

    SP_JS_IDENTIFY_CLASS ("Array")
    static Array<var>* getThisArray (Args a) { return a.thisObject.getArray(); }

    static var entries (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var from (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var isArray (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var keys (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var map (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var observe (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var of (Args a)              { return concat (a); }
    static var slice (Args a)           { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var some (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toLocaleString (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toSource (Args a)        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toString (Args a)        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var unshift (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var values (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var reduceRight (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO

    static var reduce (Args a)
    {
        if (auto* sourceArray = getThisArray (a))
        {
            if (a.numArguments > 0)
            {
                auto sortMethod = get (a, 0);
                if (sortMethod.isMethod())
                {
                    jassertfalse; //TODO!
                    //sourceArray->sort();
                }
                else
                {
                    jassertfalse; //Bogus sort function!
                }
            }
            else
            {
                sourceArray->sort();
            }
        }

        return var::undefined();
    }

    static var sort (Args a)
    {
        if (auto* sourceArray = getThisArray (a))
        {
            if (a.numArguments > 0)
            {
                auto sortMethod = get (a, 0);
                if (sortMethod.isMethod())
                {
                    jassertfalse; //TODO!
                    //sourceArray->sort();
                }
                else
                {
                    jassertfalse; //Bogus sort function!
                }
            }
            else
            {
                sourceArray->sort();
            }
        }

        return var::undefined();
    }

    static var concat (Args a)
    {
        Array<var> result;

        if (auto* sourceArray = getThisArray (a))
        {
            result = *sourceArray;

            for (int i = 0; i < a.numArguments; ++i)
            {
                auto target = get (a, 0);

                if (auto* targetArray = target.getArray())
                {
                    result.addArray (*targetArray);
                }
                else if (target.isInt() || target.isInt64() || target.isBool()
                        || target.isDouble() || target.isString())
                {
                    result.add (target);
                }
            }
        }

        result.minimiseStorageOverheads();
        return result;
    }

    static var contains (Args a)
    {
        if (auto* array = getThisArray (a))
            return array->contains (get (a, 0));

        return false;
    }

    static const var& copyWithin (Args a)
    {
        if (auto* sourceArray = getThisArray (a))
        {
            const auto length = sourceArray->size();
            const auto startIndex = getInt (a, 0);

            if (startIndex < 0 && std::abs (startIndex) > length)
                return a.thisObject;

            jassertfalse; // @todo
            return a.thisObject;
        }

        static auto ref = var::undefined();
        return ref;
    }

    static var every (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);

            if (auto func = target.getNativeFunction())
            {
                for (int i = 0; i < array->size(); ++i)
                {
                    const var arguments[] =
                    {
                        array->getUnchecked (i),
                        i,
                        a.thisObject
                    };

                    if (static_cast<bool> (func ({ target, arguments, 3 })))
                        return true;
                }
            }
            else
            {
                if (auto* p = dynamic_cast<FunctionObject*> (target.getDynamicObject()))
                {
                    for (int i = 0; i < array->size(); ++i)
                    {
                        const var arguments[] =
                        {
                            array->getUnchecked (i),
                            i,
                            a.thisObject
                        };

                        if (static_cast<bool> (p->invoke ({ nullptr, nullptr, p }, { target, arguments, 3 })))
                            return true;
                    }
                }
            }
        }

        return false;
    }

    static var fill (Args a)
    {
        if (auto* sourceArray = getThisArray (a))
        {
            const auto value = get (a, 0);
            const auto length = sourceArray->size();

            auto start  = a.numArguments >= 1 ? getInt (a, 1) : 0;
            auto end    = a.numArguments >= 2 ? getInt (a, 2) : length;

            if (start < 0)
                start = length + start;

            if (end < 0)
                end = length + end;

            if (start > end)
                std::swap (start, end);

            for (int i = start; i < end; ++i)
                sourceArray->getReference (i) = value;

            return *sourceArray;
        }

        return Array<var>(); //NB: Purposely constructing an empty array of vars here
    }

    static var filter (Args a)
    {
        Array<var> resultArray;

        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);

            if (auto func = target.getNativeFunction())
            {
                for (int i = 0; i < array->size(); ++i)
                {
                    const var arguments[] =
                    {
                        array->getUnchecked (i),
                        i,
                        a.thisObject
                    };

                    auto result = func ({ target, arguments, 3 });
                    if (! result.isBool())
                    {
                        jassertfalse; // Hackerman strikes again.
                        return var::undefined();
                    }

                    if (static_cast<bool> (result))
                        resultArray.add (array->getUnchecked (i));
                }
            }
        }

        resultArray.minimiseStorageOverheads();
        return resultArray;
    }

    static var find (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);

            if (auto func = target.getNativeFunction())
            {
                for (int i = 0; i < array->size(); ++i)
                {
                    const var arguments[] =
                    {
                        array->getUnchecked (i),
                        i,
                        a.thisObject
                    };

                    auto result = func ({ target, arguments, 3 });
                    if (result.isBool())
                    {
                        if (static_cast<bool> (result))
                            return array->getUnchecked (i);
                    }
                    else
                    {
                        jassertfalse; // Hackerman strikes again.
                        break;
                    }

                }
            }
        }

        return var::undefined();
    }

    static var findIndex (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);

            if (auto func = target.getNativeFunction())
            {
                for (int i = 0; i < array->size(); ++i)
                {
                    const var arguments[] =
                    {
                        array->getUnchecked (i),
                        i,
                        a.thisObject
                    };

                    auto result = func ({ target, arguments, 3 });
                    if (! result.isBool())
                    {
                        jassertfalse; // Hackerman strikes again.
                        return var::undefined();
                    }

                    if (static_cast<bool> (result))
                        return i;
                }
            }
        }

        return -1;
    }

    static var forEach (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);

            if (auto func = target.getNativeFunction())
            {
                for (int i = 0; i < array->size(); ++i)
                {
                    const var arguments[] =
                    {
                        array->getUnchecked (i),
                        i,
                        a.thisObject
                    };

                    func ({ target, arguments, 3 });
                }
            }
        }

        return var::undefined();
    }

    static var includes (Args a)
    {
        if (a.numArguments <= 0)
        {
            jassertfalse;
            return false;
        }

        if (auto* array = getThisArray (a))
            return array->contains (get (a, 0));

        return false;
    }

    static var indexOf (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);

            for (int i = (a.numArguments > 1 ? getInt (a, 1) : 0); i < array->size(); ++i)
                if (array->getReference (i) == target)
                    return i;
        }

        return -1;
    }

    static var lastIndexOf (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto target = get (a, 0);
            const auto startIndex = (a.numArguments > 1 ? getInt (a, 1) : 0);

            for (int i = array->size(); --i >= startIndex;)
                if (array->getReference (i) == target)
                    return i;
        }

        return -1;
    }

    /** This method is a special case where all elements of the array are joined
        into a string and returns this string, regardless of the element's type.

        @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/join
        @see https://www.w3schools.com/jsref/jsref_join.asp
    */
    static var join (Args a)
    {
        StringArray strings;

        if (auto* array = getThisArray (a))
            for (const auto& v : *array)
                strings.add (v.toString());

        return strings.joinIntoString (getString (a, 0));
    }

    static var pop (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            for (int i = 0; i < a.numArguments; ++i)
                array->add (a.arguments[i]);

            array->removeLast();
            return array->size();
        }

        return var::undefined();
    }

    static var push (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            for (int i = 0; i < a.numArguments; ++i)
                array->add (a.arguments[i]);

            return array->size();
        }

        return var::undefined();
    }

    static var remove (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            array->removeAllInstancesOf (get (a, 0));
            return *array;
        }

        return var::undefined();
    }

    static var reverse (Args a)
    {
        if (auto* sourceArray = getThisArray (a))
            std::reverse (sourceArray->begin(), sourceArray->end());

        return var::undefined();
    }

    static var shift (Args a)
    {
        if (auto* array = getThisArray (a))
            if (! array->isEmpty())
                return array->removeAndReturn (0);

        return var::undefined();
    }

    static var splice (Args a)
    {
        if (auto* array = getThisArray (a))
        {
            auto arraySize = array->size();
            int start = get (a, 0);

            if (start < 0)
                start = jmax (0, arraySize + start);
            else if (start > arraySize)
                start = arraySize;

            const int num = a.numArguments > 1 ? jlimit (0, arraySize - start, getInt (a, 1))
                                                : arraySize - start;

            Array<var> itemsRemoved;
            itemsRemoved.ensureStorageAllocated (num);

            for (int i = 0; i < num; ++i)
                itemsRemoved.add (array->getReference (start + i));

            array->removeRange (start, num);

            for (int i = 2; i < a.numArguments; ++i)
                array->insert (start++, get (a, i));

            return itemsRemoved;
        }

        return var::undefined();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrayClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String
*/
struct StringClass final : public JavascriptClass
{
    StringClass()
    {
        #define STRING_CLASS_METHODS(X) \
            X (charAt)      X (charCodeAt)      X (codePointAt)     X (concat) \
            X (endsWith)    X (fromCharCode)    X (fromCodePoint)   X (includes) \
            X (indexOf)     X (lastIndexOf)     X (localeCompare)   X (match) \
            X (normalize)   X (padEnd)          X (padStart)        X (quote) \
            X (raw)         X (repeat)          X (replace)         X (search) \
            X (slice)       X (split)           X (startsWith)      X (substr) \
            X (substring)   X (toLocaleLowerCase) X (toLocaleUpperCase) X (toLowerCase) \
            X (toString)    X (toUpperCase)     X (trim)            X (trimLeft) \
            X (trimRight)   X (valueOf)

        STRING_CLASS_METHODS (SP_JS_CREATE_METHOD)

        #undef STRING_CLASS_METHODS
    }

    SP_JS_IDENTIFY_CLASS ("String")

    static String getThisString (Args a)    { return a.thisObject.toString(); }

    static var codePointAt (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var fromCodePoint (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var normalize (Args a)           { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var padEnd (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var padStart (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var raw (Args a)                 { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var search (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var slice (Args a)               { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var substr (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var valueOf (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO

    static var charAt (Args a)              { int p = getInt (a, 0); return getThisString (a).substring (p, p + 1); }
    static var charCodeAt (Args a)          { return (int) getThisString (a)[getInt (a, 0)]; }
    static var concat (Args a)              { return getThisString (a) + getString (a, 0); }
    static var endsWith (Args a)            { return getThisString (a).endsWith (getString (a, 0)); }
    static var fromCharCode (Args a)        { return String::charToString (static_cast<juce_wchar> (getInt (a, 0))); }
    static var includes (Args a)            { return getThisString (a).substring (getInt (a, 1)).contains (getString (a, 0)); }
    static var indexOf (Args a)             { return getThisString (a).indexOf (getString (a, 0)); }
    static var lastIndexOf (Args a)         { return getThisString (a).lastIndexOf (getString (a, 0)); }
    static var localeCompare (Args a)       { return getThisString (a).compare (getString (a, 0)); }
    static var match (Args a)               { std::regex r (getString (a, 0).trim().toStdString()); return std::regex_match (getThisString (a).toStdString(), r); }
    static var quote (Args a)               { return getThisString (a).quoted(); }
    static var repeat (Args a)              { return String::repeatedString (getString (a, 0), getInt (a, 1)); }
    static var replace (Args a)             { return getThisString (a).replace (getString (a, 0), getString (a, 1)); }
    static var startsWith (Args a)          { return getThisString (a).startsWith (getString (a, 0)); }
    static var substring (Args a)           { return getThisString (a).substring (getInt (a, 0), getInt (a, 1)); }
    static var toLocaleLowerCase (Args a)   { return getThisString (a).toLowerCase(); }
    static var toLocaleUpperCase (Args a)   { return getThisString (a).toUpperCase(); }
    static var toLowerCase (Args a)         { return getThisString (a).toLowerCase(); }
    static var toString (Args a)            { return getThisString (a); }
    static var toUpperCase (Args a)         { return getThisString (a).toUpperCase(); }
    static var trim (Args a)                { return getThisString (a).trim(); }
    static var trimLeft (Args a)            { return getThisString (a).trimStart(); }
    static var trimRight (Args a)           { return getThisString (a).trimEnd(); }

    static var split (Args a)
    {
        auto str = a.thisObject.toString();
        auto sep = getString (a, 0);
        StringArray strings;

        if (sep.isNotEmpty())
            strings.addTokens (str, sep.substring (0, 1), {});
        else // special-case for empty separator: split all chars separately
            for (auto pos = str.getCharPointer(); ! pos.isEmpty(); ++pos)
                strings.add (String::charToString (*pos));

        var array;

        for (const auto& s : strings)
            array.append (s);

        return array;
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Date
*/
struct DateClass final : public JavascriptClass
{
    DateClass (const Time& v = Time::getCurrentTime()) :
        value (v)
    {
        #define DATE_CLASS_METHODS(X) \
            X (UTC) X (now) X (parse)

        DATE_CLASS_METHODS (SP_JS_CREATE_METHOD)

        #undef DATE_CLASS_METHODS
    }

    SP_JS_IDENTIFY_CLASS ("Date")

    //statics...
    static var UTC (Args)                   { return Time::getCurrentTime().toMilliseconds(); }
    static var now (Args a)                 { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var parse (Args a)               { return Time::fromISO8601 (getString (a, 0)).toMilliseconds(); }

    //methods...
    static var getDate (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getFullYear (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getHours (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getMilliseconds (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getMinutes (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getMonth (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getSeconds (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getTime (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getTimezoneOffset (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCDate (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCDay (Args a)           { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCFullYear (Args a)      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCHours (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCMilliseconds (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCMinutes (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCMonth (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUTCSeconds (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getYear (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setDate (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setFullYear (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setHours (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setMilliseconds (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setMinutes (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setMonth (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setSeconds (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setTime (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCDate (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCFullYear (Args a)      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCHours (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCMilliseconds (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCMinutes (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCMonth (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUTCSeconds (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setYear (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toDateString (Args a)        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toGMTString (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO

    static var toISOString (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toJSON (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toLocaleDateString (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toLocaleFormat (Args a)      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toLocaleString (Args a)      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toLocaleTimeString (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toSource (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toString (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toTimeString (Args a)        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toUTCString (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var valueOf (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO

    static DateClass* construct (const Array<var>& vars)
    {
        if (vars.size() == 1)
        {
            const auto& v = vars.getReference (0);

            if (v.isString())
            {
                const auto s = v.toString();
                Time t;

                if (RFC2422TimeParser::parseString (t, s).wasOk()
                    || ISO8601TimeParser::parseString (t, s).wasOk())
                    return new DateClass (t);
            }

            return nullptr;
        }
        else if (vars.size() > 1)
        {
        }

        return new DateClass();
    }

    bool areSameValue (const var& v) override
    {
        if (auto* other = dynamic_cast<DateClass*> (v.getDynamicObject()))
            return value == other->value;

        return false;
    }

private:
    Time value { Time::getCurrentTime() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DateClass)
};

//==============================================================================
struct MathClass final : public JavascriptClass
{
    MathClass() :
        distribution (0.0, 1.0)
    {
        #define MATH_CLASS_METHODS(X) \
            X (abs) X (acos) X (acosh) X (asin) X (asinh) X (atan) X (atanh) X (atan2) \
            X (cbrt) X (ceil) X (cos) X (cosh) X (exp) X (expm1) X (floor) X (fround) X (hypot) \
            X (imul) X (log) X (log1p) X (log2) X (log10) X (max) X (min) X (pow) \
            X (randInt) X (round) X (sign) X (sin) X (sinh) X (sqrt) X (clz32) \
            X (tan) X (tanh) X (trunc) X (range) X (sqrt) X (toDegrees) X (toRadians)

        #define CREATE_MATH_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), Math_ ## methodName);

        MATH_CLASS_METHODS (CREATE_MATH_METHOD)

        #undef CREATE_MATH_METHOD
        #undef MATH_CLASS_METHODS

        const auto log2 = std::log (2.0);
        const auto log10 = std::log (2.0);
        const auto loge = std::log (MathConstants<double>::euler);

        setProperty ("PI",      MathConstants<double>::pi);
        setProperty ("E",       MathConstants<double>::euler);
        setProperty ("SQRT2",   MathConstants<double>::sqrt2);
        setProperty ("SQRT1_2", std::sqrt (0.5));
        setProperty ("LN2",     log2);
        setProperty ("LN10",    log10);
        setProperty ("LOG2E",   loge / log2);
        setProperty ("LOG10E",  loge / log10);

        randomEngine.seed (randomDevice());

        setMethod ("random", [&] (Args) { return distribution (randomEngine); });
    }

    SP_JS_IDENTIFY_CLASS ("Math")

    std::random_device randomDevice;
    std::default_random_engine randomEngine;
    std::uniform_real_distribution<double> distribution;

    static var Math_abs     (Args a) { return isInt (a, 0) ? var (std::abs   (getInt (a, 0))) : var (std::abs   (getDouble (a, 0))); }
    static var Math_acos    (Args a) { return std::acos  (getDouble (a, 0)); }
    static var Math_asin    (Args a) { return std::asin  (getDouble (a, 0)); }
    static var Math_atan    (Args a) { return std::atan  (getDouble (a, 0)); }
    static var Math_atan2   (Args a) { return std::atan2 (getDouble (a, 0), getDouble (a, 1)); }
    static var Math_cbrt    (Args a) { return std::cbrt  (getDouble (a, 0)); }
    static var Math_ceil    (Args a) { return std::ceil  (getDouble (a, 0)); }
    static var Math_cos     (Args a) { return std::cos   (getDouble (a, 0)); }
    static var Math_cosh    (Args a) { return std::cosh  (getDouble (a, 0)); }
    static var Math_exp     (Args a) { return std::exp   (getDouble (a, 0)); }
    static var Math_expm1   (Args a) { return std::expm1 (getDouble (a, 0)); }
    static var Math_floor   (Args a) { return std::floor (getDouble (a, 0)); }
    static var Math_fround  (Args a) { return Math_round (a); }
    static var Math_imul    (Args a) { return getInt (a, 0) * getInt (a, 1); }
    static var Math_log     (Args a) { return std::log   (getDouble (a, 0)); }
    static var Math_log1p   (Args a) { return std::log1p (getDouble (a, 0)); }
    static var Math_log2    (Args a) { return std::log2  (getDouble (a, 0)); }
    static var Math_log10   (Args a) { return std::log10 (getDouble (a, 0)); }
    static var Math_max     (Args a) { return (isInt (a, 0) && isInt (a, 1)) ? var (jmax (getInt (a, 0), getInt (a, 1))) : var (jmax (getDouble (a, 0), getDouble (a, 1))); }
    static var Math_min     (Args a) { return (isInt (a, 0) && isInt (a, 1)) ? var (jmin (getInt (a, 0), getInt (a, 1))) : var (jmin (getDouble (a, 0), getDouble (a, 1))); }
    static var Math_pow     (Args a) { return std::pow   (getDouble (a, 0), getDouble (a, 1)); }
    static var Math_randInt (Args a) { return a.numArguments < 2 ? var::undefined() : Random::getSystemRandom().nextInt (Range<int> (getInt (a, 0), getInt (a, 1))); }
    static var Math_round   (Args a) { return isInt (a, 0) ? var (roundToInt (getInt (a, 0))) : var (roundToInt (getDouble (a, 0))); }
    static var Math_sign    (Args a) { return isInt (a, 0) ? var (sign (getInt (a, 0))) : var (sign (getDouble (a, 0))); }
    static var Math_sin     (Args a) { return std::sin   (getDouble (a, 0)); }
    static var Math_sinh    (Args a) { return std::sinh  (getDouble (a, 0)); }
    static var Math_sqrt    (Args a) { return std::sqrt  (getDouble (a, 0)); }
    static var Math_tan     (Args a) { return std::tan   (getDouble (a, 0)); }
    static var Math_tanh    (Args a) { return std::tanh  (getDouble (a, 0)); }
    static var Math_trunc   (Args a) { return std::trunc (getDouble (a, 0)); }
    static var Math_asinh   (Args a) { return std::asinh (getDouble (a, 0)); }
    static var Math_acosh   (Args a) { return std::acosh (getDouble (a, 0)); }
    static var Math_atanh   (Args a) { return std::atanh (getDouble (a, 0)); }

    //NB: These are non-standard.
    static var Math_range     (Args a) { return isInt (a, 0) ? var (jlimit (getInt (a, 1), getInt (a, 2), getInt (a, 0))) : var (jlimit (getDouble (a, 1), getDouble (a, 2), getDouble (a, 0))); }
    static var Math_sqr       (Args a) { return square           (getDouble (a, 0)); }
    static var Math_toDegrees (Args a) { return radiansToDegrees (getDouble (a, 0)); }
    static var Math_toRadians (Args a) { return degreesToRadians (getDouble (a, 0)); }

    /** @returns the square root of the sum of squares of the given arguments.
        If at least one of the arguments cannot be converted to a number, NaN is returned.

        @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/hypot
    */
    static var Math_hypot (Args a)
    {
        if (a.numArguments <= 0)
            return 0;

        double value = 0.0;

        for (int i = 0; i < a.numArguments; ++i)
        {
            const auto& v = a.arguments[i];
            if (! v.isInt() || ! v.isInt64() || ! v.isDouble())
                return std::numeric_limits<double>::quiet_NaN();

            value += square (static_cast<double> (a.arguments[i]));
        }

        return std::sqrt (value);
    }

    static int countLeadingZeros (int x) noexcept
    {
        if (x <= 0)
            return 0;

        //do the smearing
        x |= x >> 1; 
        x |= x >> 2;
        x |= x >> 4;
        x |= x >> 8;
        x |= x >> 16;

        //count the ones
        x -= x >> 1 & 0x55555555;
        x = (x >> 2 & 0x33333333) + (x & 0x33333333);
        x = (x >> 4) + x & 0x0f0f0f0f;
        x += x >> 8;
        x += x >> 16;

        return (sizeof (int) * 8) - (x & 0x0000003f); //subtract # of 1s from 32
    }

    /** @returns the number of leading zero bits in the 32-bit binary representation of a number.
        @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Math/clz32
    */
    static var Math_clz32 (Args a)
    {
        if (a.numArguments == 1)
        {
            const auto& v = a.arguments[0];

            if (v.isInt())
                return countLeadingZeros (static_cast<int> (v));

            return countLeadingZeros (static_cast<int> (static_cast<int64> (v)));
        }

        jassertfalse; // wtf?
        return var::undefined();
    }

    template<typename Type>
    static constexpr Type sign (Type n) noexcept 
    {
        return n > 0 ? (Type) 1 : (n < 0 ? (Type) -1 : 0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MathClass)
};

//==============================================================================
struct JSONClass final : public JavascriptClass
{
    JSONClass()
    {
        setMethod ("parse", parse);
        setMethod ("stringify", stringify);
    }

    SP_JS_IDENTIFY_CLASS ("JSON")

    static var parse (Args a)       { return JSON::parse (get (a, 0)); }
    static var stringify (Args a)   { return JSON::toString (get (a, 0)); }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JSONClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Symbol
*/
struct SymbolClass final : public JavascriptClass
{
    SymbolClass()
    {
    }

    SP_JS_IDENTIFY_CLASS ("SymbolClass")

    static var Symbol_for (Args a)          { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Symbol_keyFor (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO

    //TODO + should be methods for the instance of the class.
    static var asyncIterator (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var hasInstance (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var isConcatSpreadable (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var match (Args a)               { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var matchAll (Args a)            { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var replace (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var search (Args a)              { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var split (Args a)               { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var species (Args a)             { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toPrimitive (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toStringTag (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var unscopables (Args a)         { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SymbolClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/RegExp
*/
struct RegExpClass final : public JavascriptClass
{
    RegExpClass()
    {
    }

    SP_JS_IDENTIFY_CLASS ("RegExp")

    static var compile (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var exec (Args a)        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var test (Args a)        { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toSource (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var toString (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RegExpClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Numbers_and_dates
    console.log (Number.EPSILON);
    console.log (Number.NaN);
    console.log (Number.NEGATIVE_INFINITY);
    console.log (Number.POSITIVE_INFINITY);
    console.log (Number.MIN_SAFE_INTEGER);
    console.log (Number.MAX_SAFE_INTEGER);
    console.log (Number.MIN_VALUE);
    console.log (Number.MAX_VALUE);
*/
struct NumberClass final : public JavascriptClass
{
    NumberClass (double initialValue = 0.0) :
        value (initialValue)
    {
        setMethod ("parseInt",              parseInt);
        setMethod ("parseFloat",            parseFloat);
        setMethod ("isNaN",                 isNaN);
        setMethod ("isFinite",              isFinite);
        setMethod ("isInteger",             [] (Args a) { return isInt (a, 0); });
        setMethod ("isSafeInteger",         [] (Args a) { return ! std::isnan (getDouble (a, 0)); });

        setProperty ("EPSILON",             std::numeric_limits<double>::epsilon());
        setProperty ("NaN",                 std::numeric_limits<double>::quiet_NaN());
        setProperty ("NEGATIVE_INFINITY",   -std::numeric_limits<double>::infinity());
        setProperty ("POSITIVE_INFINITY",   std::numeric_limits<double>::infinity());
        setProperty ("MIN_SAFE_INTEGER",    std::numeric_limits<int64>::min());
        setProperty ("MAX_SAFE_INTEGER",    std::numeric_limits<int64>::max());
        setProperty ("MIN_VALUE",           std::numeric_limits<double>::min());
        setProperty ("MAX_VALUE",           std::numeric_limits<double>::max());

        setMethod ("toString",             [&] (Args) { return var (value).toString(); });
    }

    SP_JS_IDENTIFY_CLASS ("Number")

    static var parseInt (Args a)
    {
        auto s = getString (a, 0).trim();

        return s[0] == '0' ? (s[1] == 'x' ? s.substring (2).getHexValue64() : getOctalValue (s))
                           : s.getLargeIntValue();
    }

    static var parseFloat (Args a)      { return getDouble (a, 0); }
    static var isNaN (Args a)           { return std::isnan (getDouble (a, 0)); }
    static var isFinite (Args a)        { return std::isfinite (getDouble (a, 0)); }

    void writeAsJSON (OutputStream& out, int, bool, int) override { out << value; }

    static NumberClass* construct (const Array<var>& vars)
    {
        auto* nc = new NumberClass();

        if (vars.size() >= 1)
        {
            const auto& v = vars.getReference (0);

            if (v.isString())
            {
                const auto s = v.toString();
                if (isNumber (s))
                    nc->value = s.getDoubleValue();
                else
                    nc->value = std::numeric_limits<double>::quiet_NaN();
            }
            else if (v.isInt())     { nc->value = (double) static_cast<int> (v); }
            else if (v.isInt64())   { nc->value = (double) static_cast<int64> (v); }
            else if (v.isDouble())  { nc->value = static_cast<double> (v); }
            else                    { nc->value = std::numeric_limits<double>::quiet_NaN(); }
        }

        return nc;
    }

    bool areSameValue (const var& v) override
    {
        if (v.isInt())          { return value == (double) static_cast<int> (v); }
        else if (v.isInt64())   { return value == (double) static_cast<int64> (v); }
        else if (v.isDouble())  { return value == static_cast<double> (v); }
        else if (v.isString())
        {
            const auto s = v.toString();
            if (isNumber (s))
                return value == s.getDoubleValue();

            return false;
        }

        if (auto* other = dynamic_cast<NumberClass*> (v.getDynamicObject()))
            return value == other->value;

        return false;
    }

private:
    double value = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NumberClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
*/
struct BooleanClass final : public JavascriptClass
{
    BooleanClass (bool initialValue = false) :
        value (initialValue)
    {
        setMethod ("toString",  [&] (Args) { return toString(); });
        setMethod ("valueOf",   [&] (Args) { return toString(); });
    }

    static BooleanClass* construct (const Array<var>& vars)
    {
        auto* bc = new BooleanClass();

        for (const auto& v : vars)
        {
            if (v.isBool())         { bc->value |= static_cast<bool> (v); }
            else if (v.isInt())     { bc->value |= static_cast<int> (v) != 0; }
            else if (v.isInt64())   { bc->value |= static_cast<int64> (v) != 0; }
            else if (v.isDouble())  { bc->value |= static_cast<double> (v) != 0.0; }
            else if (v.isString())  { bc->value |= v.toString().isNotEmpty(); }
            else                    { bc->value = true; break; }
        }

        return bc;
    }

    SP_JS_IDENTIFY_CLASS ("Boolean")

    String toString() const { return value ? "true" : "false"; }

    void writeAsJSON (OutputStream& out, int, bool, int) override { out << toString(); }

    bool areSameValue (const var& v) override
    {
        if (v.isBool())         { return value == static_cast<bool> (v); }
        else if (v.isInt())     { return value == (static_cast<int> (v) != 0); }
        else if (v.isInt64())   { return value == (static_cast<int64> (v) != 0); }
        else if (v.isDouble())  { return value == (static_cast<double> (v) != 0.0); }
        else if (v.isString())  { return value == v.toString().equalsIgnoreCase ("true"); }

        if (auto* other = dynamic_cast<BooleanClass*> (v.getDynamicObject()))
            return value == other->value;

        return false;
    }

private:
    bool value = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BooleanClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/BigInt
*/
struct BigIntClass final : public JavascriptClass
{
    BigIntClass()
    {
    }

    SP_JS_IDENTIFY_CLASS ("BigInt")

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BigIntClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Reflect
*/
struct ReflectClass final : public JavascriptClass
{
    ReflectClass()
    {
    }

    SP_JS_IDENTIFY_CLASS ("Reflect")

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReflectClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Proxy
*/
struct ProxyClass final : public JavascriptClass
{
    ProxyClass()
    {
    }

    SP_JS_IDENTIFY_CLASS ("Proxy")

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProxyClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Atomics
*/
struct AtomicsClass final : public JavascriptClass
{
    AtomicsClass()
    {
    }

    SP_JS_IDENTIFY_CLASS ("Atomics")

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AtomicsClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Boolean
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Map
*/
struct MapClass final : public JavascriptClass
{
    MapClass()
    {
        #define MAP_CLASS_METHODS(X) \
            X (delete)  X (clear)   X (entries) X (forEach) X (get) \
            X (has)     X (keys)    X (set)     X (values)

        #define CREATE_MAP_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), Map_ ## methodName);

        MAP_CLASS_METHODS (CREATE_MAP_METHOD)

        #undef MAP_CLASS_METHODS
        #undef CREATE_MAP_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("Map")

    static var Map_delete (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_clear (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_entries (Args a) { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_forEach (Args a) { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_get (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_has (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_keys (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_set (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Map_values (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO

    //operator[]
    static var Map_operatorBrackets (Args a)
    {
        ignoreUnused (a); jassertfalse; return var();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MapClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Set
*/
struct SetClass final : public JavascriptClass
{
    SetClass()
    {
        #define SET_CLASS_METHODS(X) \
            X (delete) X (add) X (clear) X (entries) X (forEach) X (has) X (values)

        #define CREATE_SET_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), Set_ ## methodName);

        SET_CLASS_METHODS (CREATE_SET_METHOD)

        #undef SET_CLASS_METHODS
        #undef CREATE_SET_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("Set")

    static var Set_delete (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Set_add (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Set_clear (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Set_entries (Args a) { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Set_forEach (Args a) { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Set_has (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var Set_values (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO

    //operator[]
    static var Set_operatorBrackets (Args a)
    {
        ignoreUnused (a); jassertfalse; return var();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SetClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WeakMap
*/
struct WeakMapClass final : public JavascriptClass
{
    WeakMapClass()
    {
        #define WEAKMAP_CLASS_METHODS(X) \
            X (delete) X (clear) X (get) X (has) X (set)

        #define CREATE_WEAKMAP_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), WeakMap_ ## methodName);

        WEAKMAP_CLASS_METHODS (CREATE_WEAKMAP_METHOD)

        #undef WEAKMAP_CLASS_METHODS
        #undef CREATE_WEAKMAP_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("WeakMap")

    static var WeakMap_delete (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakMap_clear (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakMap_get (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakMap_has (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakMap_set (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WeakMapClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WeakSet
*/
struct WeakSetClass final : public JavascriptClass
{
    WeakSetClass()
    {
        #define WEAKSET_CLASS_METHODS(X) \
            X (add) X (clear) X (delete) X (has)

        #define CREATE_WEAKSET_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), WeakSet_ ## methodName);

        WEAKSET_CLASS_METHODS (CREATE_WEAKSET_METHOD)

        #undef WEAKSET_CLASS_METHODS
        #undef CREATE_WEAKSET_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("WeakSet")

    static var WeakSet_add (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakSet_clear (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakSet_delete (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var WeakSet_has (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WeakSetClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer
*/
struct ArrayBufferClass final : public JavascriptClass
{
    ArrayBufferClass()
    {
        #define ARRAYBUFFER_CLASS_METHODS(X) \
            X (isView) X (slice) X (transfer)

        #define CREATE_ARRAYBUFFER_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), methodName);

        ARRAYBUFFER_CLASS_METHODS (CREATE_ARRAYBUFFER_METHOD)

        #undef ARRAYBUFFER_CLASS_METHODS
        #undef CREATE_ARRAYBUFFER_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("ArrayBuffer")

    static var isView (Args a)      { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var slice (Args a)       { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var transfer (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrayBufferClass)
};

//==============================================================================
/**
    https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/DataView
*/
struct DataViewClass final : public JavascriptClass
{
    DataViewClass()
    {
        #define DATAVIEW_CLASS_METHODS(X) \
            X (getFloat32)  X (getFloat64)  X (getInt8)     X (getInt16)    X (getInt32) \
            X (getUint8)    X (getUint16)   X (getUint32)   X (setFloat32)  X (setFloat64) \
            X (setInt8)     X (setInt16)    X (setInt32)    X (setUint8)    X (setUint16) \
            X (setUint32)

        #define CREATE_DAVAVIEW_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), methodName);

        DATAVIEW_CLASS_METHODS (CREATE_DAVAVIEW_METHOD)

        #undef DATAVIEW_CLASS_METHODS
        #undef CREATE_DAVAVIEW_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("DataView")

    static var getFloat32 (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getFloat64 (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getInt8 (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getInt16 (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getInt32 (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUint8 (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUint16 (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var getUint32 (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setFloat32 (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setFloat64 (Args a)  { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setInt8 (Args a)     { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setInt16 (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setInt32 (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUint8 (Args a)    { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUint16 (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO
    static var setUint32 (Args a)   { ignoreUnused (a); jassertfalse; return var(); } //TODO

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataViewClass)
};
//==============================================================================

/**
    https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest/Using_XMLHttpRequest
*/
struct XMLHttpRequestClass final : public JavascriptClass
{
    XMLHttpRequestClass()
    {
        #define XMLHTTPREQUEST_CLASS_METHODS(X)

        #define CREATE_XMLHTTPREQUEST_METHOD(methodName) \
                setMethod (JUCE_STRINGIFY (methodName), methodName);

        XMLHTTPREQUEST_CLASS_METHODS (CREATE_XMLHTTPREQUEST_METHOD)

        #undef XMLHTTPREQUEST_CLASS_METHODS
        #undef CREATE_XMLHTTPREQUEST_METHOD
    }

    SP_JS_IDENTIFY_CLASS ("XMLHttpRequest")

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XMLHttpRequestClass)
};

//==============================================================================
var Scope::findFunctionCall (const CodeLocation& location, const var& targetObject, const Identifier& functionName) const
{
    if (auto* o = targetObject.getDynamicObject())
    {
        if (auto* prop = getPropertyPointer (*o, functionName))
            return *prop;

        for (auto* p = o->getProperty (getPrototypeIdentifier()).getDynamicObject();
             p != nullptr;
             p = p->getProperty (getPrototypeIdentifier()).getDynamicObject())
        {
            if (auto* prop = getPropertyPointer (*p, functionName))
                return *prop;
        }

        // if there's a class with an overridden DynamicObject::hasMethod, this avoids an error
        if (o->hasMethod (functionName))
            return {};
    }

    if (targetObject.isString())
        if (auto* m = findRootClassProperty (StringClass::getClassName(), functionName))
            return *m;

    if (targetObject.isArray())
        if (auto* m = findRootClassProperty (ArrayClass::getClassName(), functionName))
            return *m;

    if (auto* m = findRootClassProperty (ObjectClass::getClassName(), functionName))
        return *m;

    location.throwError ("Unknown function '" + functionName.toString() + "'");
    return {};
}

//==============================================================================
var NewOperator::getResult (const Scope& s) const
{
    const auto classOrFunc = object->getResult (s);
    const bool isFunc = isFunction (classOrFunc);

    if (! (isFunc || classOrFunc.getDynamicObject() != nullptr))
        return var::undefined();

    if (isFunc)
    {
        auto* newObject = new DynamicObject();
        invokeFunction (s, classOrFunc, newObject);
        return newObject;
    }

    if (classId.isValid())
    {
        Array<var> argVars;

        for (auto* a : arguments)
            argVars.add (a->getResult (s));

        if (classId == JSONClass::getClassName()
            || classId == MathClass::getClassName())
        {
            location.throwError (String ("XYZ is not constructable!")
                                    .replace ("XYZ", classId.getCharPointer(), true));
            return var::undefined();
        }

        DynamicObject* newObject = nullptr;

        if (classId == BooleanClass::getClassName())
        {
            newObject = BooleanClass::construct (argVars);
        }
        else if (classId == NumberClass::getClassName())
        {
            newObject = NumberClass::construct (argVars);
        }
        else if (classId == DateClass::getClassName())
        {
            newObject = DateClass::construct (argVars);
        }
        else
        {
            newObject = new DynamicObject();
            jassertfalse; // No idea what kind of class this is supposed to be! @todo perhaps?
        }

        if (newObject != nullptr)
        {
            newObject->setProperty (getPrototypeIdentifier(), classOrFunc);
            return newObject;
        }

        location.throwError (String ("Failed to create class of type: \"XYZ\".")
                                .replace ("XYZ", classId.getCharPointer(), true));
    }

    return var::undefined();
}
