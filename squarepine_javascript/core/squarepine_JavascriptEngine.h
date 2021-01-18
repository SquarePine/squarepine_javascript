/** A simple Javascript interpreter.

    Create an instance of this class and call execute() to run your Javascript code.

    Variables that the script sets can be retrieved with evaluate(),
    and if you need to provide native objects and/or methods for the script to use,
    you can add them with registerNativeObject().
*/
class JavascriptEngine final
{
public:
    /** Creates an instance of the engine.

        This creates a root namespace and defines most JS objects and functions.
    */
    JavascriptEngine();

    //==============================================================================
    /** Attempts to parse and run a block of javascript code.

        If there's a parse or execution error, the error description is returned in the result.

        You can specify a maximum time for which the program is allowed to run, and
        it'll return with an error message if this time is exceeded.
    */
    Result execute (const String& javascriptCode);

    /** Attempts to parse and run a javascript expression, and returns the result.

        If there's a syntax error, or the expression can't be evaluated, the return value
        will be var::undefined(). The errorMessage parameter gives you a way to find out
        any parsing errors.

        You can specify a maximum time for which the program is allowed to run, and
        it'll return with an error message if this time is exceeded.
    */
    var evaluate (const String& javascriptCode,
                  Result* errorMessage = nullptr);

    /** Calls a function in the root namespace, and returns the result.

        The function arguments are passed in the same format as used by native
        methods in the var class.
    */
    var callFunction (const Identifier& function,
                      const var::NativeFunctionArgs& args,
                      Result* errorMessage = nullptr);

    /** Calls a function object in the namespace of a dynamic object, and returns the result.

        The function arguments are passed in the same format as used by native
        methods in the var class.
    */
    var callFunctionObject (DynamicObject* objectScope,
                            const var& functionObject,
                            const var::NativeFunctionArgs& args,
                            Result* errorMessage = nullptr);

    //==============================================================================
    /** Adds a native object to the root namespace.

        The object passed-in is reference-counted, and will be retained by the
        engine until the engine is deleted.

        The name must be a simple JS identifier, without any dots.
    */
    void registerNativeObject (const Identifier& objectName, DynamicObject* object);

    /** Adds a method to the root namespace. */
    void registerMethod (const Identifier& methodName, var::NativeFunction function);

    /** Removes a native object from the root namespace. */
    void deregisterNativeObject (const Identifier& objectName);

    //==============================================================================
    /** This value indicates how long a call to one of the evaluate methods is permitted
        to run before timing-out and failing.

        The default value is 1 minute, but you can change this to whatever value suits your application.
    */
    RelativeTime maximumExecutionTime = { RelativeTime::minutes (1.0) };

    //==============================================================================
    /** When called from another thread, causes the interpreter to time-out as soon as possible */
    void stop() noexcept;

    /** Provides access to the set of properties of the root namespace object. */
    const NamedValueSet& getRootObjectProperties() const noexcept;

private:
    //==============================================================================
    ReferenceCountedObjectPtr<RootObject> root;

    //==============================================================================
    void prepareTimeout() const noexcept;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavascriptEngine)
};
