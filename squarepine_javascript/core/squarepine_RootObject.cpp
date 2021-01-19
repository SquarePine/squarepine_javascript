RootObject::RootObject()
{
    setMethod ("exec",                  exec);
    setMethod ("eval",                  eval);
    setMethod ("trace",                 trace);
    setMethod ("typeof",                typeof_internal);
    setMethod ("instanceof",            instanceof_internal);
    setMethod ("charToInt",             charToInt);                 // NB: Non-standard
    setMethod ("parseInt",              NumberClass::parseInt);
    setMethod ("parseFloat",            NumberClass::parseFloat);
    setMethod ("isNaN",                 NumberClass::isNaN);
    setMethod ("isFinite",              NumberClass::isFinite);
    setMethod ("encodeURI",             encodeURI);
    setMethod ("encodeURIComponent",    encodeURIComponent);
    setMethod ("decodeURI",             decodeURI);
    setMethod ("decodeURIComponent",    decodeURIComponent);
    setMethod ("escape",                encodeURIComponent);
    setMethod ("unescape",              decodeURIComponent);
    setMethod ("setTimeout",            setTimeout);

    setProperty ("Infinity",            std::numeric_limits<double>::infinity());
    setProperty ("NaN",                 std::numeric_limits<double>::quiet_NaN());
    setProperty ("undefined",           var::undefined());

    //@todo https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/globalThis
    setProperty ("globalThis",          this);                      

    registerNativeObject<ConsoleClass>();           // NB: Non-standard.
    registerNativeObject<JUCEClass>();              // NB: Non-standard.

    registerNativeObject<ArrayBufferClass>();
    registerNativeObject<ArrayClass>();
    registerNativeObject<AtomicsClass>();
    registerNativeObject<BooleanClass>();
    registerNativeObject<BigIntClass>();
    registerNativeObject<DataViewClass>();
    registerNativeObject<DateClass>();
    registerNativeObject<JSONClass>();
    registerNativeObject<MapClass>();
    registerNativeObject<MathClass>();
    registerNativeObject<NumberClass>();
    registerNativeObject<ObjectClass>();
    registerNativeObject<ProxyClass>();
    registerNativeObject<ReflectClass>();
    registerNativeObject<RegExpClass>();
    registerNativeObject<SetClass>();
    registerNativeObject<StringClass>();
    registerNativeObject<SymbolClass>();
    registerNativeObject<WeakMapClass>();
    registerNativeObject<WeakSetClass>();
    registerNativeObject<XMLHttpRequestClass>();
}

//==============================================================================
void RootObject::execute (const String& code)
{
    ExpressionTreeBuilder tb (code);
    std::unique_ptr<BlockStatement> (tb.parseStatementList())
        ->perform (Scope ({}, *this, *this), nullptr);
}

var RootObject::evaluate (const String& code)
{
    ExpressionTreeBuilder tb (code);
    return ExpPtr (tb.parseExpression())->getResult (Scope ({}, *this, *this));
}

void RootObject::setTimeoutInternal (const String&)
{
    //timeout = a;
}
