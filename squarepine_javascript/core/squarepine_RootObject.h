//==============================================================================
/** */
class JavascriptClass : public DynamicObject
{
public:
    /** */
    JavascriptClass() = default;

    /** Basically operator===() for JS. */
    virtual bool areSameValue (const var&) { return false; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavascriptClass)
};

//==============================================================================
/** */
class RootObject final : public DynamicObject
{
public:
    /** */
    RootObject();

    //==============================================================================
    /** */
    void execute (const String& code);
    /** */
    var evaluate (const String& code);

    //==============================================================================
    // @todo timers and stuff
    // https://www.w3schools.com/jsref/met_win_settimeout.asp
    // https://www.w3schools.com/jsref/met_win_cleartimeout.asp
    // https://www.w3schools.com/jsref/met_win_clearinterval.asp
    OwnedArray<Timer> timers;
    Time timeout;

    //==============================================================================
    template<typename RootClass>
    void registerNativeObject()
    {
        setProperty (RootClass::getClassName(), new RootClass());
    }

    void setTimeoutInternal (const String&);

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootObject)
};
