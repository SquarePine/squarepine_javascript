JavascriptEngine::JavascriptEngine() :
    root (new RootObject())
{
}

//==============================================================================
const NamedValueSet& JavascriptEngine::getRootObjectProperties() const noexcept
{
    return root->getProperties();
}

void JavascriptEngine::prepareTimeout() const noexcept
{
    root->timeout = Time::getCurrentTime() + maximumExecutionTime;
}

void JavascriptEngine::stop() noexcept
{
    root->timeout = {};
}

//==============================================================================
void JavascriptEngine::registerNativeObject (const Identifier& name, DynamicObject* object)
{
    jassert (! root->hasProperty (name)); // Be careful - we already have this object!

    root->setProperty (name, object);
}

void JavascriptEngine::registerMethod (const Identifier& name, var::NativeFunction function)
{
    jassert (! root->hasProperty (name)); // Be careful - we already have this method!

    root->setMethod (name, function);
}

void JavascriptEngine::deregisterNativeObject (const Identifier& name)
{
    jassert (root->hasProperty (name)); // Nothing found...?

    root->removeProperty (name);
}

//==============================================================================
Result JavascriptEngine::execute (const String& code)
{
    try
    {
        prepareTimeout();
        root->execute (code);
    }
    catch (String& error)
    {
        return Result::fail (error);
    }

    return Result::ok();
}

var JavascriptEngine::evaluate (const String& code, Result* result)
{
    prepareTimeout();

    if (result != nullptr)
        *result = Result::ok();

    try
    {
        return root->evaluate (code);
    }
    catch (String& error)
    {
        if (result != nullptr)
            *result = Result::fail (error);
    }

    return var::undefined();
}

//==============================================================================
var JavascriptEngine::callFunction (const Identifier& function, const var::NativeFunctionArgs& args, Result* result)
{
    auto returnVal = var::undefined();

    prepareTimeout();

    if (result != nullptr)
        *result = Result::ok();

    try
    {
        Scope ({}, *root, *root).findAndInvokeMethod (function, args, returnVal);
    }
    catch (String& error)
    {
        if (result != nullptr)
            *result = Result::fail (error);
    }

    return returnVal;
}

var JavascriptEngine::callFunctionObject (DynamicObject* objectScope, const var& functionObject,
                                          const var::NativeFunctionArgs& args, Result* result)
{
    auto returnVal = var::undefined();

    prepareTimeout();

    if (result != nullptr)
        *result = Result::ok();

    try
    {
        Scope rootScope ({}, *root, *root);

        Scope (&rootScope, *root, DynamicObject::Ptr (objectScope))
            .invokeMethod (functionObject, args, returnVal);
    }
    catch (String& error)
    {
        if (result != nullptr)
            *result = Result::fail (error);
    }

    return returnVal;
}
